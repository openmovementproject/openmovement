using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace TeddiGui
{
    public struct Sample
    {
        public bool valid;
        private DateTime timestamp;
        private uint index;
        private short[] values;
        private float v;
        private ushort batt;

        public Sample(uint index)
        {
            this.index = index;
            this.timestamp = DateTime.MinValue;
            valid = false;
            values = new short[4];
            values[0] = 0;
            values[1] = 0;
            values[2] = 0;
            values[3] = 0;
            v = 0.0f;
            batt = 0;
        }

        public Sample(DateTime timestamp, uint index, short x, short y, short z, short h, ushort w, ushort batt)
        {
            this.index = index;
            this.timestamp = timestamp;
            valid = true;
            values = new short[4];
            values[0] = x;
            values[1] = y;
            values[2] = z;
            values[3] = h;
            v = (float)w;
            this.batt = batt;
        }


        private static short FromUShort(ushort v) { return (short)(v <= 32767 ? v : 32767); }

        public uint Index { get { return index; } set { index = value; } }
        public DateTime Timestamp { get { return timestamp; } set { timestamp = value; } }
        public short X { get { return valid ? values[0] : (short)0; } }
        public short Y { get { return valid ? values[1] : (short)0; } }
        public short Z { get { return valid ? values[2] : (short)0; } }
        public float V { get { return v; } set { v = value; } }
        public short H { get { return valid ? values[3] : (short)0; } }
        public short this[int index] { get { return values[index]; } }

        public override String ToString()
        {
            return "<" + Index + ", " + X + "," + Y + "," + Z + ">";
        }

        public float Temperature()
        {
            return ConvertC((ushort)this.Y);
        }

        public float Humidity()
        {
            return ConvertPercentage((ushort)H);
        }

        public float BattVolts()
        {
            return ConvertV(batt);
        }

        public float BattPercent()
        {
            // A three-point linear fit (based on code and values calculated by KL)
            const float BATT_FIT_CONST_1 = 409;
            const float BATT_FIT_CONST_2 = 10;
            const float BATT_FIT_CONST_3 = 450;        
            const float BATT_FIT_CONST_4 = 80;
            const float BATT_FIT_CONST_5 = 573;
            const float BATT_FIT_CONST_6 = 10;
            const float BATT_FIT_CONST_7 = 614;
            float value;

            if (batt >= BATT_FIT_CONST_7)
                value = 100;
            else if (batt >= BATT_FIT_CONST_5)
                value = (BATT_FIT_CONST_6 * (batt - BATT_FIT_CONST_5)) / (BATT_FIT_CONST_1 - BATT_FIT_CONST_5) + BATT_FIT_CONST_4 + BATT_FIT_CONST_2;
            else if (batt >= BATT_FIT_CONST_3)
                value = (BATT_FIT_CONST_4 * (batt - BATT_FIT_CONST_3)) / (BATT_FIT_CONST_5 - BATT_FIT_CONST_3) + BATT_FIT_CONST_2;
            else if (batt >= BATT_FIT_CONST_1)
                value = (BATT_FIT_CONST_2 * (batt - BATT_FIT_CONST_1)) / (BATT_FIT_CONST_3 - BATT_FIT_CONST_1);
            else
                value = 0;

            return value;
        }



        // Convert temperature to centi-degrees C (0.01 C)
        private static short Humidicon_Convert_centiC(ushort value)
        {
	        // 14-bit (0 - 16383)
            if (value == 0xffff || value >= 16384) { return 0; }
            return (short)(((16500UL * value + 8191) / 16384) - 4000);
        }

        // Convert humidity in 1/100 of a percent (0.01 %)
        private static ushort Humidicon_Convert_percentage(ushort value)
        {
	        // 14-bit (0 - 16383)
	        if (value == 0xffff || value >= 16384) { return 0; }
	        return (ushort)((10000UL * value + 8191) / 16384);
        }

        public static float ConvertPercentage(ushort value)
        {
            return Humidicon_Convert_percentage(value) / 100.0f;
        }

        public static float ConvertC(ushort value)
        {
            if ((value & 0x8000) == 0)
            {
                return ((((value * 2500.0f) + 512.0f) / 1024.0f) - 500.0f) / 10.0f;
            }
            else
            {
                return Humidicon_Convert_centiC((ushort)(0x7fff & value)) / 100.0f;
            }
        }

        public static float ConvertV(ushort value)
        {
            return value * 2.5f / 1024.0f;
        }

        public static float ConvertLux(ushort value)
        {
            float log10Lux = value * 2.5f / 1024.0f;
            return (float)Math.Pow(10, log10Lux);
        }


    }
}
