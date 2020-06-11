# CWA Metadata Reader by Dan Jackson, 2017.
#

import sys
from struct import *
import time
from datetime import datetime


def read_timestamp(data):
	value = unpack('<I', data)[0]
	if value == 0x00000000:	# Infinitely in past = 'always before now'
		return 0
	if value == 0xffffffff:	# Infinitely in future = 'always after now'
		return -1
	# bit pattern:  YYYYYYMM MMDDDDDh hhhhmmmm mmssssss
	year  = ((value >> 26) & 0x3f) + 2000
	month = (value >> 22) & 0x0f
	day   = (value >> 17) & 0x1f
	hours = (value >> 12) & 0x1f
	mins  = (value >>  6) & 0x3f
	secs  = (value >>  0) & 0x3f
	try:
		dt = datetime(year, month, day, hours, mins, secs)
		timestamp = (dt - datetime(1970, 1, 1)).total_seconds()
		return timestamp
		# return str(datetime.fromtimestamp(timestamp))
		# return time.strptime(t, '%Y-%m-%d %H:%M:%S')
	except ValueError:
		print("WARNING: Invalid date:", year, month, day, hours, mins, secs)
		return -1

# 16-bit checksum (should sum to zero)
def checksum(data):
	sum = 0
	for i in range(0, len(data), 2):
		#value = data[i] | (data[i + 1] << 8)
		value = unpack('<H', data[i:i+2])[0]
		sum = (sum + value) & 0xffff
	return sum

def short_sign_extend(value):
    return ((value + 0x8000) & 0xffff) - 0x8000

def timestamp_string(timestamp):
	if timestamp == 0:
		return "0"
	if timestamp < 0:
		return "-1"
	# return str(datetime.fromtimestamp(timestamp))
	return datetime.fromtimestamp(timestamp).strftime("%Y-%m-%d %H:%M:%S.%f")[:23]


# Local "URL-decode as UTF-8 string" function
def urldecode(input):
	output = bytearray()
	nibbles = 0
	value = 0
	# Each input character
	for char in input:
		if char == '%':
			# Begin a percent-encoded hex pair
			nibbles = 2
			value = 0
		elif nibbles > 0:
			# Parse the percent-encoded hex digits
			value *= 16
			if char >= 'a' and char <= 'f':
				value += ord(char) + 10 - ord('a')
			elif char >= 'A' and char <= 'F':
				value += ord(char) + 10 - ord('A')
			elif char >= '0' and char <= '9':
				value += ord(char) - ord('0')
			nibbles -= 1
			if nibbles == 0:
				output.append(value)
		elif char == '+':
			# Treat plus as space (application/x-www-form-urlencoded)
			output.append(ord(' '))
		else:
			# Preserve character
			output.append(ord(char))
	return output.decode('utf-8')


def cwa_parse_metadata(data):
	# Metadata represented as a dictionary
	metadata = {}
	
	# Shorthand name expansions
	shorthand = {
		"_c":  "Study Centre", 
		"_s":  "Study Code", 
		"_i":  "Investigator", 
		"_x":  "Exercise Code", 
		"_v":  "Volunteer Num", 
		"_p":  "Body Location", 
		"_so": "Setup Operator", 
		"_n":  "Notes", 
		"_b":  "Start time", 
		"_e":  "End time", 
		"_ro": "Recovery Operator", 
		"_r":  "Retrieval Time", 
		"_co": "Comments", 
		"_sc": "Subject Code", 
		"_se": "Sex", 
		"_h":  "Height", 
		"_w":  "Weight", 
		"_ha": "Handedness", 
	}
	
	# CWA File has 448 bytes of metadata at offset 64
	if sys.version_info[0] < 3:
		encString = str(data)
	else:
		encString = str(data, 'ascii')
	
	# Remove any trailing spaces, null, or 0xFF bytes
	encString = encString.rstrip('\x20\xff\x00')
	
	# Name-value pairs separated with ampersand
	nameValues = encString.split('&')
	
	# Each name-value pair separated with an equals
	for nameValue in nameValues:
		parts = nameValue.split('=')
		# Name is URL-encoded UTF-8
		name = urldecode(parts[0])
		if len(name) > 0:
			value = None
			
			if len(parts) > 1:
				# Value is URL-encoded UTF-8
				value = urldecode(parts[1])
			
			# Expand shorthand names
			name = shorthand.get(name, name)
			
			# Store metadata name-value pair
			metadata[name] = value
	
	# Metadata dictionary
	return metadata


