// STOMP Sender/receiver
// Original from: http://svn.stomp.codehaus.org/browse/stomp/trunk/dotnet/
// Modifications by Dan Jackson, 2011.

/*
 
// Simple STOMP sending:

    // Create STOMP connection
    StompConnection stompConnection = new StompConnection("localhost", 61613);

    // Send to a topic
    string topic = "/topic/TestTopic";
    stompConnection.Send(topic, @"{""test"":123}");


// Simple STOMP receiving:

    // Create STOMP connection
    StompConnection stompConnection = new StompConnection("localhost", 61613);

    // Subscribe to a topic
    stompConnection.Subscribe(topic);

    // Notify listener when a message is received (NOTE: listener is called from another thread)
    StompListener stompListener = new StompListener(stompConnection);
    stompListener.ReceivedMessage += (s, e) => 
    {
        Console.WriteLine("MESSAGE: " + e.Message.Body); 
    };


    // (An alternative: block and wait for a message synchronously)
    //StompMessage stompMessage = stompConnection.WaitForMessage();
    //if (stompMessage != null) { Console.WriteLine("MESSAGE: " + stompMessage.Body); }
*/

using System;
using System.Collections;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;

namespace WaxLib
{
    #region StompMessage
    public class StompMessage
    {
        private readonly string body;
        private readonly string destination;
        private readonly IDictionary headers;

        public StompMessage(string destination, string body, IDictionary headers)
        {
            this.body = body;
            this.destination = destination;
            this.headers = headers;
        }

        public string Body
        {
            get { return body; }
        }

        public string Destination
        {
            get { return destination; }
        }

        public string this[string key]
        {
            get { return (string)headers[key]; }
        }
    }
    #endregion

    #region StompConnection
    public class StompConnection : IDisposable
    {
        public const int DEFAULT_PORT = 61613;  // Default for Apache ActiveMQ STOMP interface

        private readonly object transmissionLock = new object();
        private Socket socket;
        private TextWriter socketWriter;
        private TextReader socketReader;

        public delegate void MessageDelegate(string destination, string body, IDictionary headers);

        public StompConnection(string host, int port, string login, string passcode)
        {
            //@listeners = {}
            socket = Connect(host, port);
            socketWriter = new StreamWriter(new NetworkStream(socket));
            socketReader = new StreamReader(new NetworkStream(socket));

            Transmit("CONNECT", null, null, "login", login, "passcode", passcode);
            Packet ack = Receive();
            if (ack.command != "CONNECTED")
            {
                throw new ApplicationException("Could not connect : " + ack);
            }
        }

        public StompConnection(string host, int port)
            : this(host, port, "", "")
        {
        }

        private Socket Connect(string host, int port)
        {
            // Looping through the AddressList allows different type of connections to be tried 
            // (IPv4, IPv6 and whatever else may be available).
            IPHostEntry hostEntry = Dns.GetHostEntry(host);            
            foreach (IPAddress address in hostEntry.AddressList)
            {
if (address.AddressFamily == AddressFamily.InterNetworkV6 && address.ToString() == "::1") { continue; }
                try
                {
                    Socket socket = new Socket(address.AddressFamily, SocketType.Stream, ProtocolType.Tcp);
                    socket.Connect(new IPEndPoint(address, port));
                    if (socket.Connected)
                    {
                        return socket;
                    }
                }
                catch (SocketException) { ; }
            }
            throw new SocketException();
        }

        public void Dispose()
        {
            if (socket != null)
            {
                Transmit("DISCONNECT", null, null);
                socket.Close();
                socketWriter.Close();
                socketReader.Close();
                socketReader = null;
                socketWriter = null;
                socket = null;
            }
        }

        public void Send(string destination, string body, IDictionary headers)
        {
            Transmit("SEND", body, headers, "destination", destination);
        }

        public void Send(string destination, string body)
        {
            Send(destination, body, null);
        }

        public void Begin()
        {
            Transmit("BEGIN", null, null);
        }

        public void Commit()
        {
            Transmit("COMMIT", null, null);
        }

        public void Abort()
        {
            Transmit("ABORT", null, null);
        }

        public void Subscribe(string destination, IDictionary headers)
        {
            Transmit("SUBSCRIBE", null, headers, "destination", destination);
        }

        public void Subscribe(string destination)
        {
            Subscribe(destination, null);
        }

        public void Unsubscribe(string destination, IDictionary headers)
        {
            // If you supplied a consumerID in the message then you will still be subscribed
            Transmit("UNSUBSCRIBE", null, headers, "destination", destination);
        }

        public void Unsubscribe(string destination)
        {
            Unsubscribe(destination, null);
        }

