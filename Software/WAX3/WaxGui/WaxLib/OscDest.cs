using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;

namespace WaxLib
{
    // A OSC destination for packets
    public class OscDest<T> : PacketDest<T>
    {
        protected OscTransmitter oscTransmitter;
        protected string topic;

        public OscDest(PacketConverter<T> packetConverter, string serverPortTopic) : base(packetConverter)
        {
            string server = OscReceiver.DEFAULT_INTERFACE;
            int port = OscReceiver.DEFAULT_PORT;
            string topic = packetConverter.GetDefaultTopic(false);
            PacketSource<T>.ParseServerPortTopic(serverPortTopic, ref server, ref port, ref topic);

            oscTransmitter = new OscTransmitter(server, port);
            this.topic = topic;
        }

        public override void SendPacket(T packet)
        {
            if (oscTransmitter != null)
            {
                OscBundle oscBundle = packetConverter.ToOscBundle(packet, topic);
                oscTransmitter.Send(oscBundle);
            }
        }

    }
}
