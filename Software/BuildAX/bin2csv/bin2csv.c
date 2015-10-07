/* 
 * Copyright (c) 2013-2014, Newcastle University, UK.
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

/**
 * BAX File format converter
 * 2014 SF: Initial implementation by Sam Finnigan, Newcastle University
 * 2015 JH: (August) command line updates by Jon Hand, ESRU to -l list sensors
 *      and frequency as well as -n XXXXXXXX to focus on a single sensor as 
 *      well as for single sensors -f dbt or -f rh or -f lux or -f pirc or -f 
 *      sw or -f bat
 * 2015 JH: (September) Command line updates by Jon Hand, ESRU to allow -f 
 *      filter for all sensors and add a -N option for output as a tuple (sql 
 *      time,data).
 * 2015 SF: Fix drag+drop overwrite bug and integrate Newcastle and Strathclyde
 *      code into single file
 *
 * Read a .bax data file containing 32-byte packets of data gathered from
 * the sensors, and write it out as formatted CSV to file or terminal.
 * 
 * Bax Data packet format (32 bytes):
 *  
 *  |--- 0 ---|--- 1 ---|--- 2 ---|--- 3 ---|--- 4 ---|--- 5 ---|--- 6 ---|--- 7 ---|
 *  |-------------------------------------------------------------------------------|
 *  |               dataNumber              |               DateTime                |
 *  |           (sequence number)           |           (Packet RX time)            |
 *  |-------------------------------------------------------------------------------|
 *  | Contin- |                Address                |  RSSI   |  pkt    |  pkt    |
 *  |  uation |            (of Sensor node)           |         |  Type   |  ID     |
 *  |-------------------------------------------------------------------------------|
 *  |xMit pwr |   Battery Level   |HumiditySaturation | Temperature deg C |  Light  >
 *  |  (dBm)  |   (millivolts)    |   (MSB and LSB)   |       (x10)       |  (lux)  >
 *  |-------------------------------------------------------------------------------|
 *  >  Light  |       PIR         |        PIR        |    Switch Count   |  null   |
 *  >  (lux)  |      Counts       |       Energy      |        Stat       |         |
 *  |-------------------------------------------------------------------------------|
 *
 * CSV Columns:
 *  Date, Time, Sensor Address, RSSI, Packet Type, Packet ID, Transmit Power (dBm), 
 *  Battery Level (mv), Humidity, Temperature (deg C x 10), Light (lux), PIR activation 
 *  count, PIR energy, Switch activation count
 *
 */

#ifdef _WIN32
	#define _CRT_SECURE_NO_WARNINGS
    #include <windows.h>
	#include <io.h>
	// 8.3 fn + space = 13 chars, total 32767 arg char len in Windows, so 32767/13=2520
	//#define ARG_MAX 2500
	#define ARG_MAX _ARGMAX
	#define strnicmp _strnicmp
#else
	#include <unistd.h>
	#define _access access
	#include <limits.h>
	#define _MAX_FNAME NAME_MAX
	// Should also get ARG_MAX from limits.h
    #define strnicmp strncasecmp
#endif

/* Values for the second argument to access().
   These may be OR'd together.  */
#ifndef __APPLE__           /* Apple's unistd.h defines these */
	#define R_OK    4       /* Test for read permission.      */
	#define W_OK    2       /* Test for write permission.     */
	//#define   X_OK    1       /* execute permission - unsupported in windows */
	#define F_OK    0       /* Test for existence.  */
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "Config.h"
#include "BaxUtils.h"
#include "BaxRx.h"


#define DEFAULT_OUTFILE "bax"
#define DEFAULT_OUT_SUF "csv"
#define SENSOR_NAME_LENGTH 8		  /* Length of a stringified sensor address */
#define MAX_ACTIVE 150				  /* Max total active sensors to handle */

// ---------------------------------------------------------------------------
/* Type and variable definitions */

// Topic/Focus definitions
typedef enum
{
	FOCUS_NONE          = 0,
	FOCUS_SINGLE_SENSOR = 1,
	FOCUS_OUTPUT_TUPLE  = 2
} Filter_t;

