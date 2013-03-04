using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;

namespace TeddiGui
{
    // A delegate type for received packets
    public delegate void UdpReceivedEventHandler(object sender, UdpReceivedEventArgs e);

    // UdpReceivedEventArgs 
    public class UdpReceivedEventArgs : EventArgs
    {
        public UdpReceivedEventArgs(DateTime timestamp, byte[] data) { Timestamp = timestamp; Data = data; }
        public DateTime Timestamp { get; protected set; }
        public byte[] Data { get; protected set; }
    }

    // Simple UDP receiver class
    public class UdpReceiver : IDisposable
    {
        // Constants
        public const string DEFAULT_INTERFACE = null;  // all interfaces
        public const int DEFAULT_PORT = 3333;       // default for TUIO

        // Private instance variables
        private string address;
        private int port;
        private IPEndPoint endPoint;
        private UdpClient udpClient;

        // Default constructor -- default interface and default port
        public UdpReceiver() : this(DEFAULT_INTERFACE, DEFAULT_PORT) { ; }

        // Receiver with specified port
        public UdpReceiver(int port) : this(DEFAULT_INTERFACE, port) { ; }

        // Receiver and port specified, port can be overridden with "address:port" in address
        public UdpReceiver(string address, int port)
        {
            this.address = address;
            this.port = port;
        }


        // An event that clients can use to be notified whenever a packet is received
        public event UdpReceivedEventHandler UdpReceived;

        // Invoke the ReceivedBundle event
        protected virtual void OnUdpReceived(UdpReceivedEventArgs e)
        {
            if (UdpReceived != null) { UdpReceived(this, e); }
        }

        public bool Listening { get { return (thread != null); } }


        // Receiver thread
        private Thread thread = null;
        private volatile bool quitReceiver = false;

        // Destructor
        ~UdpReceiver()
        {
            Stop();
        }


        // Start receiving
        public void Start()
        {
            // Ensure stopped
            Stop();


            // If a port is specified in the address, override the given port
            int index = (address == null) ? -1 : address.LastIndexOf(':');
            if (index >= 0)
            {
                port = int.Parse(address.Substring(index + 1));
                address = address.Substring(0, index);
            }

            // Lookup host
            IPHostEntry ipEntry = (address != null && address.Length > 0) ? Dns.GetHostEntry(address) : null;

            // If receiver was specified but not found
            if (ipEntry != null && ipEntry.AddressList.Length <= 0)
            {
                //Trace.TraceWarning("Host not found: " + address + " - cannot begin receiving.");
                throw new SocketException(11001);       // WSAHOST_NOT_FOUND
            }

            // Use address (any if none specified)
            IPAddress ipAddress = (ipEntry != null && ipEntry.AddressList.Length > 0) ? ipEntry.AddressList[0] : IPAddress.Any;

            // Create end-point and USP socket
            endPoint = new IPEndPoint(ipAddress, port);
            try
            {
                udpClient = new UdpClient(endPoint);
            }
            catch (SocketException e)
            {
                //Trace.TraceWarning("SocketException: " + e.Message + " - cannot begin receiving (check port not in use).");
                throw e;
            }


            // Create and start receiver thread
            quitReceiver = false;
            thread = new Thread(new ThreadStart(this.Listener));
            thread.Start();
        }

        public void Join()
        {
            if (thread != null)
            {
                thread.Join();
            }
        }

        // Stop receiving
        public void Stop()
        {
            // Stop listening thread if exists
            if (thread != null)
            {
                // Set quit flag, interrupt thread and wait to terminate...
                quitReceiver = true;
                thread.Interrupt();
                if (!thread.Join(250))
                {
                    // If that hasn't worked, send Abort and wait...
                    thread.Abort();
                    if (!thread.Join(250))
                    {
                        // If that hasn't worked, UdpClient.Receive() seems to be immune even to that! -- force close the socket...
                        if (udpClient != null && udpClient.Client != null)
                        {
                            udpClient.Client.Close();
                            if (!thread.Join(250))
                            {
                                ; // If that hasn't worked, give up!
                            }
                        }
                    }
                }
                thread = null;
            }
        }


        // UDP listening thread
        private void Listener()
        {
            while (!quitReceiver)
            {
                try
                {
                    // Receive UDP packet
                    byte[] buffer = udpClient.Receive(ref endPoint);

                    if (quitReceiver) { break; }

                    DateTime timestamp = DateTime.UtcNow;
                    OnUdpReceived(new UdpReceivedEventArgs(timestamp, buffer));
                }
                catch (ThreadInterruptedException)
                {
                    Console.Error.WriteLine("WARNING: ThreadInterruptedException in Listener...");
                    if (quitReceiver) { break; }
                }
                catch (SocketException)
                {
                    Console.Error.WriteLine("WARNING: SocketException in Listener...");
                    if (quitReceiver) { break; }
                }
            }
            udpClient.Close();
        }

        // Dispose
        public void Dispose()
        {
            Stop();
        }


    }
}
