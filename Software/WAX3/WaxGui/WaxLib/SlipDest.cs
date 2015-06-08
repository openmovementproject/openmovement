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
    // A SLIP-encoded destination for packets
    public class SlipDest<T> : PacketDest<T>, IDisposable
    {
        private Stream stream;

        public SlipDest(PacketConverter<T> packetConverter, Stream stream) : base(packetConverter)
        {
            this.stream = stream;
        }

        ~SlipDest()
        {
            Dispose();
        }

        // Helper method to returns a stream for a serial port with the correct configuration
        public static Stream OpenSerialPort(string portName)
        {
            // Create and open serial port
            SerialPort serialPort = new SerialPort(portName, 19200, Parity.None, 8, StopBits.One);
            serialPort.Handshake = Handshake.None;
            serialPort.ReadTimeout = 250;  // SerialPort.InfiniteTimeout;
            serialPort.WriteTimeout = 250; // SerialPort.InfiniteTimeout;
            serialPort.Open();
            return serialPort.BaseStream;
        }

        public override void SendPacket(T packet)
        {
            byte[] binary = packetConverter.ToBinary(packet);
            byte[] buffer = Slip.CreateSlipPacket(binary);
            stream.Write(buffer, 0, buffer.Length);
        }

        public override void Dispose()
        {
            // Close serial port if open
            if (stream != null)
            {
                stream.Close();
                stream = null;
            }
            base.Dispose();
        }

    }
}
