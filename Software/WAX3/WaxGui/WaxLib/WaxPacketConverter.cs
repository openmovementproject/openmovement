using System;
using System.Collections.Generic;
using System.Text;
using System.Collections;

namespace WaxLib
{
    public class WaxPacketConverter : PacketConverter<WaxPacket>
    {

        static int BITPACK10_SIZEOF(int n) { return (((n / 4) * 5) + (((n & 0x03) == 0) ? 0 : ((n & 0x03) + 1))); }

        // Un-pack 4x 10-bit samples from each 5-byte group (stored little-endian).
        static ushort BitUnpack_uint10(byte[] buffer, int index, int offset)
        {
            int o = offset + ((index >> 2) * 5);
            if (o < 0 || o + (index & 0x03) + 1 >= buffer.Length) { throw new IndexOutOfRangeException(); }
            switch (index & 0x03)
            {
                case 0: return (ushort)( (ushort)(buffer[o + 0]     ) | (ushort)((buffer[o + 1] & 0x0003) << 8) );    // A
                case 1: return (ushort)( (ushort)(buffer[o + 1] >> 2) | (ushort)((buffer[o + 2] & 0x000f) << 6) );    // B
                case 2: return (ushort)( (ushort)(buffer[o + 2] >> 4) | (ushort)((buffer[o + 3] & 0x003f) << 4) );    // C
                case 3: return (ushort)( (ushort)(buffer[o + 3] >> 6) | (ushort)((buffer[o + 4]         ) << 2) );    // D
            }
            throw new IndexOutOfRangeException();
        }


        // Checksum - 16-bit word-size addition
        ushort WordSum(byte[] buffer)
        {
            int len = buffer.Length;
            int words = (len / 2);
            ushort value = 0x0000;						        // Initial sum of zero
            for (int i = 0; i < words; i++)
            {
                value += (ushort)(buffer[2 * i] + (buffer[2 * i + 1] << 8));
            }
            if ((len & 1) != 0) { value += buffer[len - 1]; }   // Add odd byte
            return value;
            //return (~value) + 1;								// To calculate the checksum, take bitwise NOT of sum, then add 1 (total sum of words including checksum will be zero)
        }


