using System;
using System.Collections.Generic;
using System.Text;

namespace WaxLib
{

    public class WaxPacket
    {
        public DateTime Timestamp { get; private set; }
        public ushort DeviceId { get; private set; }
        public byte Version { get; private set; }
        public byte Battery { get; private set; }
        public ushort AdcSample { get; private set; }
        public ushort SequenceId { get; private set; }
        public byte Format { get; private set; }
        public byte SampleCount { get; private set; }
        public WaxSample[] Samples { get; private set; }

        // Construct a WaxPacket
        public WaxPacket(DateTime timestamp, ushort deviceId, byte version, byte battery, ushort adcSample, ushort sequenceId, byte format, byte sampleCount, WaxSample[] samples)
        {
            Timestamp = timestamp;
            DeviceId = deviceId;
            Version = version;
            Battery = battery;
            AdcSample = adcSample;
            SequenceId = sequenceId;
            Format = format;
            SampleCount = sampleCount;
            Samples = samples;
        }

        // the time is this time divided by number of ticks per millisecond
        //protected static long time() { return DateTime.UtcNow.Ticks / TimeSpan.TicksPerMillisecond; }



        public override string ToString()
        {
            // "$ADXL,deviceId,version,battery,adcsample,sequenceId,format,sampleCount,X/Y/Z,X/Y/Z,..."
            StringBuilder sb = new StringBuilder();
            sb.Append("$ADXL," + DeviceId + "," + Version + "," + Battery + "," + AdcSample + "," + SequenceId + "," + Format + "," + SampleCount);
            for (int i = 0; i < SampleCount; i++)
            {
                sb.Append(",").Append(Samples[i].X).Append("/").Append(Samples[i].Y).Append("/").Append(Samples[i].Z);
            }
            return sb.ToString();
        }


        public WaxSample? LastValidSample
        {
            get
            {
                WaxSample? lastValidSample = null;
                foreach (WaxSample sample in Samples)
                {
                    if (sample.valid)
                    {
                        lastValidSample = sample;
                    }
                }
                return lastValidSample;
            }
        }

    }
}
