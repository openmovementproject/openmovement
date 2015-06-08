using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;
using System.IO;
using System.IO.Ports;

namespace WaxLib
{
    // A delegate type for hooking up change notifications.
    public delegate void ManagerEventHandler(object sender, ManagerEventArgs e);

    // ManagerEventArgs
    public class ManagerEventArgs : EventArgs
    {
        public WaxDevice Device { get; private set; }
        public WaxPacket Packet { get; private set; }
        public ManagerEventArgs(WaxDevice device)
        {
            Device = device;
        }
        public ManagerEventArgs(WaxDevice device, WaxPacket packet)
        {
            Device = device;
            Packet = packet;
        }

        public string Name;
        public bool Value;
        public string Label;
        public ManagerEventArgs(string name, bool value, string label)
        {
            Name = name;
            Value = value;
            Label = label;
        }
    }


    public class WaxManager : IDisposable
    {
        // Thread-safe synchronization, callers should lock(SyncRoot) around access to the properties
        private object syncRoot = new Object();
        public object SyncRoot { get { return syncRoot; } }

        protected List<PacketSource<WaxPacket>> packetSources = new List<PacketSource<WaxPacket>>();
        protected List<PacketDest<WaxPacket>> packetDests = new List<PacketDest<WaxPacket>>();

        public string DebugString { get; protected set; }

        private IDictionary<ushort, WaxDevice> waxDevices = new Dictionary<ushort, WaxDevice>();

        public WaxDevice GetDevice(ushort id) { return waxDevices.ContainsKey(id) ? waxDevices[id] : null; }

        public ICollection<WaxDevice> Devices { get { return waxDevices.Values; } }

        // An event that clients can use to be notified whenever a packet arrives
        public event ManagerEventHandler ReceivedPacket;

        // An event that clients can use to be notified whenever a new device is seen
        public event ManagerEventHandler NewDevice;

        // An event that clients can use to be notified whenever a hypothesis changes
        public event ManagerEventHandler HypothesisChanged;

        // Invoke the ReceivedPacket event
        protected virtual void OnReceivedPacket(ManagerEventArgs e)
        {
            if (ReceivedPacket != null) { ReceivedPacket(this, e); }
        }

        // Invoke the NewDevice event
        protected virtual void OnNewDevice(ManagerEventArgs e)
        {
            if (NewDevice != null) { NewDevice(this, e); }
        }

        
        // Invoke the HypothesisChanged event
        protected virtual void OnHypothesisChanged(ManagerEventArgs e)
        {
            if (HypothesisChanged != null) { HypothesisChanged(this, e); }
        }

        public readonly WaxPacketConverter WaxPacketConverter = new WaxPacketConverter();

        public WaxManager()
        {
        }

        public void AddPacketSource(PacketSource<WaxPacket> source)
        {
            packetSources.Add(source);
            source.ReceivedPacket += PacketSource_ReceivedPacket;
        }

        public void RemovePacketSource(PacketSource<WaxPacket> source)
        {
            source.ReceivedPacket -= PacketSource_ReceivedPacket;
            packetSources.Remove(source);
        }

        public void AddPacketDest(PacketDest<WaxPacket> dest)
        {
            packetDests.Add(dest);
        }

        public void RemovePacketDest(PacketDest<WaxPacket> dest)
        {
            packetDests.Remove(dest);
        }

        private void PacketSource_ReceivedPacket(PacketSource<WaxPacket> sender, PacketEventArgs<WaxPacket> e)
        {
            // If a correctly parsed packet...
            if (e.Packet != null && e.Packet is WaxPacket)
            {
                WaxPacket packet = (WaxPacket)e.Packet;

                //Console.WriteLine("[" + packet.Timestamp + "]");
                //Console.WriteLine(packet.ToString());

                lock (SyncRoot)
                {
                    if (!waxDevices.ContainsKey(packet.DeviceId))
                    {
                        // Create new device
                        waxDevices.Add(packet.DeviceId, new WaxDevice(packet.DeviceId, packet.Timestamp));
                        OnNewDevice(new ManagerEventArgs(waxDevices[packet.DeviceId]));
                    }
                    WaxDevice waxDevice = waxDevices[packet.DeviceId];

                    waxDevice.AddSamples(packet.Samples);

                    // Forward packet to all destinations
                    foreach (PacketDest<WaxPacket> dest in packetDests)
                    {
                        dest.SendPacket(packet);
                    }

                    if (packet != null)
                    {
                        DebugString = packet.ToString();
                    }
                }
                OnReceivedPacket(new ManagerEventArgs(waxDevices[packet.DeviceId], packet));
            }
        }


