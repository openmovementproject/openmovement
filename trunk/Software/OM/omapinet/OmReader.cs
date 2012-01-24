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

        private ushort deviceId;
        public override ushort DeviceId { get { return deviceId; } }

        private uint sessionId;
        public override uint SessionId { get { return sessionId; } }

        public ushort OpenDeviceId { get; protected set; }
        public string Filename { get; protected set; }
        public IntPtr Handle { get; protected set; }
        public DateTime StartTime { get; protected set; }
        public DateTime EndTime { get; protected set; }
        public int DataBlockSize { get; protected set; }
        public int DataOffsetBlocks { get; protected set; }
        public int DataNumBlocks { get; protected set; }
        public string MetaData { get; protected set; }

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

            // Get metadata
            string metadata;
            int deviceIdInt;
            metadata = OmApi.OmReaderMetadata(handle, out deviceIdInt, out sessionId);
            MetaData = metadata;
            deviceId = (ushort)deviceIdInt;

            // Seek
            Seek(0);
        }

        // Constructor for a reader of a device file
        protected OmReader(Om om, ushort deviceId, IntPtr handle) : this(om, handle)
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

        // Factory method to open a device data stream
        public static OmReader Open(ushort deviceId)
        {
            IntPtr handle = OmApi.OmReaderOpenDeviceData(deviceId);
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
            short[] samples = new short[sampleCount * 3];
            IntPtr buffer = OmApi.OmReaderBuffer(Handle);
            System.Runtime.InteropServices.Marshal.Copy(buffer, samples, 0, sampleCount * 3);
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
