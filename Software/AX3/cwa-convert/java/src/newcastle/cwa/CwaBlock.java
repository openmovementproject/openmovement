/**
 * CWA Data Block
 */
package newcastle.cwa;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;

/**
 * A representation of a single CWA data file block, used by CwaReader (and therefore CwaCsvInputStream).
 * Mutable for efficiency reasons.
 * @author Dan Jackson, Newcastle University
 */
public class CwaBlock implements Cloneable {
	
	/** Block size for data */
	public static final int BLOCK_SIZE = 512;
	
	/** Block buffer for data */
	private ByteBuffer byteBuffer;
	private boolean bufferValid = false;
	private boolean dataBlockOk = false;
	private int sessionId = 0;
	private short deviceId = 0;
	private short light = 0;
	private short temp = 0;
	private short batt = 0;
	private short events = 0;
	private int sampleCount = 0;
	private long[] sampleTimes;
	private short[] sampleValues;
	
	
	/** Maximum number of samples per (new format) data block */	
	public static final int MAX_SAMPLES_PER_BLOCK = 120;
	
	/** Number of samples per (old format) data block */	
	public static final int NUM_SAMPLES_PER_OLD_BLOCK = 80;
	
	/** Number of axes per sample */	
	public static final int NUM_AXES_PER_SAMPLE = 3;
	
	/** Block type - no block */	
	public static final short BLOCK_NONE = 0;			//
	
	/** Block type - empty block */	
	public static final short BLOCK_EMPTY = -1;			// 0xffff
	
	/** Block type - header block */	
	public static final short BLOCK_HEADER = 0x444D;		// "MD" (length 0xFFFC)
	/** Block length - header block */	
	public static final int LENGTH_HEADER = 0xFFFC;
	
	/** Block type - usage block */	
	public static final short BLOCK_USAGE = 0x4255;			// "UB" (length 0xFFFC)
	/** Block length - usage block */	
	public static final int LENGTH_USAGE = 0xFFFC;
	
	/** Block type - session info block */	
	public static final short BLOCK_SESSIONINFO = 0x4953;	// "SI" (length 0x01FC)
	/** Block length - session info block */	
	public static final int LENGTH_SESSIONINFO = 0x01FC;
	
	/** Block type - data block */	
	public static final short BLOCK_DATA = 0x5841;			// "AX" (length 0x01FC)
	/** Block length - data block */	
	public static final int LENGTH_DATA = 0x01FC;
	
	
	/*
	// 512-byte data packet
	typedef struct DataPacket_t
	{
		unsigned short packetHeader;	// [2] = 0x5841 (ASCII "AX", little-endian)
		unsigned short packetLength;	// [2] = 508 bytes (contents of this packet is 508 bytes long, + 2 + 2 = 512 bytes total)
		unsigned short deviceId;		// [2] (16-bit device identifier, 0 = unknown)
	    unsigned int sessionId;			// [4] (32-bit unique session identifier, 0 = unknown)
	    unsigned int sequenceId;		// [4] (32-bit sequence counter, each packet has a new number -- reset if restarted?)
	    unsigned int timestamp;			// [4] (last reported RTC value, 0 = unknown)
		unsigned short light;			// [2] (last recorded light sensor value in raw units, 0 = none)
		unsigned short temperature;		// [2] (last recorded temperature sensor value in raw units)
		unsigned char  events;			// [1] (event flags since last packet, b0 = resume logging from standby, b1 = single-tap event, b2 = double-tap event, b3-b7 = reserved)
		unsigned char  battery;			// [1] (last recorded battery level in 6/256V, 0 = unknown)
		unsigned char  sampleRate;		// <was reserved> [1] = sample rate code (3200/(1<<(15-(rate & 0x0f)))) Hz, if 0, then old format where sample rate stored in 'timestampOffset' field as whole number of Hz
		unsigned char  numAxesBPS;		// [1] = 0x32 (top nibble: number of axes = 3; bottom nibble: number of bytes per axis sample - 2 = 3x 16-bit signed, 0 = 3x 10-bit signed + 2-bit exponent)
		signed short   timestampOffset;	// <was sampleRate> [2] = [if sampleRate is non-zero:] Relative sample index from the start of the buffer where the whole-second timestamp is valid [otherwise, if sampleRate is zero, this is the old format with the sample rate in Hz]
		unsigned short sampleCount;		// [2] = 80 samples (number of accelerometer samples)
		Sample sampleData[80];	// [sampleCount * numAxes * bytesPerSample = 480] (sample data)
		//unsigned char  reservedData[0];	// [packetLength - 28 - (sampleCount * numAxes * bytesPerSample) = 0] (reserved data area)
		unsigned short checksum;		// [2] = Checksum of packet (0 = either checksum is zero or no checksum is recorded)
	} DataPacket;
	
	// Data status structure
	typedef struct
	{
	    // Header
	    unsigned short header;              // 0x444D = ("MD") Meta data block
	    unsigned short blockSize;           // 0xFFFC = Packet size (2^16 - 2 - 2)
	
		// Stored data
		unsigned char performClear;
		unsigned short deviceId;
	    unsigned int sessionId;
		unsigned short shippingMinLightLevel;
	    unsigned int loggingStartTime;
	    unsigned int loggingEndTime;
	    unsigned int loggingCapacity;
		unsigned char allowStandby;			// User allowed to transition LOGGING->STANDBY (and if POWERED->STANDBY/LOGGING)
		unsigned char debuggingInfo;		// Additional LED debugging info
		unsigned short batteryMinimumToLog;	// Minimum battery level required for logging
		unsigned short batteryWarning; 		// Battery level below which show warning colour for logging status
	} DataMeta;
	 */
	
	
	