typedef enum
{
	TOPIC_NONE        = 0,
	TOPIC_TEMPERATURE = 1,
	TOPIC_HUMIDITY    = 2,
	TOPIC_LIGHT       = 3,
	TOPIC_PIR_COUNTS  = 4,
	TOPIC_SWITCH      = 5,
	TOPIC_BATTERY     = 6
} Topic_t;


// ---------------------------------------------------------------------------
// Local variables for focusing on sensors/topics

static char ilist = 0;                /* 0= ignore, 1= invoke only to list available sensors, number found returned as negative */
static Filter_t focus = FOCUS_NONE;   /* 0= assume no single sensor; 1= focus on a sensor; 2= focus and output SQL tuple */
static Topic_t  topic = TOPIC_NONE;   /* assume all measurements of interest */
static int sensors_found;             /* number of sensors found */

static char* focusname;			      /* Sensor name buffer for the single-sensor focus command line argument. */
static char envname[MAX_ACTIVE][SENSOR_NAME_LENGTH+1];   /* sensor id list */
static int sensor_hits[MAX_ACTIVE];    /* how many hits for each sensor */

// Declare settings and status (extern in Config.h)
Settings_t gSettings;
Status_t gStatus;


// ---------------------------------------------------------------------------
// Function Prototypes
void BaxNewEntry(binUnit_t*);
void BaxCountEntry(binUnit_t*);
void PrintList();
binUnit_t* ParseFileEntryToUnit(unsigned char*, binUnit_t*);


// ---------------------------------------------------------------------------
// Functions

/* Clean up function for gracefully closing program */
void CleanupOnExit(void)
{
	// Cleanup code: do kbhit in debug mode:
#if defined(_WIN32) && defined(_DEBUG) 
	if(_isatty(_fileno(stdin)) && gSettings.outputFile != stdout)	// If stdin/out isn't a pipe...
	{
		printf("\r\nPress Return to exit....");
		getc(stdin);	// Pause
	}
#endif
}

/*
 * Attempt to access files so we don't clobber an existing one on drag/drop
 * The fname buffer must be a minimum of strlen(DEFAULT_OUTFILE + DEFAULT_OUT_SUF)+(index/10) long
 *
 * Filenames are produced in the format "name.n.csv"
 *
 * This function is not thread-safe and is subject to filesystem race conditions.
 *
 * Parameters:
 *		fname	The buffer to store the resultant filename in
 *		infile	The filename to check (minus extension)
 *		index	The index to start at
 */
char* getCheckedFilename(char* fname, char* infile, int index) 
{
	sprintf(fname, "%s.%d.%s", infile, index, DEFAULT_OUT_SUF);

	if( _access( fname, F_OK ) != -1 ) {
		// file exists, look again in recursion:
		getCheckedFilename(fname, infile, index+=1);
	}

	// file doesn't exist
	return fname;
}


/**
 * Function:
 *		BaxDataRead(void)
 *
 * Summary:
 *		Perform read file, 32b at a time.
 **/
void BaxDataRead(void)
{
	unsigned long read = 0;
	unsigned char data[BINARY_DATA_UNIT_SIZE];	// Read 32b
	binUnit_t unit;

	do // Read file loop
	{
		read = fread(data, 1, BINARY_DATA_UNIT_SIZE, gSettings.inputFile);

		if(read == BINARY_DATA_UNIT_SIZE)			// If we have successfully read 32 bytes
		{
			ParseFileEntryToUnit(data, &unit);		// Parse it into a binUnit_t struct

			// List? Or decode whole packet?
			if (ilist)
				BaxCountEntry(&unit);		// List only
			else
				BaxNewEntry(&unit);			// Parse whole unit into the destination stream
		}
	}
	while(read);	// while not EOF or ^C

	if (ilist)
		PrintList();	// List what was found (if in listing mode)
}