        private void Transmit(string command, string body, IDictionary headers, params string[] additionalHeaderPairs)
        {
            lock (transmissionLock)
            {
                socketWriter.WriteLine(command);
                for (int i = 0; i < additionalHeaderPairs.Length; i += 2)
                {
                    string key = additionalHeaderPairs[i];
                    string val = additionalHeaderPairs[i + 1];
                    socketWriter.WriteLine("{0}:{1}", key, val);
                    if (headers != null)
                    {
                        headers.Remove(key);
                    }
                }
                if (headers != null)
                {
                    foreach (object key in headers.Keys)
                    {
                        object val = headers[key];
                        socketWriter.WriteLine("{0}:{1}", key, val);
                    }
                }
                socketWriter.WriteLine();
                socketWriter.WriteLine(body);
                socketWriter.WriteLine('\u0000');
                socketWriter.Flush();
            }
        }

        public StompMessage WaitForMessage()
        {
            Packet packet = Receive();
            if (packet != null && packet.command == "MESSAGE")
            {
                return new StompMessage((string)packet.headers["destination"], packet.body, packet.headers);
            }
            else
            {
                return null;
            }
        }

        private Packet Receive()
        {
            Packet packet = null;
            if (socket == null || socketReader == null) { return null; }
            try     // [dgj]
            {
                packet = new Packet();
                packet.command = socketReader.ReadLine(); // MESSAGE, ERROR or RECEIPT

                //return if command =~ /\A\s*\Z/

                // [dgj] Fix
                if (packet.command == "") { packet.command = socketReader.ReadLine(); }

                string line;
                while ((line = socketReader.ReadLine()) != "")
                {
                    string[] split = line.Split(new char[] { ':' }, 2);
                    packet.headers[split[0]] = split.Length > 1 ? split[1] : null;
                }

                StringBuilder body = new StringBuilder();
                int nextChar;
                while ((nextChar = socketReader.Read()) != 0)
                {
                    body.Append((char)nextChar);
                }
                packet.body = body.ToString().TrimEnd('\r', '\n');

                //Console.Out.WriteLine(packet);
                return packet;
            }
            catch (ObjectDisposedException e)
            {
                Console.Error.WriteLine(e.Message);
                return null;
            }
            catch (IOException e)
            {
                Console.Error.WriteLine(e.Message);
                return null;
            }
        }

        private class Packet
        {
            public string command;
            public string body;
            public IDictionary headers = new Hashtable();

            public override string ToString()
            {
                StringBuilder result = new StringBuilder();
                result.Append(command).Append(Environment.NewLine);
                foreach (DictionaryEntry entry in headers)
                {
                    result.Append(entry.Key).Append(':').Append(entry.Value).Append(Environment.NewLine);
                }
                result.Append("----").Append(Environment.NewLine); ;
                result.Append(body);
                result.Append("====").Append(Environment.NewLine);
                return result.ToString();
            }
        }

    }
    #endregion

    #region StompListener

    // [dgj] A delegate type for received messages
    public delegate void StompMessageEventHandler(object sender, StompMessageEventArgs e);

    // [dgj] StompMessageEventArgs
    public class StompMessageEventArgs : EventArgs
    {
        public StompMessage Message { get; protected set; }
        public StompMessageEventArgs(StompMessage message) { Message = message; }
    }

    // [dgj] Simple listener thread
    public class StompListener : IDisposable
    {
        private StompConnection stompConnection;

        public StompListener(StompConnection stompConnection)
        {
            this.stompConnection = stompConnection;
            Start();
        }


        // An event that clients can use to be notified whenever a message is received
        public event StompMessageEventHandler ReceivedMessage;

        // Invoke the ReceivedMessage event
        protected virtual void OnReceivedMessage(StompMessageEventArgs e)
        {
            if (ReceivedMessage != null) { ReceivedMessage(this, e); }
        }

        // Receiver thread
        private Thread thread = null;
        private volatile bool quitReceiver = false;

        // Destructor
        ~StompListener()
        {
            Stop();
        }


        // Start receiving
        protected void Start()
        {
            // Ensure stopped
            Stop();

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
                        ; // If that hasn't worked, give up!
                    }
                }
                thread = null;
                stompConnection.Dispose();
            }
        }


        // Listening thread
        protected void Listener()
        {
            while (!quitReceiver)
            {
                try
                {
                    StompMessage stompMessage = stompConnection.WaitForMessage();
                    if (stompMessage != null)
                    {
                        OnReceivedMessage(new StompMessageEventArgs(stompMessage));
                    }
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
            stompConnection.Abort();
        }

        // Dispose
        public void Dispose()
        {
            Stop();
        }


    }
    #endregion

}