	/** Constructor for mutable version of this class used by the reader */
	/* package */ CwaBlock() {
		this(null);
	}

	/** Copy constructor */
	CwaBlock(CwaBlock source) {
		if (source != null) {
			byteBuffer = ByteBuffer.wrap(source.byteBuffer.array().clone());
			bufferValid = source.bufferValid;
			dataBlockOk = source.dataBlockOk;
			sessionId = source.sessionId;
			deviceId = source.deviceId;
			light = source.light;
			temp = source.temp;
			batt = source.batt;
			events = source.events;
			sampleCount = source.sampleCount;
			sampleTimes = source.sampleTimes.clone();
			sampleValues = source.sampleValues.clone();
		} else {
			byteBuffer = ByteBuffer.wrap(new byte[BLOCK_SIZE]);
			bufferValid = false;
			dataBlockOk = false;
			sessionId = -1;
			deviceId = -1;
			light = -1;
			temp = -1;
			batt = -1;
			events = 0;
			sampleCount = 0;
			sampleTimes = new long[0];
			sampleValues = new short[0];
		}
		byteBuffer.order(ByteOrder.LITTLE_ENDIAN);	// All little endian values
		
		// In-place samples
		//byteBuffer.position(30);
		//private ShortBuffer rawSamplesShort = null;
		//rawSamplesShort = byteBuffer.asShortBuffer();
		//byteBuffer.rewind();		
	}

	// Clone method -- calls copy constructor
    public Object clone() { // throws CloneNotSupportedException
    	return new CwaBlock(this);
    }

	// Marks the block as invalidate
    /*package*/ void invalidate() {
    	bufferValid = false;
    }
    
    /**
     * @return direct read-only access to the internal ByteBuffer
     */
    public ByteBuffer buffer() {
    	return byteBuffer.asReadOnlyBuffer();
    }
    
    protected long tLast = 0;