def cwa_header(block):
	header = {}
	if len(block) >= 512:
		packetHeader = unpack('BB', block[0:2])							# @ 0  +2   ASCII "MD", little-endian (0x444D)
		packetLength = unpack('<H', block[2:4])[0]						# @ 2  +2   Packet length (1020 bytes, with header (4) = 1024 bytes total)
		if packetHeader[0] == ord('M') and packetHeader[1] == ord('D') and packetLength >= 508:
			header['packetLength'] = packetLength
			# unpack() <=little-endian, bB=s/u 8-bit, hH=s/u 16-bit, iI=s/u 32-bit		
			hardwareType = unpack('B', block[4:5])[0]					# @ 4  +1   Hardware type (0x00/0xff/0x17 = AX3, 0x64 = AX6)
			header['hardwareType'] = hardwareType
			if hardwareType == 0x00 or hardwareType == 0xff:
				hardwareType = 0x17
			if hardwareType == 0x17:
				header['deviceType'] = 'AX3'
			elif hardwareType == 0x64:
				header['deviceType'] = 'AX6'
			else:
				header['deviceType'] = hex(hardwareType)[2:] # BCD
			header['deviceId'] = unpack('<H', block[5:7])[0]			# @ 5  +2   Device identifier
			header['sessionId'] = unpack('<I', block[7:11])[0]			# @ 7  +4   Unique session identifier
			deviceIdUpper = unpack('<H', block[11:13])[0]				# @11  +2   Upper word of device id (if 0xffff is read, treat as 0x0000)
			if deviceIdUpper != 0xffff:
				header['deviceId'] |= deviceIdUpper << 16
			header['loggingStart'] = read_timestamp(block[13:17])		# @13  +4   Start time for delayed logging
			header['loggingEnd'] = read_timestamp(block[17:21])			# @17  +4   Stop time for delayed logging		
			header['loggingCapacity'] = unpack('<I', block[21:25])[0]	# @21  +4   (Deprecated: preset maximum number of samples to collect, 0 = unlimited)
			# header['reserved3'] = block[25:26]						# @25  +1   (1 byte reserved)
			header['flashLed'] = unpack('B', block[35:36])[0]			# @26  +1   Flash LED during recording
			if header['flashLed'] == 0xff:
				header['flashLed'] = 0
			# header['reserved4'] = block[27:35]						# @25  +8   (8 bytes reserved)
			sensorConfig = unpack('B', block[35:36])[0]					# @35  +1   Fixed rate sensor configuration, 0x00 or 0xff means accel only, otherwise bottom nibble is gyro range (8000/2^n dps): 2=2000, 3=1000, 4=500, 5=250, 6=125, top nibble non-zero is magnetometer enabled.
			if sensorConfig != 0x00 and sensorConfig != 0xff:
				header['gyroRange'] = 8000 / 2 ** (sensorConfig & 0x0f)
			else:
				header['gyroRange'] = 0
			rateCode = unpack('B', block[36:37])[0]						# @36  +1   Sampling rate code, frequency (3200/(1<<(15-(rate & 0x0f)))) Hz, range (+/-g) (16 >> (rate >> 6)).
			header['lastChange'] = read_timestamp(block[37:41])			# @37  +4   Last change metadata time
			header['firmwareRevision'] = unpack('B', block[41:42])[0]	# @41  +1   Firmware revision number
			# header['timeZone'] = unpack('<H', block[42:44])[0]		# @42  +2   (Unused: originally reserved for a "Time Zone offset from UTC in minutes", 0xffff = -1 = unknown)
			# header['reserved5'] = block[44:64]						# @44  +20  (20 bytes reserved)
			header['metadata'] = cwa_parse_metadata(block[64:512])		# @64  +448 "Annotation" meta-data (448 ASCII characters, ignore trailing 0x20/0x00/0xff bytes, url-encoded UTF-8 name-value pairs)
			# header['reserved'] = block[512:1024]						# @512 +512 Reserved for device-specific meta-data (512 bytes, ASCII characters, ignore trailing 0x20/0x00/0xff bytes, url-encoded UTF-8 name-value pairs, leading '&' if present?)
			
			# Timestamps
			header['loggingStartTime'] = timestamp_string(header['loggingStart'])
			header['loggingEndTime'] = timestamp_string(header['loggingEnd'])
			header['lastChangeTime'] = timestamp_string(header['lastChange'])
			
			# Parse rateCode
			header['sampleRate'] = (3200/(1<<(15-(rateCode & 0x0f))))
			header['accelRange'] = (16 >> (rateCode >> 6))
		
	return header


