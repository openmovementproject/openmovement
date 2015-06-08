using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;

namespace WaxLib
{
    // A OSC-originated source of packets
    public class OscSource<T> : PacketSource<T>
    {
        protected string topic;

        public OscSource(PacketConverter<T> packetConverter, string serverPortTopic) : base(packetConverter)
        {
            string server = OscReceiver.DEFAULT_INTERFACE;
            int port = OscReceiver.DEFAULT_PORT;
            string topic = packetConverter.GetDefaultTopic(false);
            PacketSource<T>.ParseServerPortTopic(serverPortTopic, ref server, ref port, ref topic);
            this.topic = topic;

            // Create OSC connection
            OscReceiver oscReceiver = null;
            try
            {
                oscReceiver = new OscReceiver(port);        // OscReceiver.DEFAULT_PORT
                oscReceiver.ReceivedBundle += oscReceiver_ReceivedBundle;
                oscReceiver.ReceivedMessage += oscReceiver_ReceivedMessage;
                oscReceiver.Start();
            }
            catch (Exception) { ; }
        }

        void oscReceiver_ReceivedBundle(object sender, OscBundleEventArgs e)
        {
            e.AutoUnpack = false;
            if (topic == null || e.Bundle.FirstAddress.StartsWith(topic))
            {
                T packet = packetConverter.FromOscBundle(e.Bundle, topic);
                if (packet != null)
                {
                    OnReceivedPacket(packet);
                }
            }
        }

        void oscReceiver_ReceivedMessage(object sender, OscMessageEventArgs e)
        {
            if (topic == null || e.Message.Address.StartsWith(topic))
            {
                T packet = packetConverter.FromOscBundle(new OscBundle(OscBundle.TIMESTAMP_NOW, e.Message), topic);
                if (packet != null)
                {
                    OnReceivedPacket(packet);
                }
            }
        }

    }
}
