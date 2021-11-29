/**
 * CWA Converter
 */
package newcastle.cwa;

import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.util.Map;

/**
 * A FilterInputStream that converts the binary CWA files into a CSV stream.
 * The output format is: 
 *     <code>yyyy-MM-dd HH:mm:ss.SSS,X,Y,Z</code>
 * ...where X, Y, Z are signed floats representing the axis acceleration measured in 'G'.
 * 
 * Note: New format files will have an interpolated timestamp.  
 *       For old format files, only the timestamp of the containing block is shown for each sample.
 * 
 * Example usage:
 * <pre>
 *     // Create a standard file input stream for the binary file
 *     InputStream in = new FileInputStream("CWA-DATA.CWA");
 * 
 *     // Create the filter stream to convert it into CSV
 *     CwaCsvInputStream csvIn = new CwaCsvInputStream(in);
 *    
 *     // Use a buffered reader to read the CSV line-by-line
 *     BufferedReader filter = new BufferedReader(new InputStreamReader(csvIn));
 *    
 *     // While not EOF, read and output the CSV lines
 *     String line;
 *     while ((line = filter.readLine()) != null) 
 *         System.out.println(line);
 * 
 *     // Close the outermost stream
 *     filter.close();
 * </pre>
 * @author Dan Jackson, Newcastle University
 */
public class CwaCsvInputStream extends FilterInputStream {

	// Block reader
	private CwaReader cwaReader;

	// Output buffer
	private final static int MAX_LINE_LENGTH = 192;
	private final static int MAX_OUT_BUFFER = (CwaBlock.MAX_SAMPLES_PER_BLOCK * MAX_LINE_LENGTH);
	private ByteBuffer outBuffer;
	private StringBuilder outputStringBuilder;
	private int line, firstLine, lineSkip, lineCount, options;
	private short events = 0;

	//private static final int DATA_EVENT_NONE = 0x00;
	private static final int DATA_EVENT_RESUME = 0x01;
	private static final int DATA_EVENT_SINGLE_TAP = 0x02;
	private static final int DATA_EVENT_DOUBLE_TAP = 0x04;
	private static final int DATA_EVENT_EVENT = 0x08; // (not used)
	private static final int DATA_EVENT_FIFO_OVERFLOW = 0x10;
	private static final int DATA_EVENT_BUFFER_OVERFLOW = 0x20;
	private static final int DATA_EVENT_UNHANDLED_INTERRUPT = 0x40;
	private static final int DATA_EVENT_CHECKSUM_FAIL = 0x80;  // (not used)

	/** No export options */
	public static final int OPTIONS_NONE = 0x00;
	/** Export option for light values */
	public static final int OPTIONS_LIGHT = 0x01;
	/** Export option for temperature values */
	public static final int OPTIONS_TEMP = 0x02;
	/** Export option for battery values */
	public static final int OPTIONS_BATT = 0x04;
	/** Export option for events values */
	public static final int OPTIONS_EVENTS = 0x08;
	/** Export option for meta-data header */
	public static final int OPTIONS_METADATA = 0x10;
	
	
	/**
	 * Creates a CwaCsvInputStream object
	 * @param inputStream the source input stream
	 */
	public CwaCsvInputStream(InputStream inputStream) {
		this(inputStream, 0, 1, -1, 0);
	}

