using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;

namespace TeddiGui
{
    public class Packet
    {
	    // Packet data
        public DateTime Timestamp { get; private set; }
        public DateTime RawTimestamp { get; private set; }
        public ushort DeviceId { get; private set; }
        public byte Version { get; private set; }
        public ushort Battery { get; private set; }
        public ushort AdcSample { get; private set; }
        public ushort SequenceId { get; private set; }
        public byte Format { get; private set; }
        public ushort SampleCount { get; private set; }
        public int Unsent { get; private set; }
        public int Temp { get; private set; }
        public int Light { get; private set; }
        public int Humidity { get; private set; }
        public ushort[] Pir { get; private set; }
        public ushort[] Audio { get; private set; }
        public ushort ParentAddress { get; private set; }
        public ushort ParentAltAddress { get; private set; }

		// For the graph
        public Sample[] Samples { get; private set; }
        
		
        // Construct a Packet
        public Packet(DateTime timestamp, ushort deviceId, byte version, ushort battery, ushort adcSample, ushort sequenceId, byte format, ushort sampleCount, DateTime rawTimestamp, int unsent, int temp, int light, int humidity, ushort[] pir, ushort[] audio, Sample[] samples, ushort parentAddress, ushort parentAltAddress)
        {
            Timestamp = timestamp;
            DeviceId = deviceId;
            Version = version;
            Battery = battery;
            AdcSample = adcSample;
            SequenceId = sequenceId;
            Format = format;
            SampleCount = sampleCount;
            RawTimestamp = rawTimestamp;
            Unsent = unsent;
            Temp = temp;
            Light = light;
            Humidity = humidity;
			Pir = pir;
			Audio = audio;
			
            Samples = samples;

            ParentAddress = parentAddress;
            ParentAltAddress = parentAltAddress;
        }

		
		// ToString()
        public override string ToString()
        {
            const string DATE_TIME_FORMAT = "yyyy-MM-dd HH:mm:ss.fff";
			long secs = (RawTimestamp.Ticks - 621355968000000000) / 10000;

			StringBuilder sb = new StringBuilder();
			sb.Append("TEDDI");
			sb.Append(",").Append(secs);             // epoch-timestamp, 
            sb.Append(",").Append(RawTimestamp.ToString(DATE_TIME_FORMAT));        // timestamp,
			sb.Append(",").Append(DeviceId);         // deviceId, 
			sb.Append(",").Append(Version);          // version,
			sb.Append(",").Append(SampleCount);      // sampleCount, 
			sb.Append(",").Append(SequenceId);       // sequence, 
			sb.Append(",").Append(Unsent);           // unsent,
			sb.Append(",").Append(Temp);             // temp, 
			sb.Append(",").Append(Light);            // light, 
			sb.Append(",").Append(Battery);          // battery, 
			sb.Append(",").Append(Humidity);         // humidity,

			// pirData[sampleCount]
			for (int i = 0; i < Pir.Length; i++)
			{
				sb.Append(",").Append(Pir[i]);
			}

			// audioData[sampleCount]
			for (int i = 0; i < Audio.Length; i++)
			{
				sb.Append(",").Append((int)(Audio[i]));
			}

			return sb.ToString();
        }

		
		// Calculates number of bytes required for a 10-bit packed array of the specified length
        private static int BITPACK10_SIZEOF(int n) { return (((n / 4) * 5) + (((n & 0x03) == 0) ? 0 : ((n & 0x03) + 1))); }

		
        // Un-pack 4x 10-bit samples from each 5-byte group (stored little-endian).
        private static ushort BitUnpack_uint10(byte[] buffer, int index, int offset = 0)
        {
            int o = offset + ((index >> 2) * 5);
            if (o < 0 || o + (index & 0x03) + 1 >= buffer.Length) { throw new IndexOutOfRangeException(); }
            switch (index & 0x03)
            {
                case 0: return (ushort)((ushort)(buffer[o + 0]) | (ushort)((buffer[o + 1] & 0x0003) << 8));    // A
                case 1: return (ushort)((ushort)(buffer[o + 1] >> 2) | (ushort)((buffer[o + 2] & 0x000f) << 6));    // B
                case 2: return (ushort)((ushort)(buffer[o + 2] >> 4) | (ushort)((buffer[o + 3] & 0x003f) << 4));    // C
                case 3: return (ushort)((ushort)(buffer[o + 3] >> 6) | (ushort)((buffer[o + 4]) << 2));    // D
            }
            throw new IndexOutOfRangeException();
        }

		
        // Method to return a Packet, or null if invalid byte array
        public static Packet PacketFromBinary(byte[] buffer, DateTime timestamp)
        {
            Packet packet = null;

            if (buffer != null && buffer.Length > 0)
            {
                if (buffer.Length >= 5 && buffer[0] == 0x12 && buffer[1] == 0x54)           // USER_REPORT_TYPE && TEDDI (ASCII 'T')
                {
                    /*
                    unsigned char  reportType;			// @0  [1] = 0x12 (USER_REPORT_TYPE)
                    unsigned char  reportId;		    // @1  [1] = 0x54 (ASCII 'T')
                    unsigned short deviceId;			// @2  [2] = Short device identifier (16-bit) [doesn't have to be part of the payload, but this format is the same as the WAX]
                    unsigned char version;				// @4  [1] = (0x02 = format [seq/temp/ldr/audio/pir : short])
                    */
                    ushort deviceId = (ushort)(buffer[2] | (((ushort)buffer[3]) << 8));
                    byte version = buffer[4];

                    if (((version & 0x0f) == 0x03 || (version & 0x0f) >= 0x04) && buffer.Length >= 18)
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
                        unsigned short humidity;            // @16  [2] Humidity [V4] (0.2 Hz) -- or [V3] 16-bit checksum to make packet zero-sum
                        unsigned char  data[BITPACK10_SIZEOF(DATA_MAX_INTERVAL * 2)];   // @18 [50] PIR and audio energy (4 Hz, 20x 2x 10-bit samples)
                        unsigned short parentAddress;		// @ADDITIONAL_OFFSET+0  [2] (optional) Parent address
                        unsigned short parentAltAddress;	// @ADDITIONAL_OFFSET+2  [2] (optional) Parent alt. address
                        */
                        byte config = (byte)(buffer[4] >> 4);
						byte sampleCount = (byte)(buffer[5]);
						ushort sequence = (ushort)(buffer[6] | (((ushort)buffer[7]) << 8));
						ushort unsent = (ushort)(buffer[8] | (((ushort)buffer[9]) << 8));
						ushort temp = (ushort)(buffer[10] | (((ushort)buffer[11]) << 8));
						ushort light = (ushort)(buffer[12] | (((ushort)buffer[13]) << 8));
						ushort battery = (ushort)(buffer[14] | (((ushort)buffer[15]) << 8));
						ushort humidity = (ushort)(buffer[16] | (((ushort)buffer[17]) << 8));
						ushort[] pir = new ushort[sampleCount];
						ushort[] audio = new ushort[sampleCount];
                        ushort parentAddress = 0xffff;
                        ushort parentAltAddress = 0xffff;

						if ((version & 0x0f) == 0x03)
						{
							humidity = 0;
						}

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

							// For V3 data, divide temp/light/battery measurement down
                            if ((version & 0x0f) <= 0x03 && sampleCount > 0)
							{
								temp /= sampleCount;
								light /= sampleCount;
								battery /= sampleCount;
								humidity /= sampleCount;
							}

                            // Check for additional data
                            int additionalIndex = 18 + BITPACK10_SIZEOF(sampleCount * 2);
                            if (additionalIndex + 4 <= buffer.Length)
                            {
                                parentAddress = (ushort)(buffer[additionalIndex + 0] | (((ushort)buffer[additionalIndex + 1]) << 8));
                                parentAltAddress = (ushort)(buffer[additionalIndex + 2] | (((ushort)buffer[additionalIndex + 3]) << 8));
                                Console.WriteLine("SENSOR: #" + deviceId + ", parent " + parentAddress + " (" + parentAltAddress + " = #" + Device.RouterAltId(parentAltAddress) + ").");
                            }

							// Create samples from the measurement data: deviceId, temp, light, humidity, pir[], audio[]
							Sample[] sampleData = new Sample[sampleCount];
							for (uint i = 0; i < sampleCount; i++)
							{
								sampleData[i] = new Sample(timestamp - TimeSpan.FromMilliseconds((unsent + sampleCount - 1 - i) * sampleInterval), (ushort)(sequence * sampleCount) + i, (short)pir[i], (short)temp, (short)light, (short)humidity, audio[i], battery);
							}
							packet = new Packet(timestamp - TimeSpan.FromMilliseconds((unsent + sampleCount - 1) * sampleInterval), deviceId, 1, 0, 0, (ushort)(sequence * sampleCount), 0, sampleCount, timestamp, unsent, temp, light, humidity, pir, audio, sampleData, parentAddress, parentAltAddress);
						}
						catch (IndexOutOfRangeException ex)
						{
							Console.Error.Write("EXCEPTION: " + ex + " while parsing packet.");
						}
                    }
                    else
                    {
                        Console.Error.Write("[T?]");	// Unknown TEDDI packet type
                    }
                }
                else
                {
                    //Console.Error.Write("[?]");		// Unknown packet type
                }
            }
            return packet;
        }

		
