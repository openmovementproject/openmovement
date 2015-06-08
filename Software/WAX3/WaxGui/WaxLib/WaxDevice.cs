using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace WaxLib
{
    public class WaxDevice
    {
        public const int DEFAULT_MAX_HISTORY = (50 * 60 * 1);    // 1 minute at 50 Hz
        public int DeviceId { get; private set; }
        public DateTime CreationTime { get; private set; }
        public DateTime UpdateTime { get; private set; }
        public List<WaxSample> Samples { get; private set; }
        public int MaxHistory { get; private set; }
        public int LastIndex { get; set; }


        public WaxDevice(int deviceId, DateTime creationTime)
        {
            DeviceId = deviceId;
            CreationTime = creationTime;
            UpdateTime = CreationTime;
            MaxHistory = DEFAULT_MAX_HISTORY;
            Samples = new List<WaxSample>();
            Clear();
        }

        public void Clear()
        {
            Samples.Clear();
            LastIndex = -1;
        }

        public void AddSamples(WaxSample[] samples)
        {
            UpdateTime = DateTime.UtcNow;

            if (samples == null || samples.Length == 0) { return; }

            // If the last index of the new samples is the same as the last index seen -- duplicate packet (ignore)
            if ((int)samples[samples.Length - 1].Index == LastIndex && LastIndex != 0)
            {
                Console.WriteLine("WARNING: Duplicate packet detected (last index: " + LastIndex + ") - ignoring.");
                return; 
            }

            // If not a duplicate, and the first index of the new samples is earlier than the last index seen -- must be restart
            if ((int)samples[0].Index < LastIndex && LastIndex != 0)
            {
                // Must have re-started broadcast, clear history
//if (LastIndex - (int)samples[0].Index > 20)
                Clear();
                LastIndex = (int)samples[0].Index - 1;
            }

            // Calculate number of missed packets
            int numMissed = (int)(samples[0].Index - LastIndex - 1);
            if (numMissed < 0) { numMissed = 0; }
            if (numMissed > 1000) { numMissed = 1000; }

            if (numMissed > 0)
            {
                Console.WriteLine("WARNING: Dropped packet(s) detected, missing samples: " + numMissed);
            }

            // Add any missing samples
            WaxSample[] missedSamples = new WaxSample[numMissed];
            for (int i = 0; i < missedSamples.Length; i++)
            {
                missedSamples[i] = new WaxSample((uint)++LastIndex);
            }
            Samples.AddRange(missedSamples);

            // Add samples
            int firstIndex = Samples.Count;
            int length = samples.Length;
            Samples.AddRange(samples);

            // Update last index
            LastIndex = (int)(Samples[Samples.Count - 1].Index);

            // Trim to maximum samples range
            if (Samples.Count > MaxHistory)
            {
                firstIndex -= Samples.Count - MaxHistory;
                if (firstIndex < 0) { length += firstIndex; firstIndex = 0; }
                Samples.RemoveRange(0, Samples.Count - MaxHistory);
            }
        }

    }
}
