using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;

namespace WaxLib
{
    // A STOMP-originated source of packets
    public class StompSource<T> : PacketSource<T>
    {
        protected StompConnection stompConnection = null;
        protected StompListener stompListener;

        public StompSource(PacketConverter<T> packetConverter, string serverPortTopic) : base(packetConverter)
        {
            string server = "localhost";
            int port = StompConnection.DEFAULT_PORT;
            string topic = packetConverter.GetDefaultTopic(true);
            PacketSource<T>.ParseServerPortTopic(serverPortTopic, ref server, ref port, ref topic);

            // Create STOMP connection
            stompConnection = null;
            try
            {
                stompConnection = new StompConnection(server, port);
            }
            catch (Exception) { ; }

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
            T packet = packetConverter.FromStomp(e.Message.Body);
            if (packet != null)
            {
                OnReceivedPacket(packet);
            }
        }

        public override void Dispose()
        {
            if (stompListener != null) { stompListener.Stop(); }
        }

    }
}
