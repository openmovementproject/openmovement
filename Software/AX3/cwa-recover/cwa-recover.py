#!/usr/bin/env python
# coding=UTF-8
# CWA Dump File Recovery

import sys
import os
from struct import *

def checksum512(data):
  sum = 0
  for o in range(256):
    value = data[2 * o] | (data[2 * o + 1] << 8)
    sum = (sum + value) & 0xffff
  return sum

def singleBit(value):
  if value != 0 and not (value & (value - 1)):
    return value
  else:
    return 0

def _fast_timestamp(value):
    """Faster date/time parsing.  This does not include 'always' limits; invalid dates do not cause an error; the first call will be slower as a lookup table is created."""
    # On first run, build lookup table for initial 10-bits of the packed date-time parsing, minus one day as days are 1-indexed
    if not hasattr(_fast_timestamp, "SECONDS_BEFORE_YEAR_MONTH"):
        _fast_timestamp.SECONDS_BEFORE_YEAR_MONTH = [0] * 1024        # YYYYYYMM MM (Y=years since 2000, M=month-of-year 1-indexed)
        SECONDS_PER_DAY = 24 * 60 * 60
        DAYS_IN_MONTH = [ 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 0, 0, 0 ]    # invalid month 0, months 1-12 (non-leap-year), invalid months 13-15
        seconds_before = 946684800      # Seconds from UNIX epoch (1970) until device epoch (2000)
        for year in range(0, 64):       # 2000-2063
            for month in range(0, 16):  # invalid month 0, months 1-12, invalid months 13-15
                index = (year << 4) + month
                _fast_timestamp.SECONDS_BEFORE_YEAR_MONTH[index] = seconds_before - SECONDS_PER_DAY    # minus one day as day-of-month is 1-based
                days = DAYS_IN_MONTH[month]
                if year % 4 == 0 and month == 2:    # Correct for this year range (2000 was a leap year, despite being a multiple of 100, as it is a multiple of 400)
                    days += 1
                seconds_before += days * SECONDS_PER_DAY

    year_month = (value >> 22) & 0x3ff
    day   = (value >> 17) & 0x1f
    hours = (value >> 12) & 0x1f
    mins  = (value >>  6) & 0x3f
    secs  = value & 0x3f
    return _fast_timestamp.SECONDS_BEFORE_YEAR_MONTH[year_month] + ((day * 24 + hours) * 60 + mins) * 60 + secs


