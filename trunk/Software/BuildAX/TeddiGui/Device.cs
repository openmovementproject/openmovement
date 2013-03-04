using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace TeddiGui
{
    public class Device
    {
        public const int MAX_HISTORY = 2 * 60 * 4;

        public bool Sensor { get; protected set; }

        public int Id { get; protected set; }
        private List<Sample> _samples = new List<Sample>();
        public List<Sample> Samples { get { return _samples; } }

        public DateTime UpdateTime { get; private set; }
        public int MaxHistory { get; private set; }
        public int LastIndex { get; set; }


        public Device(int id, bool sensor)
        {
            Id = id;
            Sensor = sensor;
            Clear();
        }

        public void Clear()
        {
            Samples.Clear();
            LastIndex = -1;
            MaxHistory = MAX_HISTORY;
        }

        public void AddSamples(Sample[] samples)
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
                Console.WriteLine("WARNING: Restart detected (index: " + samples[0].Index + ", last index: " + LastIndex + ") - clearing history.");
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
            Sample[] missedSamples = new Sample[numMissed];
            for (int i = 0; i < missedSamples.Length; i++)
            {
                missedSamples[i] = new Sample((uint)++LastIndex);
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


        public static ushort RouterAltId(ushort altAddress)
        {
            if ((altAddress & 0xff) == 0) { return (ushort)(altAddress >> 8); }
            Console.WriteLine("WARNING: Router alt. address expected to have low 8-bits clear.");
            return altAddress;
        }

        ushort shortAddress = 0xffff;
        public ushort ShortAddress { get { return shortAddress; } }

        ushort parentAddress = 0xffff;
        public ushort ParentAddress { get { return parentAddress; } set { parentAddress = value; } }

        int parentLQI = -1;
        public int ParentLQI { get { return parentLQI; } }

        int parentRSSI = -1;
        public int ParentRSSI { get { return parentRSSI; } }

        ushort[] neighbours = null;
        public ushort[] NeighbourShortAddresses { get { return neighbours; } }

        public void AddStatus(StatusPacket statusPacket)
        {
            UpdateTime = DateTime.UtcNow;

            this.parentAddress = statusPacket.parentAddress;
            this.shortAddress = statusPacket.shortAddress;
            this.parentLQI = statusPacket.lastLQI;
            this.parentRSSI = statusPacket.lastRSSI;
            neighbours = (ushort[])statusPacket.neighbours.Clone();
        }


        public float DataAge
        {
            get
            {
                if (UpdateTime == DateTime.MinValue) { return -1.0f; }
                return (float)(DateTime.UtcNow - UpdateTime).TotalSeconds;
            }
        }

    }
}