def cwa_data(block, extractData=False):
	data = {}
	if len(block) >= 512:
		packetHeader = unpack('BB', block[0:2])							# @ 0  +2   ASCII "AX", little-endian (0x5841)
		packetLength = unpack('<H', block[2:4])[0]						# @ 2  +2   Packet length (508 bytes, with header (4) = 512 bytes total)
		if packetHeader[0] == ord('A') and packetHeader[1] == ord('X') and packetLength == 508 and checksum(block[0:512]) == 0:
			#checksum = unpack('<H', block[510:512])[0]					# @510 +2   Checksum of packet (16-bit word-wise sum of the whole packet should be zero)
			
			deviceFractional = unpack('<H', block[4:6])[0]				# @ 4  +2   Top bit set: 15-bit fraction of a second for the time stamp, the timestampOffset was already adjusted to minimize this assuming ideal sample rate; Top bit clear: 15-bit device identifier, 0 = unknown;
			data['sessionId'] = unpack('<I', block[6:10])[0]			# @ 6  +4   Unique session identifier, 0 = unknown
			data['sequenceId'] = unpack('<I', block[10:14])[0]			# @10  +4   Sequence counter (0-indexed), each packet has a new number (reset if restarted)
			timestamp = read_timestamp(block[14:18])					# @14  +4   Last reported RTC value, 0 = unknown
			light = unpack('<H', block[18:20])[0]						# @18  +2   Last recorded light sensor value in raw units, 0 = none
			# data['temperature'] = unpack('<H', block[20:22])[0]		# @20  +2   Last recorded temperature sensor value in raw units, 0 = none
			# data['events'] = unpack('B', block[22:23])[0]				# @22  +1   Event flags since last packet, b0 = resume logging, b1 = reserved for single-tap event, b2 = reserved for double-tap event, b3 = reserved, b4 = reserved for diagnostic hardware buffer, b5 = reserved for diagnostic software buffer, b6 = reserved for diagnostic internal flag, b7 = reserved)
			# data['battery'] = unpack('B', block[23:24])[0]			# @23  +1   Last recorded battery level in raw units, 0 = unknown
			rateCode = unpack('B', block[24:25])[0]					    # @24  +1   Sample rate code, frequency (3200/(1<<(15-(rate & 0x0f)))) Hz, range (+/-g) (16 >> (rate >> 6)).
			numAxesBPS = unpack('B', block[25:26])[0]					# @25  +1   0x32 (top nibble: number of axes = 3; bottom nibble: packing format - 2 = 3x 16-bit signed, 0 = 3x 10-bit signed + 2-bit exponent)
			timestampOffset = unpack('<h', block[26:28])[0]				# @26  +2   Relative sample index from the start of the buffer where the whole-second timestamp is valid
			data['sampleCount'] = unpack('<H', block[28:30])[0]			# @28  +2   Number of accelerometer samples (80 or 120 if this sector is full)
			# rawSampleData[480] = block[30:510]						# @30  +480 Raw sample data.  Each sample is either 3x 16-bit signed values (x, y, z) or one 32-bit packed value (The bits in bytes [3][2][1][0]: eezzzzzz zzzzyyyy yyyyyyxx xxxxxxxx, e = binary exponent, lsb on right)
			
			# range = 16 >> (rateCode >> 6)
			frequency = 3200 / (1 << (15 - (rateCode & 0x0f)))
			
			timeFractional = 0;
			# if top-bit set, we have a fractional date
			if deviceFractional & 0x8000:
				# Need to undo backwards-compatible shim by calculating how many whole samples the fractional part of timestamp accounts for.
				timeFractional = (deviceFractional & 0x7fff) << 1     # use original deviceId field bottom 15-bits as 16-bit fractional time
				timestampOffset += (timeFractional * int(frequency)) >> 16 # undo the backwards-compatible shift (as we have a true fractional)
			
			# Add fractional time to timestamp
			timestamp += timeFractional / 65536

			data['timestamp'] = timestamp
			data['timestampOffset'] = timestampOffset
			
			data['timestampTime'] = timestamp_string(data['timestamp'])
			
			# Maximum samples per sector
			channels = (numAxesBPS >> 4) & 0x0f
			bytesPerAxis = numAxesBPS & 0x0f
			bytesPerSample = 4
			if bytesPerAxis == 0 and channels == 3:
				bytesPerSample = 4
			elif bytesPerAxis > 0 and channels > 0:
				bytesPerSample = bytesPerAxis * channels
			samplesPerSector = 480 // bytesPerSample
			data['channels'] = channels
			data['bytesPerAxis'] = bytesPerAxis			# 0 for DWORD packing
			data['bytesPerSample'] = bytesPerSample
			data['samplesPerSector'] = samplesPerSector

			# Axes
			accelAxis = -1
			gyroAxis = -1
			magAxis = -1
			if channels >= 6:
				gyroAxis = 0
				accelAxis = 3
				if channels >= 9:
					magAxis = 6
			elif channels >= 3:
				accelAxis = 0
			
			# Default units/scaling/range
			accelUnit = 256		# 1g = 256
			gyroRange = 2000	# 32768 = 2000dps
			magUnit = 16		# 1uT = 16
			# light is least significant 10 bits, accel scale 3-MSB, gyro scale next 3 bits: AAAGGGLLLLLLLLLL
			accelScale = 1 << (8 + ((light >> 13) & 0x07))
			if ((light >> 10) & 0x07) != 0:
				gyroRange = 8000 // (1 << ((light >> 10) & 0x07))
			
			# Scale
			#accelScale = 1.0 / accelUnit
			#gyroScale = float(gyroRange) / 32768
			#magScale = 1.0 / magUnit

			# Range
			accelRange = 16
			if rateCode != 0:
				accelRange = 16 >> (rateCode >> 6)
			#magRange = 32768 / magUnit
			
			# Unit
			gyroUnit = 32768.0 / gyroRange

			if accelAxis >= 0:
				data['accelAxis'] = accelAxis
				data['accelRange'] = accelRange
				data['accelUnit'] = accelUnit
			if gyroAxis >= 0:
				data['gyroAxis'] = gyroAxis
				data['gyroRange'] = gyroRange
				data['gyroUnit'] = gyroUnit
			if magAxis >= 0:
				data['magAxis'] = magAxis
				data['magRange'] = magRange
				data['magUnit'] = magUnit
			
			# Read sample values
			if extractData:
				if accelAxis >= 0:
					accelSamples = [[0, 0, 0]] * data['sampleCount']
					if bytesPerAxis == 0 and channels == 3:
						for i in range(data['sampleCount']):
							ofs = 30 + i * 4
							#val =  block[i] | (block[i + 1] << 8) | (block[i + 2] << 8) | (block[i + 3] << 24)
							val = unpack('<I', block[ofs:ofs + 4])[0]
							ex = (6 - ((val >> 30) & 3))
							accelSamples[i][0] = (short_sign_extend((0xffc0 & (val <<  6))) >> ex) / accelUnit
							accelSamples[i][1] = (short_sign_extend((0xffc0 & (val >>  4))) >> ex) / accelUnit
							accelSamples[i][2] = (short_sign_extend((0xffc0 & (val >> 14))) >> ex) / accelUnit
					elif bytesPerSample == 2:
						for i in range(data['sampleCount']):
							ofs = 30 + (i * 2 * channels) + 2 * accelAxis
							accelSamples[i][0] = (block[ofs + 0] | (block[ofs + 1] << 8)) / accelUnit
							accelSamples[i][1] = (block[ofs + 2] | (block[ofs + 3] << 8)) / accelUnit
							accelSamples[i][2] = (block[ofs + 4] | (block[ofs + 5] << 8)) / accelUnit
					data['samplesAccel'] = accelSamples
				
				if gyroAxis >= 0 and bytesPerSample == 2:
					gyroSamples = [[0, 0, 0]] * data['sampleCount']
					for i in range(data['sampleCount']):
						ofs = 30 + (i * 2 * channels) + 2 * gyroAxis
						gyroSamples[i][0] = (block[ofs + 0] | (block[ofs + 1] << 8)) / gyroUnit
						gyroSamples[i][1] = (block[ofs + 2] | (block[ofs + 3] << 8)) / gyroUnit
						gyroSamples[i][2] = (block[ofs + 4] | (block[ofs + 5] << 8)) / gyroUnit
					data['samplesGyro'] = gyroSamples
				
				if magAxis >= 0 and bytesPerSample == 2:
					magSamples = [[0, 0, 0]] * data['sampleCount']
					for i in range(data['sampleCount']):
						ofs = 30 + (i * 2 * channels) + 2 * magAxis
						magSamples[i][0] = (block[ofs + 0] | (block[ofs + 1] << 8)) / magUnit
						magSamples[i][1] = (block[ofs + 2] | (block[ofs + 3] << 8)) / magUnit
						magSamples[i][2] = (block[ofs + 4] | (block[ofs + 5] << 8)) / magUnit
					data['samplesMag'] = magSamples
			
			# Light
			light &= 0x3ff		# actual light value is least significant 10 bits

	return data


