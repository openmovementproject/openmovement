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
    // A SLIP-originated source of packets
    public class SlipSource<T> : PacketSource<T>, IDisposable
    {
        private Thread receiverThread = null;
        private Stream stream;
        private List<byte> packetBuffer = new List<byte>();
        private volatile bool quitReceiver = false;

        public SlipSource(PacketConverter<T> packetConverter, Stream stream) : base(packetConverter)
        {
            this.stream = stream;

            quitReceiver = false;
            receiverThread = new Thread(new ThreadStart(this.Listener));
            receiverThread.Start();
        }

        ~SlipSource()
        {
            Dispose();
        }

        public static Stream OpenSerialPort(string portName)
        {
            return OpenSerialPort(portName, null);
        }

        // Helper method to returns a stream for a serial port with the correct configuration
        public static Stream OpenSerialPort(string portName, string initiallySend)
        {
            // Create and open serial port
            SerialPort serialPort = new SerialPort(portName, 19200, Parity.None, 8, StopBits.One);
            serialPort.Encoding = System.Text.Encoding.GetEncoding(28591);
            serialPort.Handshake = Handshake.None;
            serialPort.ReadTimeout = 250;  // SerialPort.InfiniteTimeout;
            serialPort.WriteTimeout = 250; // SerialPort.InfiniteTimeout;
            serialPort.Open();
            if (initiallySend != null)
            {
                try { serialPort.Write(initiallySend); }
                catch (Exception) { ; }
            }
            return serialPort.BaseStream;
        }

        public static Stream OpenTailFile(string filename)
        {
            FileStream fileStream = new FileStream(filename, FileMode.Open, FileAccess.Read, FileShare.ReadWrite);
            fileStream.Seek(fileStream.Length, SeekOrigin.Begin);
            return fileStream;
        }

        private void Listener()
        {
            while (!quitReceiver)
            {
                try
                {
                    byte[] rawPacket = ReadPacket();
                    if (rawPacket != null)
                    {
                        DateTime timestamp = DateTime.UtcNow;
                        T packet = packetConverter.FromBinary(rawPacket, timestamp);
                        if (packet != null)
                        {
                            OnReceivedPacket(packet);
                        }
                    }
                }
                catch (ThreadInterruptedException)
                {
                    if (quitReceiver) { break; }
                }
            }
        }

        private long lastEnd = -1;

        // Read a packet
        protected byte[] ReadPacket()
        {
            if (!quitReceiver && stream != null)
            {
                if (stream is FileStream)
                {
                    FileStream fileStream = (FileStream)stream;

                    // Store last end position
                    if (lastEnd == -1) { lastEnd = fileStream.Position; }

                    // Deal with file truncation, by starting at the start of the file
                    long end = fileStream.Length;
                    if (end < lastEnd)
                    {
                        fileStream.Seek(0, SeekOrigin.Begin);
                        packetBuffer.Clear();
                        lastEnd = fileStream.Position;
                    }

                    // Read any new bytes from the file
                    if (end > lastEnd)
                    {
                        byte[] buffer = new byte[end - lastEnd];
                        int length = fileStream.Read(buffer, 0, buffer.Length);
                        lastEnd += length;

                        for (int i = 0; i < length; i++)
                        {
                            packetBuffer.Add((byte)buffer[i]);
                        }
                    }

                    // Let's parse any packets
                    for (; ; )
                    {
                        byte[] packet = Slip.ExtractSlipPacket(packetBuffer);
                        if (packet == null) { break; }
                        if (packet.Length > 0)
                        {
                            //Console.WriteLine("ReadPacket():");
                            //Console.WriteLine(Slip.HexDump(packet));
                            return packet;
                        }
                    }

                    // No packets
                    Thread.Sleep(20);   // short delay to prevent spinning waiting for file appends
                }
                else
                {
                    for (; ; )
                    {
                        // LATER: Shame stream doesn't seem to support checking how many bytes are available without blocking
                        int b = -1;

                        try { b = stream.ReadByte(); }
                        catch (TimeoutException) { }
                        catch (IOException) { }
                        catch (ObjectDisposedException) { }
                        //catch (ThreadInterruptedException) { stream.Close(); break; }

                        if (b == -1) { break; }
                        packetBuffer.Add((byte)b);

                        // Let's parse any packets
                        for (; ; )
                        {
                            byte[] packet = Slip.ExtractSlipPacket(packetBuffer);
                            if (packet == null) { break; }
                            if (packet.Length > 0)
                            {
                                //Console.WriteLine("ReadPacket():");
                                //Console.WriteLine(Slip.HexDump(packet));
                                return packet;
                            }
                        }
                    }
                }
            }
            return null;
        }


        public override void Dispose()
        {
            // Clear any existing partial packets
            packetBuffer.Clear();

            if (!quitReceiver)
            {
                // Set quit flag and wait timeout
                quitReceiver = true;

                // Wait for read timeout (should check flag and terminate loop)
                if (!receiverThread.Join(1000))
                {
                    receiverThread.Interrupt();
                    Thread.Sleep(300);
                }
            }

            // Close serial port if open
            if (stream != null)
            {
                try { stream.Close(); }
                catch (Exception) { ; }
                stream = null;
            }

            base.Dispose();
        }

    }
}
