using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace WaxLib
{

    // A destination of packets
    public abstract class PacketDest<T> : IDisposable
    {

        protected PacketConverter<T> packetConverter;

        public PacketDest(PacketConverter<T> packetConverter)
        {
            this.packetConverter = packetConverter;
        }

        public abstract void SendPacket(T packet);

        // Forward a packet (can be directly attached to a PacketSource.ReceivedPacket event)
        public void ForwardPacket(PacketSource<T> sender, PacketEventArgs<T> e)
        {
            if (e.Packet != null)
            {
                SendPacket(e.Packet);
            }
        }

        public virtual void Dispose()
        {
            ;
        }
    }
}
