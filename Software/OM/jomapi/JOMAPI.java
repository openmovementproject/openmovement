/* 
 * Copyright (c) 2009-2012, Newcastle University, UK.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met: 
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE. 
 */

// Java JNI OMAPI Layer
// Dan Jackson, 2012

//package openmovement;

public class JOMAPI {

	// Load library
	static {
		try {
			if (System.getProperty("os.name").startsWith("Win")) {
				// On Windows, load an architecture-specific library
				boolean bits64 = System.getProperty("sun.arch.data.model").contains("64");
				System.loadLibrary("JOMAPI" + (bits64 ? "64" : "32"));
			} else {
				// On other OS's, load a universal library
				System.loadLibrary("JOMAPI");
			}
		} catch (Throwable e) {
			System.err.println("JOMAPI Load Error: " + e.toString());
		}
	}

	// No instances allowed (static class)
	private JOMAPI() { ; }

	// Static Log Callback
	private static JOMAPIListener logListener;
	private static Object logReference;
	public static void logCallback(String message)
	{
		System.out.println("LOG: " + message);
		if (logListener != null) {
			logListener.logCallback(message);
		}
	}
	
	// Static Device Callback
	private static JOMAPIListener deviceListener;
	private static Object deviceReference;
	public static void deviceCallback(int deviceId, int deviceStatus)
	{
		String status = "<" + deviceStatus + ">";
		if      (deviceStatus == OM_DEVICE_REMOVED)   { status = "OM_DEVICE_REMOVED"; }
		else if (deviceStatus == OM_DEVICE_CONNECTED) { status = "OM_DEVICE_CONNECTED"; }
		System.out.println("DEVICE: #" + deviceId + " - " + status);
		if (deviceListener != null) {
			deviceListener.deviceCallback(deviceId, deviceStatus);
		}
	}
	
	// Static Download Callback
	private static JOMAPIListener downloadListener;
	private static Object downloadReference;
	public static void downloadCallback(int deviceId, int downloadStatus, int downloadValue)
	{
		String status = "<" + downloadStatus + ">";
		if      (downloadStatus == OM_DOWNLOAD_NONE)      { status = "OM_DOWNLOAD_NONE"; }
		else if (downloadStatus == OM_DOWNLOAD_ERROR)     { status = "OM_DOWNLOAD_ERROR"; }
		else if (downloadStatus == OM_DOWNLOAD_PROGRESS)  { status = "OM_DOWNLOAD_PROGRESS"; }
		else if (downloadStatus == OM_DOWNLOAD_COMPLETE)  { status = "OM_DOWNLOAD_COMPLETE"; }
		else if (downloadStatus == OM_DOWNLOAD_CANCELLED) { status = "OM_DOWNLOAD_CANCELLED"; }
		System.out.println("DOWNLOAD: #" + deviceId + " @" + downloadValue + " - " + status);
		if (downloadListener != null) {
			downloadListener.downloadCallback(deviceId, downloadStatus, downloadValue);
		}
	}
	
	
	// JNI functions
	public static final int OM_VERSION = 103;      // Must match the library version
	public native static int OmStartup(int version);
	public native static int OmShutdown();
	public native static int OmSetLogStream(int fd);
	public static int OmSetLogCallback(JOMAPIListener listener, Object reference)
	{
		logListener = listener;
		logReference = reference;
		return OM_OK;
	}
	
	public static final int OM_DEVICE_REMOVED = 0;		// enum OM_DEVICE_STATUS
	public static final int OM_DEVICE_CONNECTED = 1;	// enum OM_DEVICE_STATUS	
	public static int OmSetDeviceCallback(JOMAPIListener listener, Object reference)
	{
		deviceListener = listener;
		deviceReference = reference;
		return OM_OK;
	}
	
	public native static int OmGetDeviceIds(int[] deviceIds, int maxDevices);
	public native static int OmGetVersion(int deviceId, int[] firmwareVersion, int[] hardwareVersion);
	public native static int OmGetBatteryLevel(int deviceId);
	public native static int OmSelfTest(int deviceId);
	public static final int OM_MEMORY_HEALTH_ERROR = 1;
	public static final int OM_MEMORY_HEALTH_WARNING = 8;
	public native static int OmGetMemoryHealth(int deviceId);
	public native static int OmGetBatteryHealth(int deviceId);
	public native static int OmGetAccelerometer(int deviceId, int[] xyz);
	public native static int OmGetTime(int deviceId, long[] time);
	public native static int OmSetTime(int deviceId, long time);
	
