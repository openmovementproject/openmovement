using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace TeddiGui
{
    public class StatusPacket
    {

        public const int NUM_COORDINATOR = 64;

        public ushort deviceId;
        public byte version;
        public byte power;
        public ushort sequence;
        public ushort shortAddress;
        public byte lastLQI;
        public byte lastRSSI;
        public ushort parentAddress;
        public ushort parentAltAddress;
        public ushort[] neighbours;
        public DateTime timestampReceived;

        public StatusPacket(ushort deviceId, byte version, byte power, ushort sequence, ushort shortAddress, byte lastLQI, byte lastRSSI, ushort parentAddress, ushort parentAltAddress, ushort[] neighbours, DateTime timestampReceived)
        {
            this.deviceId = deviceId;
            this.version = version;
            this.power = power;
            this.sequence = sequence;
            this.shortAddress = shortAddress;
            this.lastLQI = lastLQI;
            this.lastRSSI = lastRSSI;
            this.parentAddress = parentAddress;
            this.parentAltAddress = parentAltAddress;
            this.neighbours = (ushort[])neighbours.Clone();
            this.timestampReceived = timestampReceived;
        }


        // Method to return a StatusPacket, or null if invalid byte array
        public static StatusPacket StatusPacketFromBinary(byte[] buffer, DateTime timestampReceived)
        {
            StatusPacket statusPacket = null;

            if (buffer != null && buffer.Length > 0)
            {
                if (buffer.Length >= 5 && buffer[0] == 0x12 && buffer[1] == 0x53)           // USER_REPORT_TYPE && TEDDI (ASCII 'S')
                {
                    ushort deviceId = (ushort)(buffer[2] | (((ushort)buffer[3]) << 8));
                    byte version = buffer[4];

                    if (buffer.Length >= 24)
                    {
                        /*
                        #define NUM_COORDINATOR 64
                        typedef struct
                        {
                            unsigned char  reportType;          // @ 0  [1] USER_REPORT_TYPE (0x12)
                            unsigned char  reportId;            // @ 1  [1] Report identifier (0x53, ASCII 'S')
                            unsigned short deviceId;            // @ 2  [2] Device identifier (16-bit)
                            unsigned char  version;             // @ 4  [1] Low nibble = packet version (0x3), high nibble = config (0x0)
                            unsigned char  power;               // @ 5  [1] Power (top-bit indicates USB, if low-15 are 0x7ffff, unknown)
                            unsigned short sequence;			// @ 6  [2] Packet sequence number
                            unsigned short shortAddress;		// @ 8  [2] Short address
                            unsigned char  lastLQI;				// @ 10 [1] LQI of last received keep-alive broadcast
                            unsigned char  lastRSSI;			// @ 11 [1] RSSI of last received keep-alive broadcast
                            unsigned short parentAddress;		// @ 12 [2] Parent address
                            unsigned short parentAltAddress;	// @ 14 [2] Parent alt. address
                            unsigned char  neighbours[NUM_COORDINATOR/8];	// @ 16 [8] Neighbouring routers bitmap
                            unsigned long long timestampReceived;
                        } TeddiStatusPacket;
                        */
                        byte power = (byte)(buffer[5]);
                        ushort sequence = (ushort)(buffer[6] | (((ushort)buffer[7]) << 8));
                        ushort shortAddress = (ushort)(buffer[8] | (((ushort)buffer[9]) << 8));
                        byte lastLQI = (byte)(buffer[10]);
                        byte lastRSSI = (byte)(buffer[11]);
                        ushort parentAddress = (ushort)(buffer[12] | (((ushort)buffer[13]) << 8));
                        ushort parentAltAddress = (ushort)(buffer[14] | (((ushort)buffer[15]) << 8));

                        // Neighbour table
                        byte[] neighbourBitmap = new byte[NUM_COORDINATOR / 8];
                        for (int i = 0; i < NUM_COORDINATOR / 8; i++)
                        {
                            neighbourBitmap[i] = buffer[16 + i];
                        }
                        List<ushort> neighbourList = new List<ushort>();
                        for (int i = 0; i < NUM_COORDINATOR; i++)
                        {
                            if ((neighbourBitmap[i / 8] & (1 << (i & 7))) != 0)
                            {
                                neighbourList.Add((ushort)i);
                            }
                        }
                        ushort[] neighbours = neighbourList.ToArray();

                        statusPacket = new StatusPacket(deviceId, version, power, sequence, shortAddress, lastLQI, lastRSSI, parentAddress, parentAltAddress, neighbours, timestampReceived);

                    }
                    else
                    {
                        Console.Error.Write("[S?]");	// Unknown TEDDI status packet type
                    }
                }
                else
                {
                    //Console.Error.Write("[?]");		// Unknown packet type
                }
            }
            return statusPacket;
        }



        public static StatusPacket StatusPacketFromStomp(Hashtable data)
        {
            long timestampReceivedLong = 0;
            ushort deviceId = 0;
            byte version = 0;
            byte power = 0;
            ushort sequence = 0;
            ushort shortAddress = 0;
            byte lastLQI = 0;
            byte lastRSSI = 0;
            ushort parentAddress = 0;
            ushort parentAltAddress = 0;
            ushort[] neighbours = null;

            // Check for "TEDDI_Status" type
            if (data != null && data.ContainsKey("Type") && data["Type"].ToString() == "TEDDI_Status")
            {
                if (data.ContainsKey("TimestampReceived")) { timestampReceivedLong = (long)(double)Double.Parse(data["TimestampReceived"].ToString()); }
                if (data.ContainsKey("DeviceId")) { deviceId = (ushort)(double)Double.Parse(data["DeviceId"].ToString()); }
                if (data.ContainsKey("Version")) { version = (byte)(double)Double.Parse(data["Version"].ToString()); }
                if (data.ContainsKey("Power")) { power = (byte)(double)Double.Parse(data["Power"].ToString()); }
                if (data.ContainsKey("Sequence")) { sequence = (ushort)(double)Double.Parse(data["Sequence"].ToString()); }
                if (data.ContainsKey("ShortAddress")) { shortAddress = (ushort)(double)Double.Parse(data["ShortAddress"].ToString()); }
                if (data.ContainsKey("LastLQI")) { lastLQI = (byte)(double)Double.Parse(data["LastLQI"].ToString()); }
                if (data.ContainsKey("LastRSSI")) { lastRSSI = (byte)(double)Double.Parse(data["LastRSSI"].ToString()); }
                if (data.ContainsKey("ParentAddress")) { parentAddress = (ushort)(double)Double.Parse(data["ParentAddress"].ToString()); }
                if (data.ContainsKey("ParentAltAddress")) { parentAltAddress = (ushort)(double)Double.Parse(data["ParentAltAddress"].ToString()); }

                if (data.ContainsKey("Neighbours") && data["Neighbours"] is ArrayList)
                {
                    ArrayList neighbourList = (ArrayList)data["Neighbours"];
                    neighbours = new ushort[neighbourList.Count];
                    for (int i = 0; i < neighbourList.Count; i++)
                    {
                        neighbours[i] = (ushort)(double)Double.Parse(neighbourList[i].ToString());
                    }
                }
            }
            else
            {
                Console.Error.Write("[STOMP:null]");
            }

            DateTime timestampReceived = DateTime.FromBinary(timestampReceivedLong);
            return new StatusPacket(deviceId, version, power, sequence, shortAddress, lastLQI, lastRSSI, parentAddress, parentAltAddress, neighbours, timestampReceived);
        }



    }
}