/** 
 * Function:
 *		ParseFileEntryToUnit(unsigned char*, binUnit_t*)
 * Summary:
 *		Parse out an entry from the binary data file, populating
 *		the passed-in binUnit_t and returning its pointer.
 *
 * Description:
 *	 	This function will copy members of the binUnit_t struct individually:
 *	 	The reason for this is that neither the struct packing (despite using
 *		the __packed__ attribute) nor the width of the datatypes is guaranteed 
 *		to be exactly the same on any given platform. 
 **/
binUnit_t* ParseFileEntryToUnit(unsigned char* received, binUnit_t* unit)
{
	// For debugging, when viewing memory directly:
	//memset(unit, 0x00, sizeof(binUnit_t));
	
	unit->dataNumber = (unsigned short) received[0];
	memcpy(&unit->dataTime, &received[4], sizeof(DateTime));

	unit->continuation = (unsigned char) received[8];
	memcpy(&unit->data, &received[9], BINARY_DATA_SIZE);

	return unit;
}



/**
 * Function:
 *		BaxNewEntry(binUnit_t* unit)
 *
 * Summary:
 *		Write decoded line to output. Decoded packets are written
 *		to the destination configured in the gSettings struct.
 **/
void BaxNewEntry(binUnit_t* unit)
{
	// The new decoded packets are output by this function.
	//BaxPacket_t decoded;	// Split out so we don't need to stack as much memory
	unsigned char address[4];
	unsigned char rssi;
	signed char pktType;
	char cname[SENSOR_NAME_LENGTH+1];  /* current sensor being decoded (plus null) */

	unsigned short *ptr;
	BaxSensorPacket_t pkt;	// Unpack bytes into a data packet

	// Copy everything by hand because we can't rely on struct packing.
	memcpy(address, unit->data, 4);

	// TODO: replace this string compare with binary compare.
	// Can cast to a long and check equality with decoded sensor address
	// If focused on a single sensor setup current cname to compare.
	if (focus == FOCUS_SINGLE_SENSOR || focus == FOCUS_OUTPUT_TUPLE)
	{
		memset(cname, '\0', SENSOR_NAME_LENGTH);  // clear current name from cname buffer
		sprintf(cname, "%02X%02X%02X%02X", address[3], address[2], address[1], address[0]);

		if (strncmp(cname, focusname, 8) != 0)
			return;	// not a match: jump out and try reading another
	}

	rssi    = (unsigned char) unit->data[4];
	pktType = (signed char)   unit->data[5];
	
	// Offset of BaxSensorPacket_t data into binUnit_t == 6.
	pkt.pktId       = (unsigned char)  unit->data[6 + 0];
	pkt.xmitPwrdBm  = (signed char)    unit->data[6 + 1];

	// We want the following values as 16 bit shorts (cast the rest of the array)
	ptr = (unsigned short *) &unit->data[8];

	// Copy the rest of the values into the BaxSensorPacket_t struct:
	pkt.battmv      = ptr[0];
	pkt.humidSat    = ptr[1];
	pkt.tempCx10    = (signed short) ptr[2];
	pkt.lightLux    = ptr[3];
	pkt.pirCounts   = ptr[4];
	pkt.pirEnergy   = ptr[5];
	pkt.swCountStat = ptr[6];

	
	// Print data to file or STDOUT
	// Note: because we opened the file in non-binary write mode,
	//  LF (\n) is automatically converted to CRLF (\n) on Windows
	// Now check the packet type is decoded (i.e. we want to print it out):
	if( (pktType <= 3 && pktType > 0) || gSettings.printEncryptedPackets ) // decoded ENV packets
	{
		// Print time (in correct format)
		fprintf(gSettings.outputFile, "%s,",
			((focus == FOCUS_OUTPUT_TUPLE) ? RtcToStringSql(unit->dataTime) : RtcToString(unit->dataTime)));

		// Print address:
		if (focus != FOCUS_OUTPUT_TUPLE)	// exclude if command line included -N
		{
			fprintf(gSettings.outputFile, "%02X%02X%02X%02X", address[3], address[2], address[1], address[0]);
		}

		// Print RSSI and pktType if not focusing on a single data type
		if (topic == TOPIC_NONE)
		{
			fprintf(gSettings.outputFile, ",%d,%d,", (signed short)RssiTodBm(rssi), (signed short)pktType);
		}

		// If implementing decoding packets using a BAX_INFO.BIN file, they
		// would be decoded here. 
		
		// Print out encrypted packets (if this case was hit with gSettings.printEncryptedPackets)
		if( pktType > RAW_PKT_LIMIT || pktType <= 0 || pktType == BAX_NAME_PKT )
		{
			fprintf(gSettings.outputFile, "\tencrypted packet\n");
			return;
		}

		// else print decoded packet
		switch (topic)
		{
		case TOPIC_NONE:
			// else print decoded packet
			fprintf(gSettings.outputFile, "%u,%u,%u,%u.%02u,",
				pkt.pktId, 			// pktId
				pkt.xmitPwrdBm, 	// txPwr dbm
				pkt.battmv, 		// battmv
				pkt.humidSat >> 8, 	// humidSat MSB
				(((signed short)39 * (pkt.humidSat & 0xff)) / 100));// humidSat LSB
			fprintf(gSettings.outputFile, "%d,%u,%u,%u,%u\n",
				pkt.tempCx10,		// tempCx10
				pkt.lightLux, 		// lightLux
				pkt.pirCounts,		// pirCounts
				pkt.pirEnergy,		// pirEnergy
				pkt.swCountStat); 	// swCountStat
			break;

		case TOPIC_TEMPERATURE:
			// exclude comma ',' if command line included -N
			fprintf(gSettings.outputFile,
				((focus == FOCUS_OUTPUT_TUPLE) ? "%d\n" : ",%d\n"),
				pkt.tempCx10);
			break;

		case TOPIC_HUMIDITY:
			fprintf(gSettings.outputFile,
				((focus == FOCUS_OUTPUT_TUPLE) ? "%u.%02u\n" : ",%u.%02u\n"),
				pkt.humidSat >> 8, 	// humidSat MSB
				(((signed short)39 * (pkt.humidSat & 0xff)) / 100)); // humidSat LSB
			break;

		case TOPIC_LIGHT:
			fprintf(gSettings.outputFile,
				((focus == FOCUS_OUTPUT_TUPLE) ? "%u\n" : ",%u\n"),
				pkt.lightLux);
			break;

		case TOPIC_PIR_COUNTS:
			fprintf(gSettings.outputFile,
				((focus == FOCUS_OUTPUT_TUPLE) ? "%u\n" : ",%u\n"),
				pkt.pirCounts);
			break;

		case TOPIC_SWITCH:
			fprintf(gSettings.outputFile,
				((focus == FOCUS_OUTPUT_TUPLE) ? "%u\n" : ",%u\n"),
				pkt.swCountStat);
			break;

		case TOPIC_BATTERY:
			fprintf(gSettings.outputFile,
				((focus == FOCUS_OUTPUT_TUPLE) ? "%u\n" : ",%u\n"),
				pkt.battmv);
			break;
		}
	}	
}