	public static final int OM_LED_UNKNOWN = -2;	// OM_LED_STATE
	public static final int OM_LED_AUTO = -1;		// OM_LED_STATE
	public static final int OM_LED_OFF = 0;			// OM_LED_STATE
	public static final int OM_LED_BLUE = 1;		// OM_LED_STATE
	public static final int OM_LED_GREEN = 2;		// OM_LED_STATE
	public static final int OM_LED_CYAN = 3;		// OM_LED_STATE
	public static final int OM_LED_RED = 4;			// OM_LED_STATE
	public static final int OM_LED_MAGENTA = 5;		// OM_LED_STATE
	public static final int OM_LED_YELLOW = 6;		// OM_LED_STATE
	public static final int OM_LED_WHITE = 7;		// OM_LED_STATE
	
	public native static int OmSetLed(int deviceId, int ledState);
	
	public native static int OmIsLocked(int deviceId, int[] hasLockCode);
	public native static int OmSetLock(int deviceId, int code);
	public native static int OmUnlock(int deviceId, int code);
	public native static int OmSetEcc(int deviceId, int state);
	public native static int OmGetEcc(int deviceId);
	
// TODO: OmCommand()
//public native static int OmCommand(int deviceId, String command, byte[] buffer, int bufferSize, String expected, int timeoutMs, String[] parseParts, int parseMax);
	
	public native static int OmGetDelays(int deviceId, long[] startTime, long[] stopTime);
	public native static int OmSetDelays(int deviceId, long startTime, long stopTime);
	public native static int OmGetSessionId(int deviceId, long[] sessionId);
	public native static int OmSetSessionId(int deviceId, long sessionId);
	public static final int OM_METADATA_SIZE = 448;
	public native static int OmGetMetadata(int deviceId, StringBuffer metadata);
	public native static int OmSetMetadata(int deviceId, String metadata, int size);
	public native static int OmGetLastConfigTime(int deviceId, long[] time);
	
	public static final int OM_ERASE_NONE = 0;			// OM_ERASE_LEVEL
	public static final int OM_ERASE_DELETE = 1;		// OM_ERASE_LEVEL
	public static final int OM_ERASE_QUICKFORMAT = 2;	// OM_ERASE_LEVEL
	public static final int OM_ERASE_WIPE = 3;			// OM_ERASE_LEVEL	
	public native static int OmEraseDataAndCommit(int deviceId, int eraseLevel);
	public static int OmClearDataAndCommit(int deviceId) { return OmEraseDataAndCommit(deviceId, OM_ERASE_QUICKFORMAT); }
	public static int OmCommit(int deviceId) { return OmEraseDataAndCommit(deviceId, OM_ERASE_NONE); }
	
	public static final int OM_ACCEL_DEFAULT_RATE = 100;
	public static final int OM_ACCEL_DEFAULT_RANGE = 8;
	public native static int OmGetAccelConfig(int deviceId, int[] rate, int[] range);
	public native static int OmSetAccelConfig(int deviceId, int rate, int range);
	
	public static final int OM_DOWNLOAD_NONE = 0;		// OM_DOWNLOAD_STATUS
	public static final int OM_DOWNLOAD_ERROR = 1;		// OM_DOWNLOAD_STATUS
	public static final int OM_DOWNLOAD_PROGRESS = 2;	// OM_DOWNLOAD_STATUS
	public static final int OM_DOWNLOAD_COMPLETE = 3;	// OM_DOWNLOAD_STATUS
	public static final int OM_DOWNLOAD_CANCELLED = 4;	// OM_DOWNLOAD_STATUS

	public static int OmSetDownloadCallback(JOMAPIListener listener, Object reference)
	{
		downloadListener = listener;
		downloadReference = reference;
		return OM_OK;
	}
	