	/**
	 * Reads the next block into the buffer.
	 * @param inputStream the input stream to read from
	 * @return true if block successfully read, false otherwise (end of file)
	 * @throws IOException if reading the input stream fails
	 */
	public boolean readFromInputStream(InputStream inputStream) throws IOException {
		// Read a block into the buffer
		int offset = 0;
		bufferValid = false;
		while (offset < BLOCK_SIZE) {
			int numRead = inputStream.read(byteBuffer.array(), offset, BLOCK_SIZE - offset);
			if (numRead < 0) { break; }
			offset += numRead;
		}
		dataBlockOk = false;
		// Buffer is valid if block was fully read
		if (offset >= BLOCK_SIZE) {
			bufferValid = true;
			
			if (getBlockType() == BLOCK_DATA) {
				deviceId = byteBuffer.getShort(4);
				sessionId = byteBuffer.getInt(6);
				int blockTimestamp = byteBuffer.getInt(14);				
				light = byteBuffer.getShort(18);				
				temp = byteBuffer.getShort(20);				
				events = (short)(byteBuffer.get(22) & 0xff);
				batt = (short)(byteBuffer.get(23) & 0xff);
                short sampleRate = (short)(byteBuffer.get(24) & 0xff);
                short numAxesBPS = (short)(byteBuffer.get(25) & 0xff);
                short timestampOffset = byteBuffer.getShort(26);
				sampleCount = byteBuffer.getShort(28);
				short sum = 0;
				float freq;
				float offsetStart;
				
				// See which format the packet is
				if (sampleRate == 0) {
					sum = 0;
					freq = (float)timestampOffset;  // Old format, frequency stored directly
					offsetStart = 0.0f;
				} else {
					// Calculate sum of packet (should be zero)
					sum = 0;
					for (int i = 0; i < BLOCK_SIZE / 2; i++) {
						sum += byteBuffer.getShort(i * 2);
					}
					// New format
					freq = 3200.0f / (1 << (15 - (sampleRate & 0x0f)));
					if (freq <= 0.0f) { freq = 1.0f; }
					offsetStart = -timestampOffset / freq;
				}

								
				if (sum == 0x0000) { 
				
	                int bytesPerSample = 0;
	                if (((numAxesBPS >> 4) & 0x0f) != NUM_AXES_PER_SAMPLE) { ; }    // Unexpected number of axes
	                if ((numAxesBPS & 0x0f) == 2) { bytesPerSample = 6; }    // 3*16-bit
	                else if ((numAxesBPS & 0x0f) == 0) { bytesPerSample = 4; }    // 3*10-bit + 2
	                short expectedCount = (short)((bytesPerSample != 0) ? 480 / bytesPerSample : 0);
	                if (sampleCount != expectedCount) { sampleCount = expectedCount; }
					if (sampleCount < 0) { sampleCount = 0; }
					if (sampleCount > MAX_SAMPLES_PER_BLOCK) { sampleCount = MAX_SAMPLES_PER_BLOCK; }
	
					int arraySize = sampleCount * NUM_AXES_PER_SAMPLE;
					if (sampleValues.length != arraySize) {
						sampleValues = new short[arraySize];
						sampleTimes = new long[arraySize];
					}
					
					long time0 = getTimestamp(blockTimestamp) + (long)(1000 * offsetStart / freq);
					long time1 = time0 + (long)(1000 * sampleCount / freq);		// Packet end time
//System.err.println("[" + time0 + " - " + time1 + "]");
					if (tLast != 0 && time0 - tLast < 1000) 
					{ 
						time0 = tLast;
					}
					tLast = time1;
					long timeD = time1 - time0;
					
    				for (int i = 0; i < sampleCount; i++) {
						long t = (sampleRate == 0) ? time0 : (time0 + ((i * timeD) / sampleCount));
                        short x, y, z;
                                                
                        if (bytesPerSample == 4) {
                            long value = byteBuffer.getInt(30 + 4 * i);
                        	x = (short)((short)(0xffffffc0 & (value <<  6)) >> (6 - ((byte)(value >> 30))));		// Sign-extend 10-bit value, adjust for exponent
                        	y = (short)((short)(0xffffffc0 & (value >>  4)) >> (6 - ((byte)(value >> 30))));		// Sign-extend 10-bit value, adjust for exponent
                        	z = (short)((short)(0xffffffc0 & (value >> 14)) >> (6 - ((byte)(value >> 30))));      // Sign-extend 10-bit value, adjust for exponent
                        } else if (bytesPerSample == 6) {
	    					x = byteBuffer.getShort(30 + 2 * NUM_AXES_PER_SAMPLE * i + 0);
	    					y = byteBuffer.getShort(30 + 2 * NUM_AXES_PER_SAMPLE * i + 2);
	    					z = byteBuffer.getShort(30 + 2 * NUM_AXES_PER_SAMPLE * i + 4);                        	
                        } else {
                        	x = 0; y = 0; z = 0;
                        }
                        
                        sampleTimes[i] = t;
                        sampleValues[i * NUM_AXES_PER_SAMPLE + 0] = x;
                        sampleValues[i * NUM_AXES_PER_SAMPLE + 1] = y;
                        sampleValues[i * NUM_AXES_PER_SAMPLE + 2] = z;
    				}

					dataBlockOk = true;
				}
				
			} else if (getBlockType() == BLOCK_HEADER) {
				deviceId = byteBuffer.getShort(5);
				sessionId = byteBuffer.getInt(7);
			}
			
			return true;
		} else {
			return false;
		}
	}
	
