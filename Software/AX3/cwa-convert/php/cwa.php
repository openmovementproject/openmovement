<?php

class CWA
{
	static function fread_dword($fp)
	{
		if ($fp == null || feof($fp)) { return 0; }
		$buffer = fread($fp, 4);
		if (strlen($buffer) != 4) { return 0; }
		$value = ord($buffer{0}) | (ord($buffer{1})<<8) | (ord($buffer{2})<<16) | (ord($buffer{3})<<24);
		return $value;
	}

	static function fread_word($fp)
	{
		if ($fp == null || feof($fp)) { return 0; }
		$buffer = fread($fp, 2);
		if (strlen($buffer) != 2) { return 0; }
		$value = ord($buffer{0}) | (ord($buffer{1})<<8);
		return $value;
	}

	static function fread_byte($fp)
	{
		if ($fp == null || feof($fp)) { return 0; }
		$buffer = fread($fp, 1);
		if (strlen($buffer) != 1) { return 0; }
		$value = ord($buffer{0});
		return $value;
	}

	static function timestamp($timestamp)
	{
		// 'DateTime' bit pattern:  YYYYYYMM MMDDDDDh hhhhmmmm mmssssss
		if ($timestamp == 0) { return ""; }
		else if ($timestamp == -1) { return ""; }	// 0xffffffff
		$year  = strval((($timestamp >> 26) & 0x3f)) + 2000; $year = str_pad($year, 4, "0", STR_PAD_LEFT);
		$month = strval((($timestamp >> 22) & 0x0f)); $month = str_pad($month, 2, "0", STR_PAD_LEFT);
		$day   = strval((($timestamp >> 17) & 0x1f)); $day   = str_pad($day,   2, "0", STR_PAD_LEFT);
		$hours = strval((($timestamp >> 12) & 0x1f)); $hours = str_pad($hours, 2, "0", STR_PAD_LEFT);
		$mins  = strval((($timestamp >>  6) & 0x3f)); $mins  = str_pad($mins,  2, "0", STR_PAD_LEFT);
		$secs  = strval((($timestamp >>  0) & 0x3f)); $secs  = str_pad($secs,  2, "0", STR_PAD_LEFT);
		return "$year-$month-$day $hours:$mins:$secs";  // "Y-m-d H:i:s"
	}

	
    function CWA($filename)
	{
		$this->error = null;
		$fp = fopen($filename, "rb");
		if ($fp != null)
		{
			// Header
			$header = CWA::fread_word($fp);					// [0] 0x444D = ("MD") Meta data block
			$blockSize = CWA::fread_word($fp);				// [2] 0xFFFC = Packet size (2^16 - 2 - 2)

			// Stored data
			$performClear = CWA::fread_byte($fp);			// [4]
			$deviceId = CWA::fread_word($fp);				// [5]
			$sessionId = CWA::fread_dword($fp);				// [7]
			$shippingMinLightLevel = CWA::fread_word($fp);	// [11]
			$loggingStartTime = CWA::fread_dword($fp);		// [13]
			$loggingEndTime = CWA::fread_dword($fp);			// [17]
			$loggingCapacity = CWA::fread_dword($fp);		// [21]
			$allowStandby = CWA::fread_byte($fp);			// [25]
			$debuggingInfo = CWA::fread_byte($fp);			// [26]
			$batteryMinimumToLog = CWA::fread_word($fp);		// [27]
			$batteryWarning = CWA::fread_word($fp);			// [29]
			$enableSerial = CWA::fread_byte($fp);			// [31]
			$lastClearTime = CWA::fread_dword($fp);			// [32]
			$samplingRate = CWA::fread_byte($fp);			// [36]
			$lastChangeTime = CWA::fread_dword($fp);			// [37]
			$firmwareVersion = CWA::fread_byte($fp);			// [41]

			// Reserved block
			$reserved = fread($fp, 22);					// [42] Another 22 bytes reserved before the annotation starts

			// Annotation metadata
			$annotationBlock = fread($fp, 448 + 512);				// [64] 448 bytes of metadata annotations
			if (strlen($annotationBlock) < 448 + 512)
			{
				$annotationBlock = "";
			}

			// Remove any 0xff characters
			$annotation = "";
			for ($i = 0; $i < strlen($annotationBlock); $i++)
			{
			    $c = $annotationBlock{$i};
				if (ord($c) != 0xff && $c != ' ') 
				{ 
					if ($c == '?') { $c = '&'; }	// Merge the URL-encoded strings
					$annotation .= $c;
				}
			}
			$annotationBlock = rtrim($annotation);

			// Parse user annotations
			$labelMap = array(
				// At device set-up time
				'_c' => 'studyCentre',
				'_s' => 'studyCode',
				'_i' => 'investigator',
				'_x' => 'exerciseCode',
				'_v' => 'volunteerNum',
				'_p' => 'bodyLocation',
				'_so' => 'setupOperator',
				'_n' => 'notes',
				// At retrieval time
				'_b' => 'startTime', 
				'_e' => 'endTime', 
				'_ro' => 'recoveryOperator', 
				'_r' => 'retrievalTime', 
				'_co' => 'comments'
			);
			$annotations = array();
			$annotationElements = explode('&', $annotationBlock);
			foreach ($annotationElements as $annotationElement)
			{
				$parts = explode('=', $annotationElement, 2);
				$name = urldecode($parts[0]);
				$value = count($parts) > 1 ? urldecode($parts[1]) : "";
				if (array_key_exists($name, $labelMap)) { $name = $labelMap[$name]; }
				$annotations[$name] = $value;
			}
			
			// TODO: Read first valid data block for start time, find last valid data block for end time
			;
			
			// Close file
			fclose($fp);
			
			// Store some of the data in the object for retrieval
			$this->deviceId = $deviceId;
			$this->lastClearTime = CWA::timestamp($lastClearTime);
			//$this->samplingRate = 0;	// TODO: Convert sampling rate from $samplingRate
			//$this->samplingRange = 0;	// TODO: Convert sampling range from $samplingRate
			$this->lastChangeTime = CWA::timestamp($lastChangeTime);			
			$this->annotations = $annotations;			
			$this->firmwareVersion = ($firmwareVersion == 255 ? 0 : $firmwareVersion);
		}
		else
		{
			$this->error = "Problem opening file " . $filename . "";
			$this->deviceId = null;
			$this->annotations = array();
		}
    }
	
	function getError() { return $this->error; }
}


function CWA_test()
{
	//$filename = "1000-100.CWA";
	//$filename = "SMALL.CWA";
	$filename = "TEST.CWA";

	echo "Opening: " . $filename . "\r\n";
	$cwa = new CWA($filename);
	if ($cwa->getError() != null)
	{
		echo "ERROR: " . $cwa->getError() . "\r\n";
	}
	
	echo "DeviceID: $cwa->deviceId\r\n";
	echo "MetaDataSetTime: $cwa->lastChangeTime\r\n";
	echo "LastClearTime: $cwa->lastClearTime\r\n";
	echo "FirmwareVersion: $cwa->firmwareVersion\r\n";
	echo "Annotations:\r\n";
	foreach ($cwa->annotations as $name => $value)
	{
		echo "* '$name' => '$value'\r\n";
	}
	
	echo "Done.\r\n";
}

//CWA_test();

?>