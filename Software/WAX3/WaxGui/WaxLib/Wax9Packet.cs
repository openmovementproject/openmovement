using System;
using System.Collections.Generic;
using System.Text;
using System.Collections;

namespace WaxLib
{
    public class Wax9Packet
    {

        // 9-axis packet type (always little-endian, transmitted SLIP-encoded)

        // Standard part (26-bytes)
        public byte packetType;                        // @ 0 ASCII '9' for 9-axis
        public byte packetVersion;                     // @ 1 Version (0x01 = standard, 0x02 = extended)
        public ushort sampleNumber;                    // @ 2 Sample number (reset on configuration change, inactivity, or wrap-around)
        public uint timestamp;                         // @ 4 Timestamp (16.16 fixed-point representation, seconds)
        public short[] accel = new short[3];           // @ 8 Accelerometer
        public short[] gyro = new short[3];            // @14 Gyroscope
        public short[] mag = new short[3];             // @20 Magnetometer

        // Extended part
        public ushort battery = 0xffff;                // @26 Battery (mV)
        public short temperature = -32768;             // @28 Temperature (0.1 degrees C)
        public uint pressure = 0xffffffff;             // @30 Pressure (Pascal)

        // Factory method to return a WaxPacket, or null if invalid byte array
        public static Wax9Packet FromBinary(byte[] buffer, DateTime timestamp)
        {
            // Invalid packet
            if (buffer == null || buffer.Length < 20 || buffer[0] != '9')
            {
                return null;
            }

            Wax9Packet packet = new Wax9Packet();
            packet.packetType = buffer[0];                                          // @ 0 ASCII '9' for 9-axis
            packet.packetVersion = buffer[1];                                       // @ 1 Version (0x01 = standard, 0x02 = extended)
            packet.sampleNumber = (ushort)(buffer[2] | (((ushort)buffer[3]) << 8)); // @ 2 Sample number (reset on wrap-around)
            packet.timestamp = (uint)(buffer[4] | (((uint)buffer[5]) << 8) | (((uint)buffer[6]) << 16) | (((uint)buffer[7]) << 24));    // @ 4 Timestamp (16.16 fixed-point representation, seconds)
            packet.accel[0] = (short)(buffer[8] | (((short)buffer[9]) << 8));
            packet.accel[1] = (short)(buffer[10] | (((short)buffer[11]) << 8));
            packet.accel[2] = (short)(buffer[12] | (((short)buffer[13]) << 8));

            if (buffer.Length >= 20)
            {
                packet.gyro[0] = (short)(buffer[14] | (((short)buffer[15]) << 8));
                packet.gyro[1] = (short)(buffer[16] | (((short)buffer[17]) << 8));
                packet.gyro[2] = (short)(buffer[18] | (((short)buffer[19]) << 8));
            }

            if (buffer.Length >= 26)
            {
                packet.mag[0] = (short)(buffer[20] | (((short)buffer[21]) << 8));
                packet.mag[1] = (short)(buffer[22] | (((short)buffer[23]) << 8));
                packet.mag[2] = (short)(buffer[24] | (((short)buffer[25]) << 8));
            }

            if (buffer.Length >= 28)
            {
                packet.battery = (ushort)(buffer[26] | (((ushort)buffer[27]) << 8));
            }

            if (buffer.Length >= 30)
            {
                packet.temperature = (short)(buffer[28] | (((short)buffer[29]) << 8));
            }

            if (buffer.Length >= 34)
            {
                packet.pressure = (uint)(buffer[30] | (((uint)buffer[31]) << 8) | (((uint)buffer[32]) << 16) | (((uint)buffer[33]) << 24));
            }

            return packet;
        }

        public override string ToString()
        {
            // "N,Ax,Ay,Az,Gx,Gy,Gz,Mx,My,Mz,Batmv,Temp0.1C,PresPa,Ia"
            StringBuilder sb = new StringBuilder();
            sb.Append(this.sampleNumber).Append(",");
            sb.Append(this.accel[0]).Append(","); sb.Append(this.accel[1]).Append(","); sb.Append(this.accel[2]).Append(",");
            sb.Append(this.gyro[0]).Append(","); sb.Append(this.gyro[1]).Append(","); sb.Append(this.gyro[2]).Append(",");
            sb.Append(this.mag[0]).Append(","); sb.Append(this.mag[1]).Append(","); sb.Append(this.mag[2]);
            if (this.pressure != 0xffffffff)
            {
                sb.Append(",");
                sb.Append(this.battery).Append(",");
                sb.Append(this.temperature).Append(",");
                sb.Append(this.pressure);
            }
            return sb.ToString();
        }

    }
}
