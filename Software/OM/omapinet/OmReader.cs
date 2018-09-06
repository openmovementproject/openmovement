using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace OmApiNet
{
    public class OmReader : OmSource
    {
        protected Om om;

        // Properties
        public override SourceCategory Category { get { return SourceCategory.File; } }

        private uint deviceId;
        public override uint DeviceId { get { EnsureMetadataRead(); return deviceId; } }

        private uint sessionId;
        public override uint SessionId { get { return sessionId; } }

        public uint OpenDeviceId { get; protected set; }
        public string Filename { get; protected set; }
        public IntPtr Handle { get; protected set; }
        public DateTime StartTime { get; protected set; }
        public DateTime EndTime { get; protected set; }
        public int DataBlockSize { get; protected set; }
        public int DataOffsetBlocks { get; protected set; }
        public int DataNumBlocks { get; protected set; }

        private string metadata = null;
        private void EnsureMetadataRead()
        {
            // Check if already read
            if (metadata != null) { return; }

            // Get metadata
            int deviceIdInt = 0;
            metadata = OmApi.OmReaderMetadata(Handle, out deviceIdInt, out sessionId);
            deviceId = (ushort)deviceIdInt;
        }


        public string MetaData { get { EnsureMetadataRead(); return metadata; } protected set { metadata = value; } }

        public int Channels { get; protected set; }
        public uint SequenceId { get; set; }
        public int AccelOneG { get; set; }  // 256
        public int GyroRange { get; set; }  // 0, 2000, 1000, 500, 250, 125
        public float Light { get; protected set; }
        public float Temp { get; protected set; }
        public float Batt { get; protected set; }
        public float BattRaw { get; protected set; }

        // Base constructor
        protected OmReader(Om om, IntPtr handle)
        {
            this.om = om;
            Handle = handle;

            // Get data range
            int dataBlockSize = 0, dataOffsetBlocks = 0, dataNumBlocks = 0;
            uint startTime = 0, endTime = 0;
            OmApi.OmReaderDataRange(handle, out dataBlockSize, out dataOffsetBlocks, out dataNumBlocks, out startTime, out endTime);
            DataBlockSize = dataBlockSize;
            DataOffsetBlocks = dataOffsetBlocks;
            DataNumBlocks = dataNumBlocks;
            StartTime = OmApi.OmDateTimeUnpack(startTime);
            EndTime = OmApi.OmDateTimeUnpack(endTime);

            // Defer this
            //EnsureMetadataRead();

            // Seek
            Seek(0);
        }

        // Constructor for a reader of a device file
        protected OmReader(Om om, uint deviceId, IntPtr handle) : this(om, handle)
        {
            OpenDeviceId = deviceId;
        }

        // Constructor for a reader of a disk file
        protected OmReader(Om om, string filename, IntPtr handle) : this(om, handle)
        {
            Filename = filename;
        }

        // Factory method to open a file
        public static OmReader Open(string filename)
        {
            IntPtr handle = OmApi.OmReaderOpen(filename);
            if (handle == null) { return null; }
            return new OmReader(Om.Instance, filename, handle);
        }

        //TS - TODO - Doesn't check if it is already open or not.
        public static void Close(OmReader reader)
        {
            OmApi.OmReaderClose(reader.Handle);
        }

        // Factory method to open a device data stream
        public static OmReader Open(uint deviceId)
        {
            IntPtr handle = OmApi.OmReaderOpenDeviceData((int)deviceId);
            if (handle == null) { return null; }
            return new OmReader(Om.Instance, deviceId, handle);
        }

        // Seek
        public bool Seek(int dataBlock)
        {
            if (Handle == IntPtr.Zero) { return false; }
            return OmApi.OM_SUCCEEDED(OmApi.OmReaderDataBlockSeek(Handle, dataBlock));
        }

        // Pos
        public int Pos()
        {
            if (Handle == IntPtr.Zero) { return 0; }
            return OmApi.OmReaderDataBlockPosition(Handle);
        }

        // ReadBlock
        public short[] ReadBlock()
        {
            if (Handle == IntPtr.Zero) { return new short[0]; }
            int sampleCount = OmApi.OmReaderNextBlock(Handle);
            if (sampleCount <= 0) { return null; }

            Channels = OmApi.OmReaderGetValue(Handle, OmApi.OM_READER_VALUE_TYPE.OM_VALUE_AXES);
            SequenceId = (uint)OmApi.OmReaderGetValue(Handle, OmApi.OM_READER_VALUE_TYPE.OM_VALUE_SEQUENCEID); // DWORD @10 offset in block
            AccelOneG = OmApi.OmReaderGetValue(Handle, OmApi.OM_READER_VALUE_TYPE.OM_VALUE_SCALE_ACCEL);
            GyroRange = OmApi.OmReaderGetValue(Handle, OmApi.OM_READER_VALUE_TYPE.OM_VALUE_SCALE_GYRO);
            Light = OmApi.OmReaderGetValue(Handle, OmApi.OM_READER_VALUE_TYPE.OM_VALUE_LIGHT);
            Temp = OmApi.OmReaderGetValue(Handle, OmApi.OM_READER_VALUE_TYPE.OM_VALUE_TEMPERATURE_MC);
            Batt = OmApi.OmReaderGetValue(Handle, OmApi.OM_READER_VALUE_TYPE.OM_VALUE_BATTERY_PERCENT);
            BattRaw = OmApi.OmReaderGetValue(Handle, OmApi.OM_READER_VALUE_TYPE.OM_VALUE_BATTERY_MV);

            int numValues = sampleCount * Channels;
            if (numValues > 360) { numValues = 360; } // packed triaxials
            if (numValues < 0) { numValues = 0; }

            short[] samples = new short[numValues];
            IntPtr buffer = OmApi.OmReaderBuffer(Handle);
            System.Runtime.InteropServices.Marshal.Copy(buffer, samples, 0, numValues);

            return samples;
        }

        public DateTime TimeForSample(int i)
        {
            if (Handle == IntPtr.Zero) { return DateTime.MinValue; }
            ushort fractional = 0x0000;
            uint timestamp = OmApi.OmReaderTimestamp(Handle, i, out fractional);
            return OmApi.OmDateTimeUnpack(timestamp, fractional);
        }

        public void Close()
        {
            if (Handle != IntPtr.Zero)
            {
                OmApi.OmReaderClose(Handle);
                Handle = IntPtr.Zero;
            }
        }


    }
}