        // Factory method to return a WaxPacket, or null if invalid byte array
        public WaxPacket FromBinary(byte[] buffer, DateTime timestamp)
        {
            WaxPacket waxPacket = null;

            // USER_REPORT_TYPE
            if (buffer != null && buffer.Length > 0)
            {
//Console.WriteLine("FromBinary():");
//Console.WriteLine(Slip.HexDump(buffer));

                if (buffer.Length >= 2 && buffer[0] == 0x12 && buffer[1] == 0x78 && buffer.Length >= 12)           // New WAX hardware (ASCII 'x')
                {
                    /*
	                unsigned char  reportType;		// [1] = 0x12 (USER_REPORT_TYPE)
	                unsigned char  reportId;	    // [1] = 0x78 (ASCII 'x')
	                unsigned short deviceId;		// [2] = Device identifier (16-bit)
	                unsigned char  status;			// [1] = Device status (bit 0 is battery warning, top 7 bits reserved)
	                unsigned short sample;			// [2] = Analogue sample (top 6 bits indicate measurement information, lower 10 bits are the value)
	                unsigned char  format;			// [1] = Accelerometer data format 0xE9 (+/-2g, 2-bytes, 100Hz); Top two bits is g-range, next two bits is format [0=3x10+2 bits, 2=signed 16-bit] (2); lowest four bits is the rate code: frequency = 3200 / (1 << (15-(n & 0x0f)))
	                unsigned short sequenceId;		// [2] = Sequence number of first accelerometer reading in this packet (16-bit sample index -- will wrap or be reset if device sleeps and resumes)
	                unsigned char  outstanding;		// [1] = Number of samples remaining on device after this packet (0xff: >= 255)
	                unsigned char  sampleCount;		// [1] = Number of samples in this packet. 0x0c = 13 samples (number of accelerometer samples)
	                signed short   sampleData[BUFFER_PACKET_MAX_SAMPLES * 3];	// [sampleCount * numAxes * bytesPerSample = 78 -- would be 102 with 17 samples] (sample data)
                    */

                    ushort deviceId = (ushort)(buffer[2] | (((ushort)buffer[3]) << 8));
                    byte status = buffer[4];
                    ushort adcSample = (ushort)(buffer[5] | (((ushort)buffer[6]) << 8));
                    byte format = buffer[7];
                    ushort sequenceId = (ushort)(buffer[8] | (((ushort)buffer[9]) << 8));
                    byte outstanding = buffer[10];
                    byte sampleCount = buffer[11];
                    WaxSample[] sampleData = new WaxSample[sampleCount];

                    // Format: 0xE0 | AccelCurrentRate()    
                    // [1] = Accelerometer data format 0xEA (3-axis, 2-bytes, 100Hz); Top two bits is number of axes (3), next two bits is format [1=unsigned 8-bit,2=signed 16-bit] (2); lowest four bits is the rate code: frequency = 3200 / (1 << (15-(n & 0x0f)))
                    int bytesPerSample = 0;
                    if (((format >> 4) & 0x03) == 2) { bytesPerSample = 6; }    // 3*16-bit
                    else if (((format >> 4) & 0x03) == 0) { bytesPerSample = 4; }    // 3*10-bit + 2

                    int expectedLength = 12 + sampleCount * bytesPerSample;
                    if (buffer.Length < expectedLength)
                    {
                        Console.WriteLine("WARNING: Ignoring truncated- or unknown-format data packet (received " + buffer.Length + " expected " + expectedLength + ").");
                    }
                    else
                    {
                        if (buffer.Length > expectedLength)
                        {
                            Console.WriteLine("WARNING: Data packet was larger than expected, ignoring additional samples");
                        }

                        int frequency = 3200 / (1 << (15 - (format & 0x0f)));

                        StringBuilder sampleDataString = new StringBuilder();

                        for (int i = 0; i < sampleCount; i++)
                        {
                            int samplesAgo = sampleCount + outstanding - 1 - i;

                            short x = 0, y = 0, z = 0;
                            if (bytesPerSample == 6)
                            {
                                x = (short)((ushort)(buffer[12 + i * 6] | (((ushort)buffer[13 + i * 6]) << 8)));
                                y = (short)((ushort)(buffer[14 + i * 6] | (((ushort)buffer[15 + i * 6]) << 8)));
                                z = (short)((ushort)(buffer[16 + i * 6] | (((ushort)buffer[17 + i * 6]) << 8)));
                            }
                            else if (bytesPerSample == 4)
                            {
                                uint value = (uint)buffer[12 + i * 4] | ((uint)buffer[13 + i * 4] << 8) | ((uint)buffer[14 + i * 4] << 16) | ((uint)buffer[15 + i * 4] << 24);
                                x = (short)( (short)((ushort)0xffc0 & (ushort)(value <<  6)) >> (6 - ((byte)(value >> 30))) );		// Sign-extend 10-bit value, adjust for exponent
                                y = (short)( (short)((ushort)0xffc0 & (ushort)(value >>  4)) >> (6 - ((byte)(value >> 30))) );		// Sign-extend 10-bit value, adjust for exponent
                                z = (short)( (short)((ushort)0xffc0 & (ushort)(value >> 14)) >> (6 - ((byte)(value >> 30))) );		// Sign-extend 10-bit value, adjust for exponent
                            }
                            uint index = (uint)sequenceId + (uint)i;

                            DateTime t = timestamp - TimeSpan.FromMilliseconds(samplesAgo * 1000 / frequency);

                            sampleData[i] = new WaxSample(t, index, x, y, z);
                        }

                        //Console.WriteLine(line);
                        waxPacket = new WaxPacket(timestamp, deviceId, 1, 0, adcSample, sequenceId, format, sampleCount, sampleData);
                    }

                }
                else if (buffer.Length >= 2 && buffer[0] == 0x12 && buffer[1] == 0x58 && buffer.Length >= 12)           // Old WAX hardware (ASCII 'X')
                {
                    if (buffer.Length > 0 && buffer.Length != 90)
                    {
                        Console.WriteLine("Unexpected packet length: " + buffer.Length + "");
                    }

                    // "$ADXL,deviceId,version,battery,adcsample,sequenceId,format,sampleCount,X/Y/Z,X/Y/Z,..."

                    /*
                    @0  byte   reportType;		// [1] = 0x12 (USER_REPORT_TYPE)
                    @1  byte   reportId;	    // [1] = 0x58 (ASCII 'X')
                    @2  ushort deviceId;		// [2] (16-bit device identifier, 0 = unknown)
                    @4  byte   version;			// [1] = Packet version
                    @5  byte   battery;			// [1] = Battery value
                    @6  ushort adcSample;		// [1] = Analogue Sample
                    @8  ushort sequenceId;		// [2] (16-bit sequence counter, each packet has a new number -- reset if restarted)
                    @10 byte   format;			// [1] = 0xEA (3-axis, 2-bytes, 100Hz); Top two bits is number of axes (3), next two bits is number of bytes (2); lowest four bits is the rate code: frequency = 3200 / (1 << (15-(n & 0x0f)))
                    @11 byte   sampleCount;		// [1] = 0x11 = 17 samples (number of accelerometer samples)
                    @12+i*6 short[,] sampleData[BUFFER_SAMPLE_COUNT, 3];	// [sampleCount * numAxes * bytesPerSample = 78 -- would be 102 with 17 samples] (sample data)
                    */
                    ushort deviceId = (ushort)(buffer[2] | (((ushort)buffer[3]) << 8));
                    byte version = buffer[4];
                    byte battery = buffer[5];
                    ushort adcSample = (ushort)(buffer[6] | (((ushort)buffer[7]) << 8));
                    ushort sequenceId = (ushort)(buffer[8] | (((ushort)buffer[9]) << 8));
                    byte format = buffer[10];
                    byte sampleCount = buffer[11];
                    WaxSample[] sampleData = new WaxSample[sampleCount];

                    int expectedLength = 12 + sampleCount * 6;
                    if (buffer.Length < expectedLength)
                    {
                        Console.WriteLine("WARNING: Ignoring truncated data packet (received " + buffer.Length + " expected " + expectedLength + ").");
                    }
                    else
                    {
                        if (buffer.Length > expectedLength)
                        {
                            Console.WriteLine("WARNING: Data packet was larger than expected.");
                        }

                        StringBuilder sampleDataString = new StringBuilder();
                        for (int i = 0; i < sampleCount; i++)
                        {
                            short x = (short)((ushort)(buffer[12 + i * 6] | (((ushort)buffer[13 + i * 6]) << 8)));
                            short y = (short)((ushort)(buffer[14 + i * 6] | (((ushort)buffer[15 + i * 6]) << 8)));
                            short z = (short)((ushort)(buffer[16 + i * 6] | (((ushort)buffer[17 + i * 6]) << 8)));
                            uint index = (uint)sequenceId * sampleCount + (uint)i;
                            sampleData[i] = new WaxSample(timestamp, index, x, y, z);
                        }

                        //Console.WriteLine(line);
                        waxPacket = new WaxPacket(timestamp, deviceId, version, battery, adcSample, sequenceId, format, sampleCount, sampleData);
                    }

                }
                else if (buffer.Length >= 5 && buffer[0] == 0x12 && buffer[1] == 0x54)           // Old TEDDI (ASCII 'T'), packet 'V2'
                {
                    /*
                    unsigned char  reportType;			// @0  [1] = 0x12 (USER_REPORT_TYPE)
                    unsigned char  reportId;		    // @1  [1] = 0x54 (ASCII 'T')
                    unsigned short deviceId;			// @2  [2] = Short device identifier (16-bit) [doesn't have to be part of the payload, but this format is the same as the WAX]
                    unsigned char version;				// @4  [1] = (0x02 = format [seq/temp/ldr/audio/pir : short])
                    */
                    ushort deviceId = (ushort)(buffer[2] | (((ushort)buffer[3]) << 8));
                    byte version = buffer[4];

                    if (version == 2)
                    {
                        /*
                        unsigned char  reportType;			// @0  [1] = 0x12 (USER_REPORT_TYPE)
                        unsigned char  reportId;		    // @1  [1] = 0x54 (ASCII 'T')
                        unsigned short deviceId;			// @2  [2] = Short device identifier (16-bit) [doesn't have to be part of the payload, but this format is the same as the WAX]
                        unsigned char version;				// @4  [1] = (0x02 = format [seq/temp/ldr/audio/pir : short])
                        unsigned short sequence;			// @5  [2] = Sequence number
                        signed short temp;					// @7  [2] = Temperature
                        signed short ldr;					// @9  [2] = Light
                        unsigned short audioDC;				// @11 [2] = Microphone DC value
                        unsigned long  audioVariance;		// @13 [4] = Amplitude variance
                        unsigned short audioMean;			// @17 [2] = Amplitude mean
                        unsigned short audioMin;			// @19 [2] = Amplitude min
                        unsigned short audioMax;			// @21 [2] = Amplitude max
                        signed short pir[PIR_UPDATES_PER_WINDOW];	// [4*2 = 8] = PIR
                        */
                        byte sampleCount = 4;
                        ushort sequence = (ushort)(buffer[5] | (((ushort)buffer[6]) << 8));
                        ushort temp = (ushort)(buffer[7] | (((ushort)buffer[8]) << 8));
                        ushort ldr = (ushort)(buffer[9] | (((ushort)buffer[10]) << 8));
                        ushort audioDC = (ushort)(buffer[11] | (((ushort)buffer[12]) << 8));
                        uint audioVariance = (uint)buffer[13] | ((uint)buffer[14] << 8) | ((uint)buffer[15] << 16) | ((uint)buffer[16] << 24);
                        ushort audioMean = (ushort)(buffer[17] | (((ushort)buffer[18]) << 8));
                        ushort audioMin = (ushort)(buffer[19] | (((ushort)buffer[20]) << 8));
                        ushort audioMax = (ushort)(buffer[21] | (((ushort)buffer[22]) << 8));
                        ushort[] pir = new ushort[sampleCount];
                        pir[0] = (ushort)(buffer[23] | (((ushort)buffer[24]) << 8));
                        pir[1] = (ushort)(buffer[25] | (((ushort)buffer[26]) << 8));
                        pir[2] = (ushort)(buffer[27] | (((ushort)buffer[28]) << 8));
                        pir[3] = (ushort)(buffer[29] | (((ushort)buffer[30]) << 8));

                        WaxSample[] sampleData = new WaxSample[sampleCount];
                        for (uint i = 0; i < sampleCount; i++)
                        {
                            //sampleData[i] = new WaxSample(timestamp - TimeSpan.FromMilliseconds((sampleCount - 1 - i) * 1000 / sampleCount), (ushort)(sequence * sampleCount) + i, (short)pir[i], (short)temp, (short)ldr, Math.Sqrt(audioVariance));
                            sampleData[i] = new WaxSample(timestamp - TimeSpan.FromMilliseconds((sampleCount - 1 - i) * 1000 / sampleCount), (ushort)(sequence * sampleCount) + i, (short)pir[i], (short)temp, (short)ldr, audioVariance);
                        }
                        waxPacket = new WaxPacket(timestamp, deviceId, 1, 0, 0, (ushort)(sequence * sampleCount), 0, sampleCount, sampleData);
                    }
                    else if ((version & 0x0f) >= 0x03 && buffer.Length >= 18)
                    {
                        ushort sum = WordSum(buffer);

                        if (true) // sum == 0x0000)
                        {
                            /*
                            unsigned char  reportType;          // @ 0  [1] USER_REPORT_TYPE (0x12)
                            unsigned char  reportId;            // @ 1  [1] Report identifier (0x54, ASCII 'T')
                            unsigned short deviceId;            // @ 2  [2] Device identifier (16-bit)
                            unsigned char  version;             // @ 4  [1] Low nibble = packet version (0x3), high nibble = config (0x0)
                            unsigned char  sampleCount;         // @ 5  [1] Sample count (default config is at 250 msec interval with an equal number of PIR and audio samples; 20 = 5 seconds)
                            unsigned short sequence;            // @ 6  [2] Sequence number (16-bit)
                            unsigned short unsent;              // @ 8  [2] Number of unsent samples (default config is in 250 msec units)
                            unsigned short temp;                // @10  [2] Temperature (0.2 Hz)
                            unsigned short light;               // @12  [2] Light (0.2 Hz)
                            unsigned short battery;             // @14  [2] Battery (0.2 Hz)
                            unsigned short checksum;            // @16  [2] 16-bit checksum to make packet zero-sum
                            unsigned char  data[BITPACK10_SIZEOF(DATA_MAX_INTERVAL * 2)];   // @18 [50] PIR and audio energy (4 Hz, 20x 2x 10-bit samples)
                            */
                            byte config = (byte)(buffer[4] >> 4);
                            byte sampleCount = (byte)(buffer[5]);
                            ushort sequence = (ushort)(buffer[6] | (((ushort)buffer[7]) << 8));
                            ushort unsent = (ushort)(buffer[8] | (((ushort)buffer[9]) << 8));
                            ushort temp = (ushort)(buffer[10] | (((ushort)buffer[11]) << 8));
                            ushort light = (ushort)(buffer[12] | (((ushort)buffer[13]) << 8));
                            ushort battery = (ushort)(buffer[14] | (((ushort)buffer[15]) << 8));
                            ushort checksum = (ushort)(buffer[16] | (((ushort)buffer[17]) << 8));
                            ushort[] pir = new ushort[sampleCount];
                            ushort[] audio = new ushort[sampleCount];

                            ushort sampleInterval = 250;

                            try
                            {
                                // Unpack PIR
                                for (int i = 0; i < sampleCount; i++)
                                {
                                    pir[i] = BitUnpack_uint10(buffer, i, 18);
                                }

                                // Unpack Audio
                                for (int i = 0; i < sampleCount; i++)
                                {
                                    audio[i] = BitUnpack_uint10(buffer, sampleCount + i, 18);
                                }

                                // Divide temp/light/battery measurement down
                                if (sampleCount > 0)
                                {
                                    temp /= sampleCount;
                                    light /= sampleCount;
                                    battery /= sampleCount;
                                }

                                // Create new sample
                                WaxSample[] sampleData = new WaxSample[sampleCount];
                                for (uint i = 0; i < sampleCount; i++)
                                {
                                    sampleData[i] = new WaxSample(timestamp - TimeSpan.FromMilliseconds((unsent + sampleCount - 1 - i) * sampleInterval), (ushort)(sequence * sampleCount) + i, (short)pir[i], (short)temp, (short)light, audio[i]);
                                }
                                waxPacket = new WaxPacket(timestamp - TimeSpan.FromMilliseconds((unsent + sampleCount - 1) * sampleInterval), deviceId, 1, 0, 0, (ushort)(sequence * sampleCount), 0, sampleCount, sampleData);
                            }
                            catch (IndexOutOfRangeException ex)
                            {
                                Console.Error.Write("EXCEPTION: " + ex + " while parsing packet.");
                            }
                        }
                        else
                        {
                            Console.Error.Write("[T3-checksum]");
                        }

                    }
                    else
                    {
                        Console.Error.Write("[T?]");
                    }
                }
                else
                { 
                    Console.Error.Write("[?]"); 
                }

            }

            return waxPacket;
        }