	public native static int OmGetDataFilename(int deviceId, StringBuffer filenameBuffer);
	public native static int OmGetDataRange(int deviceId, int[] dataBlockSize, int[] dataOffsetBlocks, int[] dataNumBlocks, long[] startTime, long[] endTime);
	public native static int OmBeginDownloading(int deviceId, int dataOffsetBlocks, int dataLengthBlocks, String destinationFile);
	public native static int OmQueryDownload(int deviceId, int[] downloadStatus, int[] value);
	public native static int OmWaitForDownload(int deviceId, int[] downloadStatus, int[] value);	
	public native static int OmCancelDownload(int deviceId);
	
	public static final int OM_TRUE              = 1;
	public static final int OM_FALSE             = 0;
	public static final int OM_OK                = 0;
	public static final int OM_E_FAIL            = -1;
	public static final int OM_E_UNEXPECTED      = -2;
	public static final int OM_E_NOT_VALID_STATE = -3;
	public static final int OM_E_OUT_OF_MEMORY   = -4;
	public static final int OM_E_INVALID_ARG     = -5;
	public static final int OM_E_POINTER         = -6;
	public static final int OM_E_NOT_IMPLEMENTED = -7;
	public static final int OM_E_ABORT           = -8;
	public static final int OM_E_ACCESS_DENIED   = -9;
	public static final int OM_E_INVALID_DEVICE  = -10;
	public static final int OM_E_UNEXPECTED_RESPONSE = -11;
	public static final int OM_E_LOCKED = -12;
	public static boolean OM_SUCCEEDED(int value) { return (value >= 0); }
	public static boolean OM_FAILED(int value) { return (value < 0); }
	
	public native static String OmErrorString(int status);
	
// TODO
/*	
	public static uint OM_DATETIME_FROM_YMDHMS(int year, int month, int day, int hours, int minutes, int seconds)
	{ 
			return ((((uint)(year % 100) & 0x3f) << 26) | (((uint)(month) & 0x0f) << 22) | (((uint)(day) & 0x1f) << 17) | (((uint)(hours) & 0x1f) << 12) | (((uint)(minutes) & 0x3f) << 6) | (((uint)(seconds) & 0x3f)));
	}
	public static int OM_DATETIME_YEAR(uint dateTime)    { return ((int)((byte)(((dateTime) >> 26) & 0x3f)) + 2000); }
	public static int OM_DATETIME_MONTH(uint dateTime)   { return ((byte)(((dateTime) >> 22) & 0x0f)); }
	public static int OM_DATETIME_DAY(uint dateTime)     { return ((byte)(((dateTime) >> 17) & 0x1f)); }
	public static int OM_DATETIME_HOURS(uint dateTime)   { return ((byte)(((dateTime) >> 12) & 0x1f)); }
	public static int OM_DATETIME_MINUTES(uint dateTime) { return ((byte)(((dateTime) >>  6) & 0x3f)); }
	public static int OM_DATETIME_SECONDS(uint dateTime) { return ((byte)(((dateTime)      ) & 0x3f)); }
	public static final uint OM_DATETIME_ZERO = 0x00000000;
	public static final uint OM_DATETIME_INFINITE = 0xffffffff;
	public static final uint OM_DATETIME_MIN_VALID = ((((uint)( 0) & 0x3f) << 26) | (((uint)( 1) & 0x0f) << 22) | (((uint)( 1) & 0x1f) << 17) | (((uint)( 0) & 0x1f) << 12) | (((uint)( 0) & 0x3f) << 6) | (((uint)( 0) & 0x3f)));
	public static final uint OM_DATETIME_MAX_VALID = ((((uint)(63) & 0x3f) << 26) | (((uint)(12) & 0x0f) << 22) | (((uint)(31) & 0x1f) << 17) | (((uint)(23) & 0x1f) << 12) | (((uint)(59) & 0x3f) << 6) | (((uint)(59) & 0x3f)));
	public static DateTime OmDateTimeUnpack(uint value, ushort fractional = 0x0000)
	{
		if (value == OM_DATETIME_ZERO) { return DateTime.MinValue; }
		if (value == OM_DATETIME_INFINITE) { return DateTime.MaxValue; }
		try
		{
			DateTime ret = new DateTime(OM_DATETIME_YEAR(value), OM_DATETIME_MONTH(value), OM_DATETIME_DAY(value), OM_DATETIME_HOURS(value), OM_DATETIME_MINUTES(value), OM_DATETIME_SECONDS(value));
			if (fractional > 0x0000) { ret.AddSeconds((double)fractional / 0x10000); }
			return ret;
		}
		catch (Exception)
		{
			return DateTime.MinValue;
		}
	}
	public static uint OmDateTimePack(DateTime value)
	{
		if (value == DateTime.MinValue || value.Year < 2000) { return OM_DATETIME_ZERO; }
		if (value == DateTime.MaxValue || value.Year > 2063) { return OM_DATETIME_INFINITE; }
		return OM_DATETIME_FROM_YMDHMS(value.Year, value.Month, value.Day, value.Hour, value.Minute, value.Second);
	}
*/

