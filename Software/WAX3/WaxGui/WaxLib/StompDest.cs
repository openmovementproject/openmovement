using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;

namespace WaxLib
{
    // A STOMP destination for packets
    public class StompDest<T> : PacketDest<T>
    {
        protected StompConnection stompConnection = null;
        protected string topic;

        public StompDest(PacketConverter<T> packetConverter, string serverPortTopic) : base(packetConverter)
        {
            string server = "localhost";
            int port = StompConnection.DEFAULT_PORT;
            string topic = packetConverter.GetDefaultTopic(false);
            PacketSource<T>.ParseServerPortTopic(serverPortTopic, ref server, ref port, ref topic);
            this.topic = topic;

            // Create STOMP connection
            stompConnection = null;
            try
            {
                stompConnection = new StompConnection(server, port);
            }
            catch (Exception) { ; }
        }

        public override void SendPacket(T packet)
        {
            if (stompConnection != null)
            {
                string body = packetConverter.ToStomp(packet);
                string postfix = packetConverter.GetTopicPostfix(packet);
                string theTopic = topic;
                if (postfix != null && postfix.Length > 0) { theTopic += "." + postfix; }
                stompConnection.Send(theTopic, body);
            }
        }

    }
}