def cwa_info(filename):
	file = {}
	header = {}
	first = {}
	last = {}
	
	file['name'] = os.path.basename(filename)
	
	# Header
	with open(filename, "rb") as f:
		sectorSize = 512
		
		# File length
		f.seek(0, 2)
		fileSize = f.tell()
		
		# Read header
		headerSize = 1024
		f.seek(0)
		headerBytes = f.read(headerSize)
		header = cwa_header(headerBytes)
		if 'packetLength' not in header:
			raise Exception('Header invalid')
		headerSize = header['packetLength'] + 4
		
		# Read first data sector
		f.seek(headerSize)
		firstBytes = f.read(sectorSize)
		first = cwa_data(firstBytes)
		
		# Read last data sector
		f.seek(fileSize - sectorSize)
		lastBytes = f.read(sectorSize)
		last = cwa_data(lastBytes)
		
		# Update file metadata
		file['size'] = fileSize
		file['sectorSize'] = sectorSize
		file['headerSize'] = headerSize
		if fileSize >= headerSize:
			file['numSectors'] = (fileSize - headerSize) // 512
		else:
			file['numSectors'] = 0
		
		# Samples per sector
		samplesPerSector = 0
		if 'samplesPerSector' in first:
			samplesPerSector = first['samplesPerSector']
		if 'samplesPerSector' in last:
			samplesPerSector = last['samplesPerSector']
		file['samplesPerSector'] = samplesPerSector
		
		# Estimate total number of samples
		file['numSamples'] = file['numSectors'] * samplesPerSector
		
		duration = 0
		if 'timestamp' in first and 'timestamp' in last:
			duration = last['timestamp'] - first['timestamp']
		file['duration'] = duration
		
		# Mean rate (assuming no breaks)
		meanRate = 0
		if duration != 0:
			meanRate = file['numSamples'] / duration
		file['meanRate'] = meanRate
	
	# Parse metadata
	info = {}
	info['file'] = file
	info['header'] = header
	info['first'] = first
	info['last'] = last
	
	# Metadata dictionary
	return info


# Test function
if __name__ == "__main__":
	import json
	import os
	mode = 'json' # '-mode:json', '-mode:ldjson', '-mode:size_rate'
	for filename in sys.argv[1:]:
		try:
			if filename[0:6] == '-mode:':
				mode = filename[6:]
				continue
			info = cwa_info(filename)
			if mode == 'json':
				print(json.dumps(info, indent=4, sort_keys=True))
			elif mode == 'ldjson':
				print(json.dumps(info))
			elif mode == 'size_rate':
				print('%s,%s,%s,%s,%s' % (info['file']['name'],info['file']['size']/1024/1024,info['file']['duration']/60/60/24,info['header']['sampleRate'],info['file']['meanRate']))
			else:
				print('ERROR: Unknown output mode: %s' % mode)
		except Exception as e:
			#print('Exception ' + e.__doc__ + ' -- ' + str(e))
			raise