def recoverCwa(inputFile, outputFile, method, modifyFlags):
  initialOffset = 0 # 0x20000 in drive dumps
  sectorSize = 512
  headerSize = 2 * sectorSize
  paddingMax = 128  # Allow first few bytes to be garbage
  # allowedPadding = [ 0xe0, 0xf0 ] # Allowed garbage bytes
  globalSessionId = None
  globalDeviceId = None

  idxTimestamp = method.index('t')
  idxSession = method.index('s')
  idxSequenceId = method.index('q')
  
  print("Reading input: ", inputFile)
  
  with open(inputFile, 'rb') as fi:
    fi.seek(0, os.SEEK_END) # 2
    fileSize = fi.tell() # 992161*512
    fi.seek(0, os.SEEK_SET) # 0
    fileData = fi.read(fileSize)
  
  # Store found data sectors
  metadata = [] # (offset,size,sessionId)
  data = []     # (offset,size,sessionId,sequenceId,
  
  countCorrectedSession = 0
  countCorrectedSequence = 0
  countChecksumErrors = 0
  maxChecksumErrors = 10000
  
  numSectors = fileSize // sectorSize
  print("Processing " + str(numSectors) + " sectors...")
  lastPerc = -1
  for i in range(numSectors):
    block = fileData[i * sectorSize:(i + 1) * sectorSize]
    perc = (100 * i) // numSectors
    if 5 * (perc // 5) != lastPerc:
      print("..." + str(i) + "/" + str(numSectors) + " = " + str(perc) + "%...")
      lastPerc = perc
      
    # Skip if 0xff
    if block[0] == 255:
      continue
    
    for o in range(paddingMax):
      ofs = i * sectorSize + o
      # Check if this is a data block
      if block[o] == ord('A') and block[o + 1] == ord('X') and block[o + 2] == 0xfc and block[o+3] == 0x01:
        completeBlock = (o == 0)
        fileOffset = i * sectorSize + o
        blockLength = sectorSize - o
        sessionId = unpack('<I', block[o+6:o+10])[0]
        
        rateCode = unpack('B', block[24:25])[0]                   # @24  +1   Sample rate code, frequency (3200/(1<<(15-(rate & 0x0f)))) Hz, range (+/-g) (16 >> (rate >> 6)).
        frequency = 3200 / (1 << (15 - (rateCode & 0x0f)))        
        
        timestamp = _fast_timestamp(unpack('<I', block[o+14:o+18])[0])
        deviceFractional = (timestamp << 16) + ((unpack('<H', block[o+4:o+6])[0] & 0x8fff) << 1)
        timestampOffset = unpack('<h', block[26:28])[0]           # @26  +2   Relative sample index from the start of the buffer where the whole-second timestamp is valid
        
        timeFractional = 0;
        # Need to undo backwards-compatible shim by calculating how many whole samples the fractional part of timestamp accounts for.
        timeFractional = (deviceFractional & 0x7fff) << 1     # use original deviceId field bottom 15-bits as 16-bit fractional time
        timestampOffset += (timeFractional * int(frequency)) >> 16 # undo the backwards-compatible shift (as we have a true fractional)

        # Add fractional time to timestamp
        timestamp += timeFractional / 65536
                    
        sequenceId = unpack('<I', block[o+10:o+14])[0]
        
        # Report on mismatched checksums
        if countChecksumErrors < maxChecksumErrors:
          if completeBlock:
            checksum = unpack('<H', block[o+510:o+512])[0]
            actualChecksum = checksum512(block)
            if actualChecksum != 0:
              countChecksumErrors += 1
              print("Mismatched checksum #" + str(countChecksumErrors) + " at " + str(i) + " = " + str(actualChecksum) + " (" + str(checksum) + ")")
              if countChecksumErrors >= maxChecksumErrors:
                print("NOTE: No more checksum errors will be reported!")

        # For an incomplete block
        if not completeBlock:
          # Check basic information
          if sessionId != globalSessionId and (modifyFlags & 4 != 0):
            x = sessionId ^ globalSessionId
            bitValue = singleBit(x)
            if bitValue:
              print("Corrected probable corrupted session ID: " + str(sessionId) + " (" + str(globalSessionId) + ") ^" + hex(x))
              sessionId ^= bitValue
              countCorrectedSession += 1
            else:
              print("Uncorrected mismatched session ID: " + str(sessionId) + " (" + str(globalSessionId) + ") ^" + hex(x))
              
          # Check basic information
          nextSequence = None
          if len(data) > 0:
            lastItem = data[len(data) - 1]
            nextSequence = lastItem[idxSequenceId] + 1
          if nextSequence != None and sequenceId != nextSequence and (modifyFlags & 8 != 0):
            x = sequenceId ^ nextSequence
            bitValue = singleBit(x)
            if bitValue:
              print("Corrected possible corrupted sequence ID: " + str(sequenceId) + " (" + str(nextSequence) + ") ^" + hex(x))
              sequenceId ^= bitValue
              countCorrectedSequence += 1
            else:
              # print("NOTE: Non-consecutive sequence ID: " + str(sequenceId) + " (" + str(nextSequence) + ") ^" + hex(x))
              pass
        
        item = [0, 0, 0, fileOffset, blockLength]
        item[idxSession] = sessionId
        item[idxTimestamp] = timestamp
        item[idxSequenceId] = sequenceId
        
        data.append(tuple(item))
        break
      # Check if this is a header block
      if block[o] == ord('M') and block[o + 1] == ord('D') and block[o + 2] == 0xfc and block[o+3] == 0x03:
        fileOffset = i * sectorSize + o
        blockLength = headerSize - o
        deviceId = unpack('<H', block[o+5:o+7])[0]			# @ 5  +2   Device identifier
        sessionId = unpack('<I', block[o+7:o+11])[0]
        deviceIdUpper = unpack('<H', block[o+11:o+13])[0]		# @ 11  +2   Upper device identifier
        if deviceIdUpper == 0xffff:
            deviceIdUpper = 0x0000
        print("Found header with session ID: " + str(sessionId) + " (device=" + str(deviceId) + ")")
        globalDeviceId = (deviceIdUpper << 16) + deviceId
        globalSessionId = sessionId
        metadata.append((fileOffset, blockLength, sessionId))
        break
  
  print("Found: " + str(len(metadata)) + " raw metadata block(s), " + str(len(data)) + " raw data block(s)")
  print("Corrected: " + str(countCorrectedSession) + " session IDs, and " + str(countCorrectedSequence) + " sequences")
  
  print("Sorting data blocks...")
  data = sorted(data)
  
  print("Writing output: ", outputFile)
  with open(outputFile, 'wb') as fo:
    if len(metadata) > 0:
      if len(metadata) > 1:
        print("WARNING: Multiple header blocks found, the first one will be used (some readers may not parse the whole data if the data block session-id does not match the one in the header")
      fileOffset = metadata[0][0]
      blockLength = metadata[0][1]
      globalSessionId = metadata[0][2]
      print("Using session ID: " + str(globalSessionId))
      block = bytearray(fileData[fileOffset:fileOffset + headerSize])  # blockLength
      if blockLength < headerSize:
        # Pad block with missing bytes
        missingBytes = headerSize - blockLength
        for o in range(missingBytes):
          block[blockLength + o] = 0xff
      fo.write(block)
    else:
      print("WARNING: No header block found, this utility does not yet create one, so the output file will not be valid")
    
    totalSamples = 0
    numOffsetSectors = 0
    totalMissingSamples = 0 # from offsets (not skipped sectors)
    numSkippedSectors = 0
    numDuplicates = 0
    numOtherSession = 0
    numOutOfSequence = 0
    numBackwards = 0
    numWritten = 0
    lastData = None
    lastPerc = -1
    for i in range(len(data)):
      # Progress
      perc = (100 * i) // len(data)
      if 5 * (perc // 5) != lastPerc:
        print("..." + str(i) + "/" + str(len(data)) + " = " + str(perc) + "%...")
        lastPerc = perc

      sessionId = data[i][idxSession]
      timestamp = data[i][idxTimestamp]
      sequenceId = data[i][idxSequenceId]
      fileOffset = data[i][3]
      fileSector = data[i][3] // sectorSize
      blockLength = data[i][4]
      if lastData != None:
        prevSessionId = lastData[idxSession]
        prevTimestamp = lastData[idxTimestamp]
        prevSequenceId = lastData[idxSequenceId]
        prevFileSector = lastData[3] // sectorSize
      else:
        prevSessionId = None
        prevTimestamp = None
        prevSequenceId = None
        prevFileSector = None
      
      print("Timestamp: " + str(timestamp))
      
      block = bytearray(fileData[fileOffset:fileOffset + sectorSize]) # blockLength
      
      # Patch-in possibly updated values
      if (modifyFlags & 12) != 0:
        pack_into('<I', block, 6, sessionId)
        pack_into('<I', block, 10, sequenceId)
      
      # Resequence
      if (modifyFlags & 1) != 0:
        if prevSequenceId is not None and sequenceId != prevSequenceId + 1:
          nextSequence = prevSequenceId + 1
          print("Resequenced: " + str(sequenceId) + " -> " + str(nextSequence))
          sequenceId = nextSequence
          pack_into('<I', block, 10, sequenceId)
          # Recalculate checksum
          pack_into('<H', block, 510, 0)
          checksumAtZero = checksum512(block)
          pack_into('<H', block, 510, (~checksumAtZero + 1) & 0xffff)
      
      # Trace
      if False:
        print("#" + str(i) + " session=" + str(sessionId) + " t=" + str(timestamp) +  " sequence=" + str(sequenceId) + " @" + str(fileOffset) + "+" + str(blockLength) + " ")
      
      if prevFileSector != None and fileSector != prevFileSector + 1:
        jump = prevFileSector - fileSector
        # print("NOTE: #" + str(i) + " Non-consecutive sectors (advanced by " + str(jump) + "), next was " + str(prevFileSector+1) + " at " + str((prevFileSector+1) * sectorSize) + "")
      
      if lastData != None and timestamp < prevTimestamp:
        print("WARNING: #" + str(i) + " Found a jump back in time: " + str(prevTimestamp) + " to " + str(timestamp))
        numBackwards += 1
        
      if lastData != None and sessionId == prevSessionId and sequenceId == prevSequenceId and timestamp == prevTimestamp:
        # print("WARNING: #" + str(i) + " Ignoring a duplicate sector: session=" + str(sessionId) + " sequence=" + str(sequenceId) + " timestamp=" + str(timestamp))
        numDuplicates += 1
        continue
        
      if lastData != None and sessionId == prevSessionId and sequenceId != prevSequenceId + 1:
        missing = sequenceId - prevSequenceId - 1
        if missing > 0 and missing < 10:
          print("WARNING: #" + str(i) + " Missing " + str(missing) + " sectors, skipped from sequence id: " + str(prevSequenceId) + " to " + str(sequenceId))
          numSkippedSectors += missing
        else:
          print("WARNING: #" + str(i) + " Found a non-consecutive sequence ID in session " + str(sessionId) + ": " + str(prevSequenceId) + " to " + str(sequenceId))
          numOutOfSequence += 1
        
      if sessionId != globalSessionId:
        print("WARNING: #" + str(i) + " Mismatched session ID: " + str(sessionId) + " but header is " + str(globalSessionId) + " file offset " + str(fileOffset))
        numOtherSession += 1
        # Resequence
        if (modifyFlags & 2) != 0:
            pack_into('<I', block, 6, globalSessionId)
            # Recalculate checksum
            pack_into('<H', block, 510, 0)
            checksumAtZero = checksum512(block)
            pack_into('<H', block, 510, (~checksumAtZero + 1) & 0xffff)
            
      
      missingBytes = 0
      if blockLength < sectorSize:
        # Pad block with missing bytes
        missingBytes = sectorSize - blockLength
        print("NOTE: Missing bytes: " + str(missingBytes))
        for o in range(missingBytes):
          block[blockLength + o] = 0
      
      # Determine number of bytes per sample
      numAxesBPS = block[25]
      channels = (numAxesBPS >> 4) & 0x0f
      bytesPerAxis = numAxesBPS & 0x0f
      bytesPerSample = 0
      if bytesPerAxis == 0 and channels == 3:
        bytesPerSample = 4
      elif bytesPerAxis > 0 and channels > 0:
        bytesPerSample = bytesPerAxis * channels
      
      if missingBytes > 0:
        # Calculate number of missing samples
        missingSamples = 0
        if missingBytes > 2 and bytesPerSample > 0: # after checksum
          missingSamples = (missingBytes - 2 + bytesPerSample - 1) // bytesPerSample
        
        print("NOTE: Missing samples: " + str(missingSamples) + " -- " + bytesPerSample + " bytes-per-sample")
          
        # Are the any missing samples?
        if missingSamples > 0:
          totalMissingSamples += missingSamples
          numOffsetSectors += 1
          sampleOffset = 30 + 480 - (missingSamples * bytesPerSample)
          # TODO: Fill in mean of samples?
        
        # Checksum invalid if any missing bytes
        if missingBytes > 0:
          # TODO: recalculate checksum instead of storing zero
          block[510] = 0x00
          block[511] = 0x00
      
      fo.write(block)
      lastData = data[i]
      numWritten += 1
      if bytesPerSample != 0:
        totalSamples += (480 / bytesPerSample)

    print("Wrote " + str(numWritten) + " sectors")
    print("Duplicates: " + str(numDuplicates))
    print("Out-of-sequence: " + str(numOutOfSequence))
    print("Backward jumps: " + str(numBackwards))
    print("Missing sectors: " + str(numSkippedSectors))
    print("Mismatching Session ID: " + str(numOtherSession))
    print("Number of sectors with initial damage: " + str(numOffsetSectors))
    print("Total number of missing samples from initial damage: " + str(totalMissingSamples))
    print("Total samples: " + str(totalSamples))
    if globalSessionId == None:
      print("WARNING: No header block was written (output file will not be valid)")


def main():
  print("Running...")

  method = 'sqt'
  inputFile = None
  outputFile = None
  modifyFlags = 0
  arg = 1
  while arg < len(sys.argv):
    if sys.argv[arg].startswith("-"):
      if sys.argv[arg] == "--method-sqt":
        # 'sqt' - sessions must be unique and sequence may not reset
        method = "sqt"
      elif sys.argv[arg] == "--method-tsq":
        # 'tsq' - clock not reset, session don't have to be unique, sequence may reset
        method = "tsq"
      elif sys.argv[arg] == "--method-stq":
        # 'stq' - clock may be reset, sessions were unique, sequence may reset
        method = "stq"
      elif sys.argv[arg] == "--modify":
        arg += 1
        modifyFlags = int(sys.argv[arg])
      else:
        print("ERROR: Unrecognized option: " + sys.argv[arg])
        return
    elif inputFile == None:
      inputFile = sys.argv[arg]
    elif outputFile == None:
      outputFile = sys.argv[arg]
    else:
      print("ERROR: Unrecognized positional argument: " + sys.argv[arg])
      return
    arg += 1

  if inputFile is None:
    inputFile = "cwa-dump.img"
  if outputFile is None:
    outputFile = "cwa-recover.cwa"

  print("NOTE: Using input file:", inputFile)
  if not os.path.exists(inputFile):
    print("ERROR: Input file does not exist:", inputFile)
    return
  
  print("NOTE: Using output file:", outputFile)
  if os.path.exists(outputFile):
    print("ERROR: Output file already exists, must remove or use another output file:", outputFile)
    return
  
  return recoverCwa(inputFile, outputFile, method, modifyFlags)

if __name__ == "__main__":
  main()