        public byte[] ToBinary(WaxPacket value)
        { 
            throw new NotImplementedException(); 
        }


        public WaxPacket FromStomp(string value)
        {
            long timestampLong = 0;
            ushort deviceId = 0;
            byte version = 0;
            byte battery = 0;
            ushort adcSample = 0;
            ushort sequenceId = 0;
            byte format = 0;
            byte sampleCount = 0;
            short[][] sampleData = new short[0][];
            WaxSample[] samples = new WaxSample[0];

            Hashtable data = (Hashtable)Json.JsonDecode(value);

            if (data != null)
            {
                if (data.ContainsKey("Timestamp")) { timestampLong = (long)(double)data["Timestamp"]; }
                if (data.ContainsKey("DeviceId")) { deviceId = (ushort)(double)data["DeviceId"]; }
                if (data.ContainsKey("Version")) { version = (byte)(double)data["Version"]; }
                if (data.ContainsKey("Battery")) { battery = (byte)(double)data["Battery"]; }
                if (data.ContainsKey("AdcSample")) { adcSample = (ushort)(double)data["AdcSample"]; }
                if (data.ContainsKey("SequenceId")) { sequenceId = (ushort)(double)data["SequenceId"]; }
                if (data.ContainsKey("Format")) { format = (byte)(double)data["Format"]; }
                if (data.ContainsKey("SampleCount")) { sampleCount = (byte)(double)data["SampleCount"]; }
                if (data.ContainsKey("Samples") && data["Samples"] is ArrayList)
                {
                    ArrayList sampleList = (ArrayList)data["Samples"];
                    samples = new WaxSample[sampleList.Count];
                    for (int i = 0; i < sampleList.Count; i++)
                    {
                        if (sampleList[i] is ArrayList && ((ArrayList)sampleList[i]).Count >= 5)
                        {
                            ArrayList al = (ArrayList)sampleList[i];

                            long ts = 0;
                            uint idx = 0;
                            short x = 0, y = 0, z = 0;

                            ts = (long)(double)al[0];
                            idx = (uint)(double)al[1];
                            x = (short)(double)al[2];
                            y = (short)(double)al[3];
                            z = (short)(double)al[4];

                            samples[i] = new WaxSample(DateTime.FromBinary(ts), idx, x, y, z);
                        }
                    }
                }
            }
            else
            {
                Console.Error.Write("[STOMP:null]");
            }

            return new WaxPacket(DateTime.FromBinary(timestampLong), deviceId, version, battery, adcSample, sequenceId, format, sampleCount, samples);
        }


