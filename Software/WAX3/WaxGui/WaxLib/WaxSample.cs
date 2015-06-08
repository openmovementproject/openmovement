using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace WaxLib
{
    public struct WaxSample
    {
        public bool valid;
        private DateTime timestamp;
        private uint index;
        private short[] values;
        private float v;

        public WaxSample(uint index)
        {
            this.index = index;
            this.timestamp = DateTime.MinValue;
            valid = false;
            values = new short[3];
            values[0] = 0;
            values[1] = 0;
            values[2] = 0;
            v = (float)Math.Sqrt((float)values[0] * values[0] + (float)values[1] * values[1] + (float)values[2] * values[2]);
        }

        public WaxSample(DateTime timestamp, uint index, short x, short y, short z)
        {
            this.index = index;
            this.timestamp = timestamp;
            valid = true;
            values = new short[3];
            values[0] = x;
            values[1] = y;
            values[2] = z;
            v = (float)Math.Sqrt((float)x * x + (float)y * y + (float)z * z);
        }

        private static short FromUShort(ushort v) { return (short)(v <= 32767 ? v : 32767); }

        // HACK: For 4-channel non-accelerometer data
        public WaxSample(DateTime timestamp, uint index, short x, short y, short z, double w) : this(timestamp, index, x, y, z)
        {
            v = (float)w;
        }

        public uint Index { get { return index; } set { index = value; } }
        public DateTime Timestamp { get { return timestamp; } set { timestamp = value; } }
        public short X { get { return valid ? values[0] : (short)0; } }
        public short Y { get { return valid ? values[1] : (short)0; } }
        public short Z { get { return valid ? values[2] : (short)0; } }
        public float V { get { return v; } set { v = value; } }
        public short this[int index] { get { return values[index]; } }

        public override String ToString()
        {
            return "<" + Index + ", " + X + "," + Y + "," + Z + ">";
        }
    }
}