	/**
	 * Creates a CwaCsvInputStream object
	 * @param inputStream the source input stream
	 * @param firstLine the first line to return (0 returns from the first line)
	 * @param lineSkip return every nth line (1 returns every line)
	 * @param lineCount maximum number of lines to return (-1 for infinite limit)
	 * @param options combination of OPTIONS_X flags
	 */
	public CwaCsvInputStream(InputStream inputStream, int firstLine, int lineSkip, int lineCount, int options) {
		super(inputStream);
		this.firstLine = firstLine;
		this.lineSkip = lineSkip;
		this.lineCount = lineCount;
		this.options = options;
		outputStringBuilder = new StringBuilder(MAX_OUT_BUFFER);
		outBuffer = ByteBuffer.wrap(new byte[MAX_OUT_BUFFER]);
		outBuffer.position(0);
		outBuffer.limit(0);
		line = 0;
		cwaReader = new CwaReader(inputStream);
		try {
			cwaReader.skipNonDataBlocks();
		} catch (IOException e) {
			System.err.println("EXCEPTION: IOException in CwaCsvInputStream() at: cwaReader.skipNonDataBlocks() -- " + e.getMessage());
		}
		
		if ((options & OPTIONS_METADATA) != 0) { 
			String lineSeparator = System.getProperty("line.separator");
			String prefix = ",,,,,,,,"; //"t,x,y,z,L,T,B,E,NAME,VALUE"
			
			outputStringBuilder.append(prefix).append("deviceId,").append(cwaReader.getDeviceId()).append(lineSeparator);
			Map<String,String> annotations = cwaReader.getAnnotations();
			for (String key : annotations.keySet()) {
				outputStringBuilder.append(prefix).append(key).append(",").append(annotations.get(key)).append(lineSeparator);
			}
			
			// Check if the output buffer is big enough (it should be, but we can create a new one if it wasn't)
			if (outputStringBuilder.length() > outBuffer.capacity()) {
				outBuffer = ByteBuffer.wrap(new byte[outputStringBuilder.length()]);
			}

			// Update the limit and position of the output buffer
			outBuffer.position(0);
			outBuffer.limit(outputStringBuilder.length());

			// Copy string builder to byte array
			int len = outputStringBuilder.length();
			for (int i = 0; i < len; i++) {
				outBuffer.array()[i] = (byte)(outputStringBuilder.charAt(i));
			}
			
			// Empty string builder
			outputStringBuilder.delete(0, outputStringBuilder.length());
		}
	}

	private boolean fillOutputBuffer() throws IOException {
		// Peek the next block (temporary copy)
		CwaBlock block;
		for (;;) {
			block = cwaReader.peekBlock();
			if (block == null) { return false; }
			if (block.isDataBlock()) { break; }
			block.invalidate();
		}
		
		long[] timestamps = block.getTimestampValues();
		String lineSeparator = System.getProperty("line.separator");
		short[] sampleValues = block.getSampleValues();
		int numSamples = block.getNumSamples();
		int numAxes = block.getNumAxes();
		int accelAxis = block.getAccelAxis();
		int accelUnit = block.getAccelUnit();
		int gyroAxis = block.getGyroAxis();
		int gyroRange = block.getGyroRange();
		float gyroUnit = (gyroRange != 0) ? (32768.0f / gyroRange) : 0;
		for (int i = 0; i < numSamples; i++) {
			float ax = 0, ay = 0, az = 0;
			if (accelAxis >= 0) {
				ax = (float)sampleValues[numAxes * i + accelAxis + 0] / accelUnit;
				ay = (float)sampleValues[numAxes * i + accelAxis + 1] / accelUnit;
				az = (float)sampleValues[numAxes * i + accelAxis + 2] / accelUnit;
			}

			float gx = 0, gy = 0, gz = 0;
			if (gyroAxis >= 0) {
				gx = (float)sampleValues[numAxes * i + gyroAxis + 0] / gyroUnit;
				gy = (float)sampleValues[numAxes * i + gyroAxis + 1] / gyroUnit;
				gz = (float)sampleValues[numAxes * i + gyroAxis + 2] / gyroUnit;
			}
			
			// Accumulate all events until displayed
			events |= block.getEvents();
			
			if (line >= firstLine && (line % lineSkip) == 0 && (lineCount < 0 || line < lineCount * lineSkip)) {
				//outputStringBuilder.append(timestamps[i]);
				outputStringBuilder.append(CwaBlock.getDateString(timestamps[i]));
				outputStringBuilder.append(',').append(ax).append(',').append(ay).append(',').append(az);
				if (gyroAxis >= 0) {
					outputStringBuilder.append(',').append(gx).append(',').append(gy).append(',').append(gz);
				}

				if ((options & OPTIONS_LIGHT) != 0) { outputStringBuilder.append(',').append(block.getLight()); }
				if ((options & OPTIONS_TEMP) != 0) { outputStringBuilder.append(',').append(block.getTemperature()); }
				if ((options & OPTIONS_BATT) != 0) { outputStringBuilder.append(',').append(block.getBattery()); }
				if ((options & OPTIONS_EVENTS) != 0) { 
					outputStringBuilder.append(','); 
					if ((events & DATA_EVENT_RESUME) != 0)              { outputStringBuilder.append('r'); }
					if ((events & DATA_EVENT_SINGLE_TAP) != 0)          { outputStringBuilder.append('s'); }
					if ((events & DATA_EVENT_DOUBLE_TAP) != 0)          { outputStringBuilder.append('d'); }
					if ((events & DATA_EVENT_EVENT) != 0)               { outputStringBuilder.append('e'); }
					if ((events & DATA_EVENT_FIFO_OVERFLOW) != 0)       { outputStringBuilder.append('F'); }
					if ((events & DATA_EVENT_BUFFER_OVERFLOW) != 0)     { outputStringBuilder.append('B'); }
					if ((events & DATA_EVENT_UNHANDLED_INTERRUPT) != 0) { outputStringBuilder.append('I'); }
					if ((events & DATA_EVENT_CHECKSUM_FAIL) != 0)       { outputStringBuilder.append('X'); }
					events = 0x00;
				}
				
				outputStringBuilder.append(lineSeparator);				
			}
			
			line++;
		}

		// Check if the output buffer is big enough (it should be, but we can create a new one if it wasn't)
		if (outputStringBuilder.length() > outBuffer.capacity()) {
			outBuffer = ByteBuffer.wrap(new byte[outputStringBuilder.length()]);
		}

		// Update the limit and position of the output buffer
		outBuffer.position(0);
		outBuffer.limit(outputStringBuilder.length());

		// Copy string builder to byte array
		int len = outputStringBuilder.length();
		for (int i = 0; i < len; i++) {
			outBuffer.array()[i] = (byte)(outputStringBuilder.charAt(i));
		}
		
		// Empty string builder
		outputStringBuilder.delete(0, outputStringBuilder.length());
		
		// Invalidate the next block (we will read the following block next time)
		block.invalidate();
		
		return true;
	}