	/**
	 * Identifies the type of block
	 * @return short integer indicating the type of the block
	 */
	public short getBlockType() {
		if (!isValid()) {
			return BLOCK_NONE;
		}
		return byteBuffer.getShort(0);	//(short)(((long)block[0] & 0xff) | (((long)block[1] << 8) & 0xff));
	}
    
	/**
	 * Determines whether the block is valid
	 * @return whether block is valid
	 */
	public boolean isValid() { 
		return bufferValid;
	}
	
	/**
	 * Determines whether the block is a valid data block
	 * @return whether block is a data block
	 */
	public boolean isDataBlock() { 
		return dataBlockOk;
	}
	
	/**
	 * Data block samples
	 * @return number of samples
	 */
	public int getNumSamples() { 
		return dataBlockOk ? sampleCount : 0; 
	}
	
	/**
	 * Session identifier
	 * @return session identifier
	 */
	public int getSessionId() { 
		return sessionId;
	}
	
	/**
	 * Device identifier
	 * @return device identifier
	 */
	public short getDeviceId() { 
		return deviceId;
	}
	
	/** @return light reading */
	public short getLight() { return light; }
	/** @return temperature reading */
	public short getTemperature() { return temp; }
	/** @return battery reading */
	public short getBattery() { return batt; }
	/** @return event flags */
	public short getEvents() { return events; }
	
	/**
	 * Data block samples
	 * @return samples as signed 16-bit integers (1G = 256), samples are X0,Y0,Z0,X1,Y1,Z1,...
	 */
	public short[] getSampleValues() { 
		if (!isDataBlock()) { return null; }
		return sampleValues; 
	}
	
	
	/**
	 * Data block timestamps
	 * @return sample times
	 */
	public long[] getTimestampValues() { 
		if (!isDataBlock()) { return null; }
		return sampleTimes; 
	}
	
	
	private static Calendar calendar = new GregorianCalendar();
	/** Convert a CWA timestamp value into a Java Date.
	 * @param cwaTimestamp 32-bit CWA timestamp value
	 * @return Number of milliseconds since 1/1/1970
	 */
	public static long getTimestamp(long cwaTimestamp) {
		int year  = (int)((cwaTimestamp >> 26) & 0x3f) + 2000;
		int month = (int)((cwaTimestamp >> 22) & 0x0f);
		int day   = (int)((cwaTimestamp >> 17) & 0x1f);
		int hours = (int)((cwaTimestamp >> 12) & 0x1f);
		int mins  = (int)((cwaTimestamp >>  6) & 0x3f);
		int secs  = (int)((cwaTimestamp      ) & 0x3f);
		
		calendar.setTimeInMillis(0);								// Otherwise milliseconds is undefined(!)
		calendar.set(year, month - 1, day, hours, mins, secs);		// Month is zero-indexed(!)
		return calendar.getTimeInMillis();
	}

	
	/**
	 * Standard CWA date formatting string
	 */
	public static final String DATE_FORMAT = "yyyy-MM-dd HH:mm:ss.SSS";
	
    private static DateFormat dateFormat = null;
	/**
	 * @param timespan Timespan since 1/1/1970 in milliseconds
	 * @return Standard formatting for the supplied date
	 */
	public static String getDateString(long timespan) {
		if (dateFormat == null) {
			dateFormat = new SimpleDateFormat(DATE_FORMAT);
		}
	    return dateFormat.format(new Date(timespan));
	}
	
	
}
