using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;
using System.IO.Ports;
using System.IO;
using System.Collections;

namespace TeddiGui
{
    public partial class MainForm : Form
    {
        // Start-up parameters
        public string PortName { get; set; }
        public bool AutoConnect { get; set; }
        public string LogFile { get; set; }

        private SerialPort serialPort;
        private StreamWriter logStream = null;

        GraphVisualizer graphPanel;
        MapVisualizer mapPanel;

        public MainForm()
        {
            // Initialize
            InitializeComponent();

            // Logging
            (new TextBoxStreamWriter(textBoxLog)).SetConsoleOut();
            Trace.Listeners.Add(new ConsoleTraceListener());
            Console.WriteLine("Started.");

            // Add visualizers
            graphPanel = new GraphVisualizer(manager);
            splitContainerLog.Panel1.Controls.Add(graphPanel);
            graphPanel.Dock = DockStyle.Fill;

            mapPanel = new MapVisualizer(manager);
            splitContainerLog.Panel1.Controls.Add(mapPanel);
            mapPanel.Dock = DockStyle.Fill;

            // Visualizer
            Visualizer = VisualizerType.None;

            // Serial port
            serialPort = new SerialPort();
            serialPort.BaudRate = 19200;
            serialPort.Parity = Parity.None;
            serialPort.DataBits = 8;
            serialPort.StopBits = StopBits.One;
            serialPort.Handshake = Handshake.None;
            serialPort.Encoding = System.Text.Encoding.GetEncoding(28591);
            //serialPort.ReadTimeout = 250;  // SerialPort.InfiniteTimeout;
            //serialPort.WriteTimeout = 250; // SerialPort.InfiniteTimeout;
            serialPort.DataReceived += serialPort_DataReceived;

            // Update serial ports
            UpdatePorts(PortName);

            // Auto-connection
            if (AutoConnect)
            {
                toolStripButtonConnect.PerformClick();
            }
        }

        public bool LoadFromFile(string filename)
        {
            // Load map
            if (mapPanel.LoadFromFile(filename))
            {
                toolStripButtonMap.PerformClick();
                return true;
            }
            return false;
        }

        public bool SaveToFile(string filename)
        {
            // Save map
            return mapPanel.SaveToFile(filename);
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            try
            {
                logStream = new StreamWriter(LogFile, true, Encoding.UTF8, 1024);
            }
            catch (Exception ex)
            {
                MessageBox.Show("Cannot open log file (" + ex.Message + ")\r\n" + LogFile);
            }
        }

        private void UpdatePorts(string existingPort = null)
        {
            // Sort ports by numerical component
            string[] ports = SerialPort.GetPortNames();
            int[] portValues = new int[ports.Length];
            for (int i = 0; i < portValues.Length; i++)
            {
                string p = ports[i];
                int value = 0;
                for (int j = 0; j < p.Length; j++)
                {
                    if (char.IsDigit(p[j])) { value = value * 10 + (int)(p[j] - '0'); }
                }
                if (value == 0) { value = int.MaxValue; }       // Don't favour non-numeric ports
                if (value == 1) { value = int.MaxValue - 2; }   // Don't favour COM1
                if (value == 2) { value = int.MaxValue - 1; }   // Don't favour COM2
                portValues[i] = value;
            }
            Array.Sort(portValues, ports);

            // Update port list
            if (existingPort != null)
            {
                existingPort = toolStripComboBoxPort.Text;
            }
            toolStripComboBoxPort.Items.Clear();
            toolStripComboBoxPort.Items.AddRange(ports);

            // Use first item if no port specified
            if ((existingPort == null || existingPort.Length == 0) && ports.Length > 0)
            {
                existingPort = ports[0];
            }

            // Check port exists
            for (int i = 0; i < ports.Length; i++)
            {
                if (ports[i].Equals(existingPort, StringComparison.OrdinalIgnoreCase))
                {
                    toolStripComboBoxPort.SelectedIndex = i;
                }
            }
        }