        // Create Source / Dest objects from the arguments, updates the arguments array (removes processed commands)
        public void FromArgs(ref string[] args)
        {
            Console.WriteLine("WaxManager - Dan Jackson, 2011.");
            List<string> outArgs = new List<string>();
            for (int i = 0; i < args.Length; i++)
            {
                string arg = args[i];
                if (arg == "-inport") 
                { 
                    string portName = args[++i];
                    if (portName == "*")
                    {
                        portName = null;
                        string[] ports = SerialPort.GetPortNames();
                        foreach (string port in ports)
                        {
                            portName = port;
                        }
                    }
                    if (portName != null)
                    {
                        AddPacketSource(new SlipSource<WaxPacket>(WaxPacketConverter, SlipSource<WaxPacket>.OpenSerialPort(portName)));
                    }
                    else
                    {
                        Console.WriteLine("WARNING: Cannot find any COM ports.");
                    }
                }
                else if (arg == "-instomp") { string serverPortTopic = args[++i]; AddPacketSource(new StompSource<WaxPacket>(WaxPacketConverter, serverPortTopic)); }
                else if (arg == "-inosc") { string serverPort = args[++i]; AddPacketSource(new OscSource<WaxPacket>(WaxPacketConverter, serverPort)); }
                else if (arg == "-intail") 
                { 
                    string filename = args[++i];
                    AddPacketSource(new SlipSource<WaxPacket>(WaxPacketConverter, SlipSource<WaxPacket>.OpenTailFile(filename)));
                }
                else if (arg == "-outfile") 
                {
                    const string DEFAULT_LOG_FILE = "wax-{date}-{time}.csv";
                    string file = args[++i];
                    if (file == "") { file = DEFAULT_LOG_FILE; }
                    if (file != null && file.Length > 0)
                    {
                        DateTime now = DateTime.Now;
                        file = file.Replace("{date}", now.ToString(@"yyyy-MM-dd"));
                        file = file.Replace("{time}", now.ToString(@"HH-mm-ss"));
                    }
                    AddPacketDest(new LogDest<WaxPacket>(WaxPacketConverter, new StreamWriter(new FileStream(file, FileMode.Append)))); 
                }
                else if (arg == "-outconsole")
                {
                    AddPacketDest(new LogDest<WaxPacket>(WaxPacketConverter, Console.Out));
                }
                else if (arg == "-outstomp") { string serverPortTopic = args[++i]; AddPacketDest(new StompDest<WaxPacket>(WaxPacketConverter, serverPortTopic)); }
                else if (arg == "-outosc") { string serverPortTopic = args[++i]; AddPacketDest(new OscDest<WaxPacket>(WaxPacketConverter, serverPortTopic)); }
                else if (arg == "-out") { string serverPortTopic = args[++i]; SetOutServerPortTopic(serverPortTopic); }
                else
                {
                    outArgs.Add(arg);
                }
            }
            args = outArgs.ToArray();
        }

        // HACK: Embedded the hypothesis generation in here for no good reason...
        public const string DEFAULT_OUT_STOMP_TOPIC = "/topic/Kitchen.Activity";
        protected bool outStompAttempted = false;
        protected StompConnection outStompConnection = null;
        protected string outStompHost = "localhost";
        protected int outStompPort = StompConnection.DEFAULT_PORT;  // 61613
        public string outStompTopic = DEFAULT_OUT_STOMP_TOPIC;

        public void SetOutServerPortTopic(string serverPortTopic)
        {
            PacketSource<WaxPacket>.ParseServerPortTopic(serverPortTopic, ref outStompHost, ref outStompPort, ref outStompTopic);
        }

        public IDictionary<string, string> activeHypothesis = new Dictionary<string, string>();
        public string CurrentHypothesis { get; protected set; }

        public void UpdateHypothesis(string name, bool value, string label = null)
		{
//Console.WriteLine("HYPOTHESIS: " + name + "=" + value + " (" + label + ").");

            if (label == null) { label = name; }
            if (!value && activeHypothesis.ContainsKey(name))
            {
                activeHypothesis.Remove(name);
            }
            else if (value)
            {
                activeHypothesis[name] = label;
            }

            string currentLabel = null;
            foreach (KeyValuePair<string, string> kvp in activeHypothesis)
            {
                currentLabel = kvp.Value;
            }
            //Console.WriteLine("HYPOTHESIS: " + currentLabel);
            CurrentHypothesis = currentLabel;
            ManagerEventArgs mea = new ManagerEventArgs(name, value, label);
            OnHypothesisChanged(mea);


            if (outStompConnection == null && !outStompAttempted)
            {
                try
                {
                    outStompConnection = new StompConnection(outStompHost, outStompPort);
                }
                catch (Exception)
                {
                    ;
                }
                outStompAttempted = true;
            }
            if (outStompConnection != null)
            {
                // Create JSON-encoded data
                Hashtable sourceData = new Hashtable();
                sourceData.Add("name", name);
                sourceData.Add("value", value);
                string jsonData = Json.JsonEncode(sourceData);
                outStompConnection.Send(outStompTopic, jsonData);
                Console.WriteLine("Sent " + jsonData + " to " + outStompTopic);
            }
            else
            {
                //Console.WriteLine("FAILED to send to " + outStompTopic);
            }
        }

        public void Dispose()
        {
            foreach (PacketSource<WaxPacket> s in packetSources) { s.Dispose(); }
            foreach (PacketDest<WaxPacket> d in packetDests) { d.Dispose(); }
            packetSources.Clear();
            packetDests.Clear();
        }

    }
}