	public native static long OmReaderOpen(String binaryFilename);
	public static long OmReaderOpenDeviceData(int deviceId)
	{
		StringBuffer filename = new StringBuffer(256 + 1);
		int ret = OmGetDataFilename(deviceId, filename);
		if (ret != OM_OK) { return ret; }
		return OmReaderOpen(filename.toString());
	}
	public native static int OmReaderDataRange(long reader, int[] dataBlockSize, int[] dataOffsetBlocks, int[] dataNumBlocks, long[] startTime, long[] endTime);
	public native static String OmReaderMetadata(long reader, int[] deviceId, long[] sessionId);
	public native static int OmReaderDataBlockPosition(long reader);
	public native static int OmReaderDataBlockSeek(long reader, int dataBlockNumber);
	public native static int OmReaderNextBlock(long reader);
	
// TODO: A more useful OmReaderBuffer(), use a short array?
//	public native static long OmReaderBuffer(long reader);
	
	public native static long OmReaderTimestamp(long reader, int index, int[] fractional);
	public static final int OM_VALUE_DEVICEID = 3;			// OM_READER_VALUE_TYPE
	public static final int OM_VALUE_SESSIONID = 4;			// OM_READER_VALUE_TYPE
	public static final int OM_VALUE_SEQUENCEID = 5;		// OM_READER_VALUE_TYPE
	public static final int OM_VALUE_LIGHT = 7;				// OM_READER_VALUE_TYPE
	public static final int OM_VALUE_TEMPERATURE = 8;		// OM_READER_VALUE_TYPE
	public static final int OM_VALUE_EVENTS = 9;			// OM_READER_VALUE_TYPE
	public static final int OM_VALUE_BATTERY = 10;			// OM_READER_VALUE_TYPE
	public static final int OM_VALUE_SAMPLERATE = 11;		// OM_READER_VALUE_TYPE
	public static final int OM_VALUE_TEMPERATURE_MC = 108;	// OM_READER_VALUE_TYPE
	public static final int OM_VALUE_BATTERY_MV = 110;		// OM_READER_VALUE_TYPE
	public native static int OmReaderGetValue(long reader, int valueType);

/*
	[StructLayout(LayoutKind.Explicit, Size=512, LayoutKind.Sequential)]
	public unsafe class OM_READER_HEADER_PACKET
	{
		[FieldOffset(0)]  public ushort packetHeader;	    // @ 0 +2 ASCII "MD", little-endian (0x444D) 
		[FieldOffset(2)]  public ushort packetLength;	    // @ 2 +2 Packet length (1020 bytes, with header (4) = 1024 bytes total) 
		[FieldOffset(4)]  public byte reserved1;            // @ 4 +1 (1 byte reserved) 
		[FieldOffset(5)]  public ushort deviceId;           // @ 5 +2 Device identifier 
		[FieldOffset(7)]  public uint sessionId;            // @ 7 +4 Unique session identifier 
		[FieldOffset(11)] public ushort reserved2;          // @11 +2 (2 bytes reserved) 
		[FieldOffset(13)] public uint loggingStartTime;     // @13 +4 Start time for delayed logging 
		[FieldOffset(17)] public uint loggingEndTime;       // @17 +4 Stop time for delayed logging 
		[FieldOffset(21)] public uint loggingCapacity;      // @21 +4 Preset maximum number of samples to collect, 0 = unlimited 
		[FieldOffset(25)] public fixed byte reserved3[11];  // @25 +11 (11 bytes reserved) 
		[FieldOffset(36)] public byte samplingRate;		    // @36 +1 Sampling rate 
		[FieldOffset(37)] public uint lastChangeTime;       // @37 +4 Last change metadata time 
		[FieldOffset(41)] public byte firmwareRevision;		// @41 +1 Firmware revision number 
		[FieldOffset(42)] public short timeZone;		    // @42 +2 Time Zone offset from UTC (in minutes), 0xffff = -1 = unknown 
		[FieldOffset(44)] public fixed byte reserved4[20];   // @44 +20 (20 bytes reserved) 
		[FieldOffset(64)] public fixed byte annotation[448]; // @64 +448 Scratch buffer / meta-data (448 characters) 
		[FieldOffset(512)] public fixed byte reserved[512];  // @512 +512 Reserved for post-collection scratch buffer / meta-data (512 characters) 
	};
*/	
// TODO: A more useful OmReaderRawHeaderPacket(), use a byte array?
//	public native static long OmReaderRawHeaderPacket(long reader);

/*
	[StructLayout(LayoutKind.Explicit, Size=512, LayoutKind.Sequential)]
	public class OM_READER_DATA_PACKET
	{
		[FieldOffset(0)]  public ushort packetHeader;	    // @ 0 +2  ASCII "AX", little-endian (0x5841) 
		[FieldOffset(2)]  public ushort packetLength;	    // @ 2 +2  Packet length (508 bytes, with header (4) = 512 bytes total) 
		[FieldOffset(4)]  public ushort deviceId;		    // @ 4 +2  Device identifier, 0 = unknown 
		[FieldOffset(6)]  public uint sessionId;			// @ 6 +4  Unique session identifier, 0 = unknown 
		[FieldOffset(10)] public uint sequenceId;		    // @10 +4  Sequence counter, each packet has a new number (reset if restarted) 
		[FieldOffset(14)] public uint timestamp;			// @14 +4  Last reported RTC value, 0 = unknown 
		[FieldOffset(18)] public ushort light;			    // @18 +2  Last recorded light sensor value in raw units, 0 = none 
		[FieldOffset(20)] public ushort temperature;		// @20 +2  Last recorded temperature sensor value in raw units, 0 = none 
		[FieldOffset(22)] public byte  events;			    // @22 +1  Event flags since last packet, b0 = resume logging, b1 = single-tap event, b2 = double-tap event, b3-b7 = reserved for diagnostic use) 
		[FieldOffset(23)] public byte  battery;			    // @23 +1  Last recorded battery level in raw units, 0 = unknown 
		[FieldOffset(24)] public byte  sampleRate;		    // @24 +1  Sample rate code, (3200/(1<<(15-(rate & 0x0f)))) Hz 
		[FieldOffset(25)] public byte  numAxesBPS;		    // @25 +1  0x32 (top nibble: number of axes = 3; bottom nibble: packing format - 2 = 3x 16-bit signed, 0 = 3x 10-bit signed + 2-bit exponent) 
		[FieldOffset(26)] public short   timestampOffset;	// @26 +2  Relative sample index from the start of the buffer where the whole-second timestamp is valid
		[FieldOffset(28)] public ushort sampleCount;		// @28 +2  Number of accelerometer samples (80 or 120) 
		[FieldOffset(30)] public fixed byte rawSampleData[480];   // @30 +480 Raw sample data.  Each sample is either 3x 16-bit signed values (x, y, z) or one 32-bit packed value (The bits in bytes [3][2][1][0]: eezzzzzz zzzzyyyy yyyyyyxx xxxxxxxx, e = binary exponent, lsb on right) 
		[FieldOffset(510)] public ushort checksum;          // @510 +2 Checksum of packet (16-bit word-wise sum of the whole packet should be zero) 
	};
*/
// TODO: A more useful OmReaderRawHeaderPacket(), use a byte array?
//	public native static long OmReaderRawDataPacket(long reader);

	public native static void OmReaderClose(long reader);
	
}
