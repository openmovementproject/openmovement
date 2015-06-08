using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace WaxLib
{

    // A delegate type for received packets
    public delegate void PacketEventHandler<T>(PacketSource<T> sender, PacketEventArgs<T> e);

    // PacketEventArgs
    public class PacketEventArgs<T> : EventArgs
    {
        public T Packet { get; protected set; }
        public PacketEventArgs(T packet) { Packet = packet; }
    }


    // A source of packets that will raise the ReceivedPacket event
    public abstract class PacketSource<T> : IDisposable
    {

        // An event that clients can use to be notified whenever a packet is received
        public event PacketEventHandler<T> ReceivedPacket;

        protected PacketConverter<T> packetConverter;

        public PacketSource(PacketConverter<T> packetConverter)
        {
            this.packetConverter = packetConverter;
        }

        protected void OnReceivedPacket(T packet)
        {
            if (ReceivedPacket != null)
            {
                PacketEventArgs<T> packetEventArgs = new PacketEventArgs<T>(packet);
                ReceivedPacket.Invoke(this, packetEventArgs);
            }
        }

        public virtual void Dispose()
        {
            ;
        }

        public static void ParseServerPortTopic(string serverPortTopic, ref string server, ref int port, ref string topic)
        {
            if (serverPortTopic != null && serverPortTopic.Length > 0 && serverPortTopic != ".")
            {
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
                int colon = serverPortTopic.IndexOf(':');
                if (colon >= 0)
                {
                    int.TryParse(serverPortTopic.Substring(colon + 1), out port);
                    if (colon > 0) { server = serverPortTopic.Substring(0, colon); }
                }
                else
                {
                    if (serverPortTopic.Length > 0) { server = serverPortTopic; }
                }
            }
        }

    }
}