        public string ToStomp(WaxPacket packet)
        {
            Hashtable sourceData = new Hashtable();

            object[] samples = new object[packet.Samples.Length];
            for (int i = 0; i < packet.Samples.Length; i++)
            {
                WaxSample sample = packet.Samples[i];
                object[] sampleArray = new object[5];
                sampleArray[0] = sample.Timestamp.ToBinary();
                sampleArray[1] = sample.Index;
                sampleArray[2] = sample[0];
                sampleArray[3] = sample[1];
                sampleArray[4] = sample[2];
                samples[i] = sampleArray;
            }

            sourceData.Add("Timestamp", packet.Timestamp.ToBinary());
            sourceData.Add("DeviceId", packet.DeviceId);
            sourceData.Add("Version", packet.Version);
            sourceData.Add("Battery", packet.Battery);
            sourceData.Add("AdcSample", packet.AdcSample);
            sourceData.Add("SequenceId", packet.SequenceId);
            sourceData.Add("Format", packet.Format);
            sourceData.Add("SampleCount", packet.SampleCount);
            sourceData.Add("Samples", samples);

            string json = Json.JsonEncode(sourceData);
            return json;
        }

        public WaxPacket FromOscBundle(OscBundle oscBundle, string topic) 
        {
            List<WaxSample> sampleData = new List<WaxSample>();
            ushort deviceId = 0xffff;
            ulong timestamp = oscBundle.Timestamp;
            foreach (OscData oscData in oscBundle.Parts)
            {
                OscMessage oscMessage = oscData as OscMessage;
                if (oscMessage == null) { continue; }

                string t = (topic == null ? "/wax" : topic);
                if (oscMessage.Address != t) { continue; }
                if (oscMessage.Arguments.Length < 5) { continue; }

                uint index;
                short x, y, z;
                deviceId = (ushort)(int)oscMessage.Arguments[0];
                index = (uint)(int)oscMessage.Arguments[1];
                x = (short)((float)oscMessage.Arguments[2] * 256.0f);
                y = (short)((float)oscMessage.Arguments[3] * 256.0f);
                z = (short)((float)oscMessage.Arguments[4] * 256.0f);

                WaxSample waxSample = new WaxSample(OscMessage.DateTimeFromTimestamp(timestamp), index, x, y, z);
Console.WriteLine(waxSample.ToString());
                sampleData.Add(waxSample);
            }

            byte version = 0;
            byte battery = 0;
            ushort adcSample = 0;
            ushort sequenceId = 0xffff;     // TODO: Fix this to something sensible
            byte format = 0;
            byte sampleCount = (byte)sampleData.Count;

            return new WaxPacket(OscMessage.DateTimeFromTimestamp(timestamp), deviceId, version, battery, adcSample, sequenceId, format, sampleCount, sampleData.ToArray());

        }

