using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;
using System.IO;
using System.Threading;
using System.IO.Ports;

namespace WaxLib
{
    // A logging destination for packets
    public class LogDest<T> : PacketDest<T>, IDisposable
    {
        private TextWriter textWriter;

        public LogDest(PacketConverter<T> packetConverter, TextWriter textWriter) : base(packetConverter)
        {
            this.textWriter = textWriter;
        }

        ~LogDest()
        {
            Dispose();
        }

        public override void SendPacket(T packet)
        {
            string logData = packetConverter.ToLog(packet);
            //byte[] buffer = (new System.Text.UTF8Encoding()).GetBytes(logData);
            //stream.Write(buffer, 0, buffer.Length);
            textWriter.Write(logData);
        }

        public override void Dispose()
        {
            // Close stream if open
            //if (stream != null) { stream.Close(); stream = null; }
            if (textWriter != null) { textWriter.Close(); textWriter = null; }
            base.Dispose();
        }

    }
}