	// Reads up to len bytes of data from this input stream into an array of bytes.
	public int read(byte[] b, int off, int len) throws IOException
	{
		// If out buffer empty, re-fill
		if (outBuffer.remaining() == 0) {
			fillOutputBuffer();
			// If out buffer still empty, must be eof
			if (outBuffer.remaining() == 0) {
				return -1;
			}
		}
				
		// At most, read to the end of the output buffer
		if (len > outBuffer.remaining()) {
			len = outBuffer.remaining();
		}
		
		// Read the bytes from the buffer
		outBuffer.get(b, off, len);
		return len;
	}
	
    // Skips over and discards n bytes of data from the input stream.
	public long skip(long length) throws IOException { 
		long remaining = length;
		while (remaining > 0)
		{
			int n = read(null, 0, (int)remaining);
			if (n == 0) { break; }
			remaining -= n;
		}
		return 0; 
	}
	
    // Returns the number of bytes that can be read from this input stream without blocking.
	public int available() { return outBuffer.remaining(); }
	
    // Reads up to byte.length bytes of data from this input stream into an array of bytes.
	public int read(byte[] b) throws IOException
	{
		return read(b, 0, b.length);
	}
	
	// Single byte buffer
	private byte[] singleByteBuffer = new byte[1];

    // Reads the next byte of data from this input stream.
	@Override
	public int read() throws IOException
	{
		if (read(singleByteBuffer, 0, 1) != 1) { return -1; }
		return singleByteBuffer[0];
	}

    // Closes this input stream and releases any system resources associated with the stream.
	public void close() throws IOException
	{
		cwaReader.close();
	}
	
    // Tests if this input stream supports the mark and reset methods.
	public boolean markSupported() { return false; }
	
    // Marks the current position in this input stream.
	public void mark(int readlimit) { /*throw new IOException("Mark not supported.");*/ }
	
    // Repositions this stream to the position at the time the mark method was last called on this input stream.
	public void reset() throws IOException { throw new IOException("Reset to mark not supported."); }
	

	/** Exposes the internal CwaReader object -- allows access to annotations, etc.
	 * @return CwaReader object
	 */
	public CwaReader getCwaReader() { return cwaReader; }
	
}