/**
* Function:
*		BaxCountEntry(binUnit_t* unit)
*
* Summary:
*		Gather statistics for list function.
**/
void BaxCountEntry(binUnit_t* unit)
{
	unsigned char address[4];
	char cname[SENSOR_NAME_LENGTH + 1];
	signed char pktType;

	int i = 0;

	// Copy everything by hand because we can't rely on struct packing.
	memcpy(address, unit->data, 4);
	pktType = (signed char)   unit->data[5];

	// Now check the packet type is decoded (i.e. we want to gather info):
	if (pktType <= 3 && pktType > 0) // decoded ENV packets only
	{
		memset(cname, '\0', SENSOR_NAME_LENGTH+1);  // clear current name and write into name buffer
		sprintf(cname, "%02X%02X%02X%02X", address[3], address[2], address[1], address[0]);

		for(i=0; i<MAX_ACTIVE; i++)
		{
			if (envname[i][0] == '\0')
			{
				// Found a new one.
				sensors_found++; // (sensors_found will be == i)
				strncpy(envname[i], cname, SENSOR_NAME_LENGTH);
				envname[i][SENSOR_NAME_LENGTH] = '\0'; /* Null-terminate */
				sensor_hits[i]++;
				break;
			}
			else if (strncmp(cname, envname[i], SENSOR_NAME_LENGTH) == 0)
			{
				// Matched an existing one
				sensor_hits[i]++;
				break;
			}
			// else look in next position
		}

		if (i == MAX_ACTIVE)
		{
			if (sensors_found <= MAX_ACTIVE)
			{
				fprintf(stderr, "More sensors found than this program can handle.\nAdditional sensors will be dropped.\n");
				sensors_found++;
			}
		}
	}
}


