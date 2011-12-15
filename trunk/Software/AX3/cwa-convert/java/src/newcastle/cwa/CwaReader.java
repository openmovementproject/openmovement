/**
 * CWA File Reader
 */
package newcastle.cwa;

import java.io.IOException;
import java.io.InputStream;
import java.net.URLDecoder;
import java.util.HashMap;
import java.util.Map;


/**
 * A binary CWA file reader class used by CwaCsvInputStream.
 * @author Dan Jackson, Newcastle University
 */
public class CwaReader {

	/** Wrapped input stream */
	private InputStream inputStream;
	
	/** Block */
	private CwaBlock currentBlock;
	
	/** Flag indicating whether the meta data has been read */
	private boolean metadataRead;
	private String metadataString;
	private int sessionId = -1;
	private short deviceId = -1;
	private Map<String, String> annotations = new HashMap<String, String>();
	
	/** @return Device ID */
	public short getDeviceId() { return deviceId; }
	
	
	/**
	 * Creates a CwaReader object
	 * @param inputStream the source input stream
	 */
	public CwaReader(InputStream inputStream) {
		this.inputStream = inputStream;
		currentBlock = new CwaBlock();
		metadataRead = false;
		//skipNonDataBlocks();
	}

	
	/** 
	 * Closes the wrapped input stream 
	 * @throws IOException if closing the input stream fails
	 */
	public void close() throws IOException {
		inputStream.close();
	}

	
	
	/**
	 * @return whether the metadata has been read.
	 */
	public boolean isMetadataRead() {
		return metadataRead;
	}

	
	/**
	 * @return metadata annotations
	 */
	public Map<String, String> getAnnotations() { 
		return annotations; 
	}
	
	
	/**
	 * Returns a *temporary* preview of the next block without consuming it.
	 * The object contents are mutable (for efficiency) and is only valid until the next read/peek call.
	 * @return The current block, null if end of file.
	 * @throws IOException if reading the input stream fails
	 */
	public CwaBlock peekBlock() throws IOException {
		// If no block buffered, read a new block
		if (!currentBlock.isValid()) {
			if (!currentBlock.readFromInputStream(inputStream)) {
				return null;
			}
		}
		if (currentBlock.isDataBlock() && currentBlock.getSessionId() != sessionId) {
			currentBlock.invalidate();			
		}
		// Return current block
		return currentBlock;
	}

	
	/**
	 * Skips a single block
	 * @return whether the block was successfully skipped
	 * @throws IOException if reading the input stream fails
	 */
	public boolean skipBlock() throws IOException {
		if (peekBlock() == null) {
			return false;
		}
		currentBlock.invalidate();
		return true;
	}

	

	/**
	 * Returns a copy of, and consumes the next block in the file.  
	 * Internally, this returns the current buffered block or a newly-read block if none is buffered.
	 * @return The next block in the file, null if end of file.
	 * @throws IOException if reading the input stream fails
	 */
	public CwaBlock readBlock() throws IOException {
		// If no block buffered, read a new block
		if (!currentBlock.isValid()) {
			currentBlock.readFromInputStream(inputStream);
		}
		// If invalid
		if (!currentBlock.isValid()) {
			return null;
		}
		// Return a copy
		CwaBlock returnBlock = new CwaBlock(currentBlock);
		// Invalidate the block
		currentBlock.invalidate();
		return returnBlock;
	}

	
	/** 
	 * Skips the number of specified blocks 
	 * @param toSkip the number of blocks to skip
	 * @return the number of blocks skipped 
	 * @throws IOException if reading the input stream fails
	 */
	public long skipBlocks(long toSkip) throws IOException {
		long numSkipped = 0;

		// Consume the current buffer block if it exists
		if (currentBlock.isValid() && toSkip > 0) {
			currentBlock.invalidate();
			numSkipped++;
			toSkip--;
		}
		
		// Seek the input stream forward
		numSkipped += inputStream.skip(toSkip * CwaBlock.BLOCK_SIZE) / CwaBlock.BLOCK_SIZE;
		
		return numSkipped;
	}

	
	/** 
	 * Consumes header and null blocks 
	 * @return the number of non-data blocks skipped
	 * @throws IOException 
	 */
	public long skipNonDataBlocks() throws IOException {
		long numSkipped = 0;
		for (;;)
		{
			// Peek at the next block
			CwaBlock block = peekBlock();
			// If EOF, finished (no data)
			if (block == null) { 
				break; 
			}
			// Determine type of block
			short blockType = block.getBlockType();
			
			// If header block, read contents
			if (blockType == CwaBlock.BLOCK_HEADER) {
				Map<String, String> labelMap = new HashMap<String, String>();
				// At device set-up time
				labelMap.put("_c", "studyCentre");
				labelMap.put("_s", "studyCode");
				labelMap.put("_i", "investigator");
				labelMap.put("_x", "exerciseCode");
				labelMap.put("_v", "volunteerNum");
				labelMap.put("_p", "bodyLocation");
				labelMap.put("_so", "setupOperator");
				labelMap.put("_n", "notes");
				// At retrieval time
				labelMap.put("_b", "startTime"); 
				labelMap.put("_e", "endTime"); 
				labelMap.put("_ro", "recoveryOperator"); 
				labelMap.put("_r", "retrievalTime"); 
				labelMap.put("_co", "comments");
				
				// Process fist sector (metadata annotations)
				sessionId = block.getSessionId();
				deviceId = block.getDeviceId();
				StringBuilder sb = new StringBuilder();				
				
				for (int i = 0; i < 448; i++) {
					char c = (char)block.buffer().get(64 + i);
					if (c != ' ' && c >= 0 && c < 128) {
						if (c == '?') { c = '&'; }
						sb.append(c);
					}
				}
				
				sb.append('&');
				
				// Process second sector (edited metadata annotations)
				block.invalidate();
				readBlock();
				
				for (int i = 0; i < 512; i++) {
					char c = (char)block.buffer().get(0 + i);
					if (c != ' ' && c >= 32 && c < 128) {
						if (c == '?') { c = '&'; }
						sb.append(c);
					}
				}
				
				metadataString = sb.toString().trim();
				String[] pairs = metadataString.split("&");
				
				for (String pair : pairs) {
					int i = pair.indexOf('=');
					String name = pair;
					String value = "";
					if (i >= 0)
					{
						name = pair.substring(0, i);
						value = pair.substring(i + 1);
					}
					if (value.trim().length() > 0 && name.trim().length() > 0)
					{
						name = URLDecoder.decode(name, "UTF-8");
						value = URLDecoder.decode(value, "UTF-8");
				
						if (labelMap.containsKey(name)) { name = labelMap.get(name); }

						annotations.put(name, value);
					}
				}
				metadataRead = true;
			}
			// If first item of data, peek at contents then exit loop
			else if (blockType == CwaBlock.BLOCK_DATA) {
				metadataRead = true;
				break;		// exit loop if data seen
			}
			
			// If not data then consume block
			readBlock();
		}		
		return numSkipped;
	}


}