/*
        // The time is this time divided by number of ticks per millisecond
        protected static long time() { return DateTime.UtcNow.Ticks / TimeSpan.TicksPerMillisecond; }

		
        public Sample? LastValidSample
        {
            get
            {
                Sample? lastValidSample = null;
                foreach (Sample sample in Samples)
                {
                    if (sample.valid)
                    {
                        lastValidSample = sample;
                    }
                }
                return lastValidSample;
            }
		}
		
		
        // Checksum - 16-bit word-size addition
        private static ushort WordSum(byte[] buffer)
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
*/


        public static Packet PacketFromStomp(Hashtable data)
        {
            long timestampReceivedLong = 0;
            long timestampEstimatedLong = 0;
            ushort deviceId = 0;
            byte version = 0;
            ushort battery = 0;
            ushort sequence = 0;
            ushort unsent = 0;
            ushort temp = 0;
            ushort light = 0;
            ushort humidity = 0;
            ushort sampleCount = 0;
            ushort[] pir = new ushort[0];
            ushort[] audio = new ushort[0];
            Sample[] samples = new Sample[0];
            ushort parentAddress = 0xffff;
            ushort parentAltAddress = 0xffff;

            // Check for "TEDDI" packet type, or no type info (old packets), 
            if (data != null && (!data.ContainsKey("Type") || data["Type"].ToString() == "TEDDI"))
            {
                if (data.ContainsKey("TimestampEstimated")) { timestampEstimatedLong = (long)(double)Double.Parse(data["TimestampEstimated"].ToString()); }
                if (data.ContainsKey("TimestampReceived")) { timestampReceivedLong = (long)(double)Double.Parse(data["TimestampReceived"].ToString()); }
                if (data.ContainsKey("DeviceId")) { deviceId = (ushort)(double)Double.Parse(data["DeviceId"].ToString()); }
                if (data.ContainsKey("Version")) { version = (byte)(double)Double.Parse(data["Version"].ToString()); }
                if (data.ContainsKey("SampleCount")) { sampleCount = (ushort)(double)Double.Parse(data["SampleCount"].ToString()); }
                if (data.ContainsKey("Sequence")) { sequence = (ushort)(double)Double.Parse(data["Sequence"].ToString()); }
                if (data.ContainsKey("Unsent")) { unsent = (ushort)(double)Double.Parse(data["Unsent"].ToString()); }

                if (data.ContainsKey("Temp")) { temp = (ushort)(double)Double.Parse(data["Temp"].ToString()); }
                if (data.ContainsKey("Light")) { light = (ushort)(double)Double.Parse(data["Light"].ToString()); }
                if (data.ContainsKey("Battery")) { battery = (ushort)(double)Double.Parse(data["Battery"].ToString()); }
                if (data.ContainsKey("Humidity")) { humidity = (ushort)(double)Double.Parse(data["Humidity"].ToString()); }

                if (data.ContainsKey("Samples") && data["Samples"] is ArrayList)
                {
                    ArrayList sampleList = (ArrayList)data["Samples"];
                    samples = new Sample[sampleList.Count];
                    pir = new ushort[sampleList.Count];
                    audio = new ushort[sampleList.Count];
                    for (int i = 0; i < sampleList.Count; i++)
                    {
                        if (sampleList[i] is ArrayList && ((ArrayList)sampleList[i]).Count >= 3)
                        {
                            ArrayList al = (ArrayList)sampleList[i];

                            long ts = 0;
                            uint idx = 0;
                            pir[i] = 0;
                            audio[i] = 0;

                            ts = (long)(double)al[0];
                            idx = (uint)(sequence * sampleCount + i);
                            pir[i]= (ushort)(double)al[1];
                            audio[i] = (ushort)(double)al[2];

						    samples[i] = new Sample(DateTime.FromBinary(ts), (uint)idx, (short)pir[i], (short)temp, (short)light, (short)humidity, (ushort)audio[i], battery);

                        }
                    }
                }

                if (data.ContainsKey("ParentAddress")) { parentAddress = (ushort)(double)Double.Parse(data["ParentAddress"].ToString()); }
                if (data.ContainsKey("ParentAltAddress")) { parentAltAddress = (ushort)(double)Double.Parse(data["ParentAltAddress"].ToString()); }

            }
            else
            {
                Console.Error.Write("[STOMP:null]");
            }

            return new Packet(DateTime.FromBinary(timestampEstimatedLong), deviceId, version, battery, 0, sequence, 0, sampleCount, DateTime.FromBinary(timestampReceivedLong), unsent, temp, light, humidity, pir, audio, samples, parentAddress, parentAltAddress);
            
        }


        /*
        public static string PacketToStomp(Packet packet)
        {
            Hashtable sourceData = new Hashtable();

            object[] samples = new object[packet.Samples.Length];
            for (int i = 0; i < packet.Samples.Length; i++)
            {
                Sample sample = packet.Samples[i];
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
        */

        /*
        public static Packet PacketFromOscBundle(OscBundle oscBundle, string topic)
        {
            List<Sample> sampleData = new List<Sample>();
            ushort deviceId = 0xffff;
            ulong timestamp = oscBundle.Timestamp;
            foreach (OscData oscData in oscBundle.Parts)
            {
                OscMessage oscMessage = oscData as OscMessage;
                if (oscMessage == null) { continue; }

                string t = (topic == null ? "/teddi" : topic);
                if (oscMessage.Address != t) { continue; }
                if (oscMessage.Arguments.Length < 5) { continue; }

                uint index;
                short x, y, z;
                deviceId = (ushort)(int)oscMessage.Arguments[0];
                index = (uint)(int)oscMessage.Arguments[1];
                x = (short)((float)oscMessage.Arguments[2] * 256.0f);
                y = (short)((float)oscMessage.Arguments[3] * 256.0f);
                z = (short)((float)oscMessage.Arguments[4] * 256.0f);

                Sample waxSample = new Sample(OscMessage.DateTimeFromTimestamp(timestamp), index, x, y, z);
                Console.WriteLine(waxSample.ToString());
                sampleData.Add(waxSample);
            }

            byte version = 0;
            byte battery = 0;
            ushort adcSample = 0;
            ushort sequenceId = 0xffff;     // TODO: Fix this to something sensible
            byte format = 0;
            byte sampleCount = (byte)sampleData.Count;

            return new Packet(OscMessage.DateTimeFromTimestamp(timestamp), deviceId, version, battery, adcSample, sequenceId, format, sampleCount, sampleData.ToArray());
        }
        */

        public static OscBundle PacketToOscBundle(Packet packet, string topic, bool cooked)
        {
            if (topic == null || topic.Length == 0)
            {
                topic = cooked ? "/teddi-cooked/@" : "/teddi/@";
            }

            Sample[] samples = packet.Samples;
            OscMessage[] messages = new OscMessage[samples.Length];
            for (int i = 0; i < samples.Length; i++)
            {
                string t = topic;
                t = t.Replace("@", "" + packet.DeviceId);

                ushort rawPir = (ushort)samples[i].V;
                ushort rawAudio = (ushort)samples[i].X;

                if (cooked)
                {
                    float temp = Sample.ConvertC((ushort)packet.Temp);
                    float light = Sample.ConvertLux((ushort)packet.Light);
                    float battery = Sample.ConvertV((ushort)packet.Battery);
                    float humidity = Sample.ConvertPercentage((ushort)packet.Humidity);
                    float pir = Sample.ConvertV(rawPir);
                    float audio = Sample.ConvertV(rawAudio);
                    messages[i] = new OscMessage(t, packet.DeviceId, temp, light, battery, humidity, pir, audio, samples[i].Index);
                }
                else
                {
                    messages[i] = new OscMessage(t, packet.DeviceId, packet.Temp, packet.Light, packet.Battery, packet.Humidity, rawPir, rawAudio, samples[i].Index);
                }
            }

            OscBundle bundle = new OscBundle(OscBundle.TIMESTAMP_NOW, messages);
            return bundle;
        }


    }
}