        protected override void WndProc(ref Message m)
        {
            // Definitions from dbt.h and winuser.h
            const int WM_DEVICECHANGE = 0x0219;
            //const int DBT_DEVICEARRIVAL = 0x8000;

            if (m.Msg == WM_DEVICECHANGE)   // && m.WParam.ToInt32() == DBT_DEVICEARRIVAL
            {
                //int devType = Marshal.ReadInt32(m.LParam, 4);
                //if (devType == DBT_DEVTYP_...
                UpdatePorts();
            }
            base.WndProc(ref m);
        }

        private void toolStripButtonConnect_Click(object sender, EventArgs e)
        {
            if (serialPort.IsOpen)
            {
                Console.WriteLine("Disconnecting...");
                serialPort.Close();
            }
            else
            {
                try
                {
                    Console.WriteLine("Connecting: " + toolStripComboBoxPort.Text + "...");
                    serialPort.PortName = toolStripComboBoxPort.Text;
                    serialPort.Open();
                }
                catch (System.Exception ex)
                {
                    MessageBox.Show(this, "" + ex.Message, this.Text, MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }

            // Update buttons
            toolStripButtonConnect.Checked = serialPort.IsOpen;
            toolStripComboBoxPort.Enabled = !serialPort.IsOpen;
        }



        private List<byte> buffer = new List<byte>();

        private void serialPort_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            try
            {
                byte[] bytes = new byte[serialPort.BytesToRead];
                serialPort.Read(bytes, 0, bytes.Length);
                buffer.AddRange(bytes);

                byte[] slipPacket;
                while ((slipPacket = Slip.ExtractSlipPacket(buffer)) != null)
                {
                    BinaryPacketReceived(slipPacket, DateTime.UtcNow);
                }
            }
            catch (System.Exception ex)
            {
                Console.WriteLine("ERROR: Exception reading serial port: " + ex.Message);
            }
        }



        // OSC Transmitter
        protected OscTransmitter oscTransmitter;
        protected string oscTopic = null;
        protected bool oscCooked = false;

        public void StartOscTransmitter(string serverPortTopic, bool cooked)
        {
            oscCooked = cooked;
            string server = OscReceiver.DEFAULT_INTERFACE;
            int port = OscReceiver.DEFAULT_PORT;
            string topic = null;
            string user = "";
            string password = "";
            ParseServerPortTopic(serverPortTopic, ref server, ref port, ref topic, ref user, ref password);
            oscTransmitter = new OscTransmitter(server, port);
            oscTopic = topic;
        }

        public void SendOscPacket(Packet packet)
        {
            if (oscTransmitter != null)
            {
                OscBundle oscBundle = Packet.PacketToOscBundle(packet, oscTopic, oscCooked);
                if (oscBundle != null)
                {
                    oscTransmitter.Send(oscBundle);
                }
            }
        }



        // Manager / visualizer
        protected Manager manager = new Manager();

        private void BinaryPacketReceived(byte[] data, DateTime timestamp)
        {
            if (data == null) { return; }

            Packet packet = Packet.PacketFromBinary(data, timestamp);
            if (packet != null)
            {
                TeddiPacketReceived(packet);
            }

            StatusPacket statusPacket = StatusPacket.StatusPacketFromBinary(data, timestamp);
            if (statusPacket != null)
            {
                TeddiStatusPacketReceived(statusPacket);
            }
        }

        private void TeddiPacketReceived(Packet packet)
        {
            if (logStream != null)
            {
                logStream.WriteLine(packet.ToString());
            }

            lock (manager)
            {
                manager.PacketReceived(packet);
            }

            if (motionRun != null)
            {
                if (motionRun.ContainsKey(packet.DeviceId))
                {
                    MotionTracker motionTracker = motionRun[packet.DeviceId];
                    motionTracker.NewPacket(packet);
                }
            }

            SendOscPacket(packet);

            if (graphPanel.Visible) { graphPanel.Invalidate(); }
            if (mapPanel.Visible) { mapPanel.Invalidate(); }
        }

        private void TeddiStatusPacketReceived(StatusPacket statusPacket)
        {
            if (logStream != null)
            {
//                logStream.WriteLine(statusPacket.ToString());
            }

            lock (manager)
            {
                manager.StatusPacketReceived(statusPacket);
            }

            //SendOscPacket(statusPacket);

            if (graphPanel.Visible) { graphPanel.Invalidate(); }
            if (mapPanel.Visible) { mapPanel.Invalidate(); }
        }


        public enum VisualizerType { None, Graph, Map };

        VisualizerType visualizer = VisualizerType.None;
        VisualizerType Visualizer
        {
            get { return visualizer; }
            set
            {
                visualizer = value;

                if (visualizer == VisualizerType.Graph)
                {
                    graphPanel.Visible = true;
                    mapPanel.Visible = false;
                }
                else if (visualizer == VisualizerType.Map)
                {
                    graphPanel.Visible = false;
                    mapPanel.Visible = true;
                }
                else
                {
                    graphPanel.Visible = false;
                    mapPanel.Visible = false;
                }

                toolStripButtonGraph.Checked = graphPanel.Visible;
                toolStripButtonMap.Checked = mapPanel.Visible;
            }
        }

        private void toolStripButtonGraph_Click(object sender, EventArgs e)
        {
            if (!toolStripButtonGraph.Checked)
            {
                Visualizer = VisualizerType.Graph;
            }
            else
            {
                Visualizer = VisualizerType.None;
            }
        }

        private void toolStripButtonMap_Click(object sender, EventArgs e)
        {
            if (!toolStripButtonMap.Checked)
            {
                Visualizer = VisualizerType.Map;
            }
            else
            {
                Visualizer = VisualizerType.None;
            }
        }

        private void toolStripButtonOpen_Click(object sender, EventArgs e)
        {
            DialogResult dr = openFileDialog.ShowDialog();
            if (dr == System.Windows.Forms.DialogResult.OK)
            {
                LoadFromFile(openFileDialog.FileName);
            }
        }

        private void toolStripButtonSave_Click(object sender, EventArgs e)
        {
            DialogResult dr = saveFileDialog.ShowDialog();
            if (dr == System.Windows.Forms.DialogResult.OK)
            {
                SaveToFile(saveFileDialog.FileName);
            }
        }




        // UDP Receiver
        private UdpReceiver udpReceiver;
        public void StartListening(int listenPort)
        {
            udpReceiver = new UdpReceiver(listenPort);
            udpReceiver.UdpReceived += udpReceiver_UdpReceived;
            udpReceiver.Start();
        }
        void udpReceiver_UdpReceived(object sender, UdpReceivedEventArgs e)
        {
            BinaryPacketReceived(e.Data, e.Timestamp);
        }




        // STOMP Receiver
        protected StompConnection stompConnection = null;
        protected StompListener stompListener;

        public static void ParseServerPortTopic(string serverPortTopic, ref string server, ref int port, ref string topic, ref string user, ref string password)
        {
            if (serverPortTopic != null && serverPortTopic.Length > 0 && serverPortTopic != ".")
            {
                // Locate path after server name
                int slash = serverPortTopic.IndexOf('/');
                if (slash >= 0)
                {
                    topic = serverPortTopic.Substring(slash);
                    topic = topic.Replace("$lt", "<");
                    topic = topic.Replace("$gt", ">");
                    topic = topic.Replace("$amp", "&");
                    topic = topic.Replace("$perc", "%");
                    serverPortTopic = serverPortTopic.Substring(0, slash);
                }

                // Locate username/password before server name
                int atsymbol = serverPortTopic.IndexOf('@');
                if (atsymbol >= 0)
                {
                    string userPassword = serverPortTopic.Substring(0, atsymbol);
                    serverPortTopic = serverPortTopic.Substring(atsymbol + 1);

                    // Locate password in username
                    int colonPassword = userPassword.IndexOf(':');
                    if (colonPassword >= 0)
                    {
                        password = userPassword.Substring(colonPassword + 1);
                        userPassword = userPassword.Substring(0, colonPassword);
                    }
                    user = userPassword;
                }

                // Parse and remove port from server name
                int colonPort = serverPortTopic.IndexOf(':');
                if (colonPort >= 0)
                {
                    int.TryParse(serverPortTopic.Substring(colonPort + 1), out port);
                    serverPortTopic = serverPortTopic.Substring(0, colonPort);
                }

                // Server name
                if (serverPortTopic.Length > 0)
                {
                    server = serverPortTopic;
                }

            }
        }

        public void StartStompReceiver(string serverPortTopic)
        {
            string server = "localhost";
            int port = StompConnection.DEFAULT_PORT;
            string topic = "/topic/Teddi.Sensor";
            string user = "";
            string password = "";
            ParseServerPortTopic(serverPortTopic, ref server, ref port, ref topic, ref user, ref password);

            // Create STOMP connection
            stompConnection = null;
            try
            {
                stompConnection = new StompConnection(server, port, user, password);
            }
            catch (Exception e)
            {
                Console.WriteLine("ERROR: Cannot connect to STOMP server: " + e.Message + ".");
            }

            if (stompConnection != null)
            {
                // Subscribe to a topic
                stompConnection.Subscribe(topic);

                // Create threaded listener and add a message handler
                stompListener = new StompListener(stompConnection);
                stompListener.ReceivedMessage += stompListener_ReceivedMessage;
            }

        }

        void stompListener_ReceivedMessage(object sender, StompMessageEventArgs e)
        {
            Hashtable data = (Hashtable)Json.JsonDecode(e.Message.Body);
            if (data != null)
            {
                string type = null;

                if (data.ContainsKey("Type")) { type = data["Type"].ToString(); }

                if (type == "TEDDI" || (type == null && data.ContainsKey("Humidity")))  // Appears to be TEDDI type
                {
                    Packet packet = Packet.PacketFromStomp(data);
                    if (packet != null)
                    {
                        TeddiPacketReceived(packet);
                    }
                }
                else if (type == "TEDDI_Status")            // Appears to be TEDDI_Status type
                {
                    StatusPacket statusPacket = StatusPacket.StatusPacketFromStomp(data);
                    if (statusPacket != null)
                    {
                        TeddiStatusPacketReceived(statusPacket);
                    }
                }
                else if (type == null)
                {
                    string debug = e.Message.Body;
                    if (debug.Length > 128) { debug = debug.Substring(0, 128) + "..."; }
                    Console.WriteLine("WARNING: Un-typed JSON STOMP message: " + debug + "");
                }
                else
                {
                    Console.WriteLine("WARNING: Unknown STOMP type: " + type + "");
                }
            }
            else
            {
                string debug = e.Message.Body;
                if (debug.Length > 128) { debug = debug.Substring(0, 128) + "..."; }
                Console.WriteLine("WARNING: Couldn't parse JSON: " + debug + "");
            }
        }




        // Stop
        private void MainForm_FormClosed(object sender, FormClosedEventArgs e)
        {
            if (udpReceiver != null) { udpReceiver.Stop(); }
            if (stompListener != null) { stompListener.Stop(); }
        }


        private IDictionary<int, MotionTracker> motionRun = new Dictionary<int, MotionTracker>();
        public void AddMotionRun(ICollection<MotionTracker> motionRun)
        {
            foreach (MotionTracker motionTracker in motionRun)
            {
                this.motionRun.Add(motionTracker.Id, motionTracker);
            }
        }

    }
}