/**
* Function:
*		PrintList(void)
*
* Summary:
*		Print the list of found sensors
*		Make use of substring name as buffer for the i-th portion of the
*		envname pointer to array of string pointers.
**/
void PrintList(void)
{
	unsigned char i; // i < MAX_ACTIVE which is defined to 70 by default
	// debug  fprintf(stderr, "  %s \n",envname);
	fprintf(stderr, " Sensors found %d\n", sensors_found);
	fprintf(stderr, " Name   Number of packets\n");

	for (i = 0; i<sensors_found; i++)
	{
		fprintf(stderr, " %s %d\n", envname[i], sensor_hits[i]);
	}
}



/**
 * Function:
 *		PrintHelp(void)
 *
 * Summary:
 *		Print the help message (-h or bad option)
 **/
void PrintHelp(void)
{
	fprintf(stderr, "BAX bin2csv converter (2014 Newcastle University)\n"
		"Usage: bin2csv [-hesloOnfN] [File List]\n"
		"\n"
		"Options:\n"
		"\t-h      Display this message\n"
		"\t-e      Print encrypted and un-decodable packets\n"
		"\t-s      Stream data from stdin (file list is ignored)\n"
		"\t-l      List EVN sensors and their frequency and then exit\n"
		"\t-o      Write directly to stdout\n"
		"\t-O file Specify a single output file to write decoded input\n"
		"\t        into. Ensure you add a space. File will not be \n"
		"\t        clobbered if it already exists.\n"
		"\t-n name Extract a specific ENV sensor name. May also\n"
		"\t        filter for a single measurement via:\n"
		"\t-f DBT -f RH -f LUX -f PIRC -f SW or -f BAT\n"
		"\t        (being Temperature, Relative Humidity, Light, PIR\n"
		"\t        Counts, Switch Counts, and Battery Level)\n"
		"\t-N name Extract a specific ENV sensor single measurement\n"
		"\t        as a tuple with SQL timings\n"
		"\n"
		"Example usages: \n"
		"\n"
		"Decode files into .csv individual files (e.g. default \n"
		"drag-drop behaviour):\n"
		"\tbin2csv DAT00001.BIN DAT00002.BIN [...] DAT0000n.BIN \n"
		"\n"
		"List the sensors and their frequency found in fetch.bax: \n"
		"\tbin2csv -l fetch.bax\n"
		"\n"
		"Convert and extract humidity data in fetch.bax into datarh.csv: \n"
		"\tbin2csv -f RH fetch.bax -O datarh.csv\n"
		"\n"
		"Filter for the sensor named 42D89A75: \n"
		"\tbin2csv fetch.bax -O 42D89A75.csv -n 42D89A75\n"
		"\n"
		"Filter for relative humidity from the sensor named 42D89A75: \n"
		"\tbin2csv fetch.bax -O 42D89A75.csv -n 42D89A75 -f RH\n"
		"\n"
		"Example as above but with output as a tuple with SQL timings: \n"
		"\tbin2csv fetch.bax -O 42D89A75.csv -N 42D89A75 -f RH\n"
		"\n"
		"Decoded data is written to a file with a .csv extension, e.g.\n"
		"\"out.n.csv\"\n" );
}



