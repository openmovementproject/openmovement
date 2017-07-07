def cwa_metadata(filename):
	"""CWA Metadata Reader by Dan Jackson, 2017."""
	
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
	with open(filename, "rb") as f:
		f.seek(64)
		rawbytes = f.read(448)
	
	# Remove any trailing spaces, null, or 0xFF bytes
	encString = str(rawbytes, 'ascii').rstrip('\x20\xff\x00')
	
	# Name-value pairs separated with ampersand
	nameValues = encString.split('&')
	
	# Each name-value pair separated with an equals
	for nameValue in nameValues:
		parts = nameValue.split('=')
		# Name is URL-encoded UTF-8
		name = urldecode(parts[0])
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


# Test function
if __name__ == "__main__":
	import os
	import sys
	for filename in sys.argv[1:]:
		try:
			metadata = cwa_metadata(filename)
			result = {}
			result['filename'] = os.path.basename(filename)
			result.update(metadata)
			print(repr(result))
		except Exception as e:
			print('Exception ' + e.__doc__ + ' -- ' + e.message)