        public OscBundle ToOscBundle(WaxPacket packet, string topic) 
        {
            WaxSample[] samples = packet.Samples;
            OscMessage[] messages = new OscMessage[samples.Length];
            for (int i = 0; i < samples.Length; i++)
            {
                messages[i] = new OscMessage((topic == null ? "/wax" : topic), (int)packet.DeviceId, samples[i].Index, samples[i].X / 256.0f, samples[i].Y / 256.0f, samples[i].Z / 256.0f);
            }
            OscBundle bundle = new OscBundle(OscBundle.TIMESTAMP_NOW, messages);
            return bundle;
        }

        public string ToLog(WaxPacket packet) 
        {
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < packet.Samples.Length; i++)
            {
                const string DATE_TIME_FORMAT = "yyyy-MM-dd HH:mm:ss.fff";
                sb.Append("ACCEL,");
                sb.Append(packet.Samples[i].Timestamp.ToString(DATE_TIME_FORMAT)).Append(",");
                sb.Append(packet.DeviceId).Append(",");
                sb.Append(packet.Samples[i].Index).Append(",");
                sb.Append((float)packet.Samples[i].X / 256.0f).Append(",");
                sb.Append((float)packet.Samples[i].Y / 256.0f).Append(",");
                sb.Append((float)packet.Samples[i].Z / 256.0f).Append("");
                sb.AppendLine();
            }
            return sb.ToString();
        }

        public string GetTopicPostfix(WaxPacket packet)
        {
            //return null;
            return packet.DeviceId.ToString();
        }

        public string GetDefaultTopic(bool receiving)
        {
            return "/topic/Kitchen.Sensor.Wax" + (receiving ? ".>" : "");
        }

    }
}