/**
 * Function:
 *		main(int argc, char *argv[])
 *
 * Summary:
 *		Parse arguments and open files for reading/writing
 * 
 * Command line args:
 *		[-hs] [somepath.bax] [outfile.csv]
 **/
int main(int argc, char *argv[])
{
	unsigned char i;
	unsigned char singleOutput = 0;	 // Flag- single output file instead of one per file.
	char fname[_MAX_FNAME];		     // buffer for output file name

	gSettings.inputFile = NULL;
	gSettings.outputFile = NULL;

	// Check arg lengths
	if (argc < 2 || argc > ARG_MAX)
	{
		fprintf(stderr, "Too many or too few arguments. Exiting...\n\n");
		PrintHelp();
		return 1;
	}
	
	// Set cleanup funtion
	atexit(CleanupOnExit);

	// -----------------------------------------------------------------------
	// Parse dash args 
	// (eg: -so for streaming from stdin to stdout, etc). 
	for (i = 1; argc > i; i++)	// Args to process? (Arg 0 is the executable, skip it)
	{
		if (argv[i][0] == '-')	
		{
			unsigned short offset = 1;
			unsigned short len = strlen(argv[i]);

			while (offset < len)	// Iterate over this arg
			{
				if (argv[i][offset] == 'h')		// Help flag -h
				{
					PrintHelp();
					return 0;
				}

				// To parse more -abc arguments in future, add them here.
				//   When a valid arg is detected, increment offset++ and put a
				//   continue statement to jump back up to the while() condition. 
				//   This ensures all args in the found block (e.g. -aeiou) will 
				//   be parsed regardless of order.
				if (argv[i][offset] == 'e')		// Print encrypted packets
				{
					gSettings.printEncryptedPackets = TRUE;
					offset++;
					continue;
				}

				if (argv[i][offset] == 's')		// Stream from stdin
				{
					gSettings.inputFile = stdin;
					offset++;
					continue;
				}

				if (argv[i][offset] == 'o')		// Stream to stdout
				{
					gSettings.outputFile = stdout;
					offset++;
					continue;
				}

				if (argv[i][offset] == 'l')		// List sensors and exit
				{
					ilist = 1; 			// Set ilist for use later
					offset++;
					fprintf(stderr, "Listing of ENV sensors follows:\n");

					// initial clear of sensor list
					sensors_found = 0;
					memset(sensor_hits, 0, sizeof(sensor_hits));
					memset(envname,   0, sizeof(envname[0][0] * MAX_ACTIVE * SENSOR_NAME_LENGTH+1));  // clear name

					continue;
				}

				if (argv[i][offset] == 'O')		// Explicitly set output file (for use with stdin)
				{
					char* filename = argv[++i];

					if (_access(filename, F_OK) != -1) 
					{
						fprintf(stderr, "File %s specified with -O already exists! Will not clobber. Exiting.\n", filename);
						return 1;
					}

					strcpy(fname, filename);

					gSettings.outputFile = fopen(fname, "w");	// Next arg, output file
					if (!gSettings.outputFile)
					{
						fprintf(stderr, "Could not open file %s for writing!\n", fname);
						return 1;
					}

					// Indicate that we don't want to open any FHs over the top of this one
					singleOutput = 1;

					break; // Don't evaluate the rest of this arg- it's a filename!
				}

				if (argv[i][offset] == 'n')		// named sensor
				{
					offset++;
					focusname = argv[++i];
					focus = FOCUS_SINGLE_SENSOR;// signal output to focus on name
					fprintf(stderr, "Focus on the sensor: %s\n", focusname);
					continue;
				}
				else if (argv[i][offset] == 'N')		// named sensor with output as tuple
				{
					offset++;
					focusname = argv[++i];
					focus = FOCUS_OUTPUT_TUPLE;	// signal output to focus on name with SQL timing format
					fprintf(stderr, "Focus on the sensor with tuple output: %s\n", focusname);
					continue;
				}

				if (argv[i][offset] == 'f')		// focus on
				{
					char* topicname = argv[++i];	// if focusing on single measurement.

					if (strnicmp(topicname, "DBT", 3) == 0)
					{
						topic = TOPIC_TEMPERATURE;
					}
					else if (strnicmp(topicname, "RH", 2) == 0)
					{
						topic = TOPIC_HUMIDITY;
					}
					else if (strnicmp(topicname, "LUX", 3) == 0)
					{
						topic = TOPIC_LIGHT;
					}
					else if (strnicmp(topicname, "PIRC", 4) == 0)
					{
						topic = TOPIC_PIR_COUNTS;
					}
					else if (strnicmp(topicname, "SW", 2) == 0)
					{
						topic = TOPIC_SWITCH;
					}
					else if (strnicmp(topicname, "BAT", 3) == 0)
					{
						topic = TOPIC_BATTERY;
					}
					else
					{
						fprintf(stderr, "Unknown topic: %s\n", argv[i]);
					}

					fprintf(stderr, "Filter on topic: %s\n", topicname);

					break;
				}

				// else
				fprintf(stderr, "Unknown option in %s\n", argv[i]);
				PrintHelp();
				return 1;
			}
		}
		
		// File name provided directly? Look at it in the next loop.
	}

	// -----------------------------------------------------------------------
	// Read input stream:
	if (gSettings.inputFile == stdin)
	{
		// From stdin
		if (gSettings.outputFile == NULL)	// && gSettings.outputFile != stdout
		{
			// Use default outfile if not set.
			getCheckedFilename(fname, DEFAULT_OUTFILE, 0);
			gSettings.outputFile = fopen(fname, "w");
			if (!gSettings.outputFile)
			{
				fprintf(stderr, "Could not open file %s for writing!\n", fname);
				return 1;
			}
		}

		// Read binary data from stdin
		BaxDataRead();

		if (gSettings.outputFile != stdout)		fclose(gSettings.outputFile);
	}
	else
	{
		// From file list
		// Iterate args again + open files for reading and writing
		for (i = 1; argc > i; i++)	// Args to process? (Arg 0 is the executable, skip it)
		{
			// Ignore dash args
			if (argv[i][0] == '-')
			{
				// Ignore parameters after -O or -f (e.g. RH or ouputfile.csv)
				unsigned short offset = 0;
				unsigned short len = strlen(argv[i]);

				while (++offset < len)	// Iterate over this arg looking for O,f,n,N (we need to skip the next arg if we find it)
					if (argv[i][offset] == 'O' || argv[i][offset] == 'f' || 
						argv[i][offset] == 'n' || argv[i][offset] == 'N')
						i++;
				continue;
			}

			// Sanity check- if we enter here, we should not have had input/output defined. Interpret as input file (unless seen -s switch already)
			if (gSettings.inputFile != NULL)
			{
				fprintf(stderr, "Error processing file %s\n", argv[i]);
				return 1;
			}

			gSettings.inputFile = fopen(argv[i], "rb");	// Read binary mode data (will fail out early with just r)
			if (!gSettings.inputFile)	// Did it open OK?
			{
				fprintf(stderr, "Could not open file %s for reading!\n", argv[i]);
				return 1;
			}

			if (gSettings.outputFile == NULL)
			{
				getCheckedFilename(fname, argv[i], 0);
				gSettings.outputFile = fopen(fname, "w");	// Next arg, output file
				if (!gSettings.outputFile)
				{
					fprintf(stderr, "Could not open file %s for writing!\n", argv[i]);
					return 1;
				}
			}

			// OK, we have valid args now so go ahead and process data:
			BaxDataRead();

			// Close files (if they aren't std streams)
			fclose(gSettings.inputFile);
			gSettings.inputFile = NULL;

			if (gSettings.outputFile != stdout && !singleOutput )
			{
				fclose(gSettings.outputFile);
				gSettings.outputFile = NULL;
			}
		}

		if (singleOutput) fclose(gSettings.outputFile);
	}
	
	// Done. Exit.
	return 0;
}

//EOF
