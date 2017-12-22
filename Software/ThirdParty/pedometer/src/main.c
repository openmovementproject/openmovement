// Test code for pedometer

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS 1
#include <windows.h>
#define strcasecmp _stricmp
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "omapi.h"
#include "StreamParser.h"
#include "Pedometer.h"

pedVars_t pedVars;
FILE* capture = NULL;
unsigned long  gStartTime = 0;
void my_exit(char pause);
extern char *GetFilename(const char *initialFilename);
void InitPedVars(pedVars_t* vars, ped_accel_t* sample, unsigned short rate);

int main(int argc, char * argv[])
{
	const char *captureFile = NULL; // "capture.csv";
	char *outputFilename = NULL;
	// Try open
	float dataRate = 100;
	char isOMX = -1;
	char is8Bit = 0;
	OmReaderHandle inputFile;
	ReaderHandle inputFileOMX;
	const char *filename = NULL;
	char pauseAtEnd = 0;

#ifdef _DEBUG
	pauseAtEnd = 1;
#endif

	// Use command-line filename if specified (e.g. if drag-and-drop to executable)
	if (argc > 1) { filename = argv[1]; }
	if (argc > 2) { fprintf(stderr, "ERROR: Too many arguments.\n"); return -1; }
	if (filename[0] == '-') { fprintf(stderr, "ERROR: Parameters not supported.\n"); return -1; }

	// If not specified, use file open dialog
#ifdef _WIN32
	if (filename == NULL)
	{
		filename = GetFilename(NULL);
		pauseAtEnd = 1;
	}
#endif

	// Error if no file specified
	if (filename == NULL)
	{
		fprintf(stderr, "ERROR: File not specified.\n");
		return -1;
	}

	// Determine file type
	{
		int extPos = -1;
		const char *p;
		for (p = filename; *p != '\0'; p++)
		{
			if (*p == '/' || *p == '\\') { extPos = -1; }
			else if (*p == '.') { extPos = (p - filename); }
		}
		if (extPos >= 0 && !strcasecmp(filename + extPos, ".omx")) { isOMX = 1; }
		else if (extPos >= 0 && !strcasecmp(filename + extPos, ".cwa")) { isOMX = 0; }

		// Output filename
		outputFilename = malloc(strlen(filename) + 32);
		strcpy(outputFilename, filename);
		if (extPos >= 0) { outputFilename[extPos] = '\0'; }
		strcat(outputFilename, ".steps.csv");
	}

	if(isOMX == 0)
	{
		inputFile = OmReaderOpen(filename);
		if(inputFile==NULL){fprintf(stderr, "ERROR: Couldn't open CWA file\n");my_exit(pauseAtEnd);}
	}
	else if (isOMX == 1)
	{
		inputFileOMX = ReaderOpen(filename);
		if(inputFileOMX==NULL){fprintf(stderr, "ERROR: Couldn't open OMX file\n");my_exit(pauseAtEnd);}
	}
	else
	{
		fprintf(stderr, "ERROR: Unknown file type\n");
		my_exit(pauseAtEnd);
	}
	
	// Capture file
	if (captureFile != NULL && captureFile[0] != '\0')
	{
		remove(captureFile);
		capture = fopen(captureFile, "w");
		if (capture == NULL) fprintf(stderr, "WARNING: Capture unsuccessful\n\n");
	}

	// Read data CWA
	if(!isOMX)
	{
		char mystring[32];
		int dataBlockSize; 
		int dataOffsetBlocks;
		int dataNumBlocks;
		OM_DATETIME startTime;
		OM_DATETIME endTime;
		OmReaderDataRange(inputFile, &dataBlockSize, &dataOffsetBlocks, &dataNumBlocks, &startTime, &endTime);
		fprintf(stderr, "NOTE: Using CWA file.\n");
//		fprintf(stderr, "NOTE: Data block size %d\n",dataBlockSize);
//		fprintf(stderr, "NOTE: Data offset blocks %d\n",dataOffsetBlocks);
//		fprintf(stderr, "NOTE: Data num blocks %d\n",dataNumBlocks);
		fprintf(stderr, "NOTE: Start: %s\n",OmDateTimeToString(startTime,mystring));
//		fprintf(stderr, "NOTE: End: %s\n",OmDateTimeToString(endTime,mystring));
		// Try seek
		{
			int timeout = 0;
			while((OmReaderNextBlock(inputFile)<=0)&&(++timeout<1000));
			if(timeout>=1000){fprintf(stderr, "ERROR: Seek failure\n");my_exit(pauseAtEnd);}
		}
	}
	else
	{
		char mystring[32];
		unsigned char dataSize = 0;
		unsigned char dataType = 0;
		unsigned long startTime; 
		unsigned char thisStream = 0;
		unsigned char channels = 0;

		int success = ReaderNextBlock(inputFileOMX);
		if(success<=0)	fprintf(stderr, "WARNING: OMX file read first fail?\n");
		startTime = gStartTime = ReaderTimestamp(inputFileOMX, 0, NULL);
		ReaderDataType(inputFileOMX, &thisStream, &channels);
		fprintf(stderr, "NOTE: Using OMX file.\n");
		fprintf(stderr, "NOTE: Start: %s\n",OmDateTimeToString(startTime,mystring));
		ReaderChannelPacking(inputFileOMX, &thisStream, &dataSize);
		ReaderDataType(inputFileOMX, &thisStream, &dataType);
		ReaderDataRateHz(inputFileOMX, &thisStream, &dataRate);
		if (dataSize == FILESTREAM_PACKING_SINT8) 
		{
			is8Bit = 1;
			fprintf(stderr, "NOTE: 8 bit data detected\n");
		}
		else is8Bit = 0;
	}
	

	fprintf(stderr, "NOTE: Starting...\n");
		
	// Process data
	{
		unsigned char firstSample = TRUE;
		int num_samples;
		int timeout = 0;
		long samples_processed = 0;

		short* data_source = NULL;

		while(1)
		{
			if(!isOMX)
			{
				num_samples = OmReaderNextBlock(inputFile);
				data_source = OmReaderBuffer(inputFile);
			}
			else
			{
				// Read the information
				unsigned char thisStream = 0;
				unsigned char channels = 0;

				num_samples = ReaderNextBlock(inputFileOMX);
				ReaderDataType(inputFileOMX, &thisStream, &channels);
				data_source = ReaderBufferRawShorts(inputFileOMX);
				if (num_samples <=0)
				{
					if(++timeout>100){break;} // Break at end of file
				}
				if (thisStream != 'a')
				{	
					continue; // Skip any other streams with timeout
				}


			}

			if(num_samples < 0) break;

			if (data_source != NULL)
			{
				for(;num_samples>0;num_samples--)
				{
					if(firstSample)
					{
						ped_accel_t sample, *sampPtr = (ped_accel_t*)data_source;
						firstSample = FALSE;
						if (is8Bit)
						{
							sample.x = ((signed char*)data_source)[0];
							sample.y = ((signed char*)data_source)[1];
							sample.z = ((signed char*)data_source)[2];
							sampPtr = &sample;
						}
						if (capture)
							fprintf(capture,"steps,eeSum,state,tick,lastStep,lastInterval,iirAveInterval,"
										"aveInterval,acc,iirAveSvm,aveSvm,iirAveAcc,aveAcc,peak/256,count,HighT,LowT\r");
						InitPedVars(&pedVars, sampPtr, (unsigned short)dataRate);
						PedInit(sampPtr); // Initialise
					}
					// Process
					if (is8Bit)
					{
						ped_accel_t sample;
						sample.x = *(signed char*)data_source++;
						sample.y = *(signed char*)data_source++;
						sample.z = *(signed char*)data_source++;
						PedTasks(&sample, 1);
					}
					else
					{
						PedTasks((ped_accel_t*)data_source, 1);
						data_source += sizeof(ped_accel_t);
					}
					{
						static unsigned short countNow = 0;
						unsigned short counted;
						if(countNow != ped.steps)
						{
							countNow = ped.steps;
							counted = 5;
						}
						else
						{
							counted = 0;
						}
						// Output to csv file
						if (capture)
							fprintf(capture,"%u,%lu, %u,%u,%u,%u,%u,%u, %d,%lu,%u,%lu,%u,%u,%u,%u,%u\r",
								ped.steps,
								ped.eeSum,
								ped.state,
								ped.tick,
								ped.lastStep,
								ped.lastInterval,	
								ped.iirAveInterval,
								ped.aveInterval,
								ped.acc,
								ped.iirAveSvm,
								ped.aveSvm,
								ped.iirAveAcc,
								ped.aveAcc,
								(ped.peakTracker>>8),
								counted,
								ped.highThresh,
								ped.lowThresh);
					}
				}
				samples_processed += num_samples;
			}
			else 
			{
				if(++timeout>1000){fprintf(stderr, "ERROR: No data source.\n");my_exit(pauseAtEnd);}
			}

		}
		// Results
		fprintf(stdout, "ee\t%lu\n", ped.eeSum);
		fprintf(stdout, "steps\t%d\n", ped.steps);

		if (outputFilename != NULL)
		{
			fprintf(stderr, "RESULTS: %s\n", outputFilename);
			FILE *ofp = fopen(outputFilename, "wt");
			if (ofp != NULL)
			{
				fprintf(ofp, "%d\n", ped.steps);
				fclose(ofp);
			}
			else
			{
				fprintf(stderr, "ERROR: Problem writing results: %s\n", outputFilename);
			}
		}
	}

if(isOMX)ReaderClose(inputFileOMX);

my_exit(pauseAtEnd);
}

void my_exit(char pause)
{
	if (capture != NULL) fclose (capture);
	if (pause)
	{
		fprintf(stderr, "--- return to exit ---\n");
		fgetc(stdin);
	}
	return;
}

void InitPedVars(pedVars_t* vars, ped_accel_t* sample, unsigned short rate)
{
	double temp, pow2;
	unsigned long svm = ((long)sample->x*sample->x)+((long)sample->y*sample->y)+((long)sample->z*sample->z);
	svm = (unsigned long)sqrt((double)svm);
	temp = svm;
	temp = log(temp)/log(2.0) - 5; // Make 1g ~ 32
	if(temp < 0)
	{
		fprintf(stderr, "WARNING: Error converting initial sample to range scaler: %u, %u, %u",sample->x,sample->y,sample->z);
		fprintf(stderr, "WARNING: ...SVM of first sample is %lu, using 1g shift of 7\n",svm);
		vars->one_g_shift = 7;
	}
	else
	{
		vars->one_g_shift = (unsigned short)round(temp);
		fprintf(stderr, "NOTE: SVM of first sample is %lu, using 1g shift of %u\n",svm, vars->one_g_shift);
	}

	pedVars.samp_rate = rate;
	temp = rate/50;
	pow2 = log(temp)/log(2.0);

	vars->svm_filter_val		= (unsigned short)round(pow2+8);	
	vars->smooth_lpf_bw			= (unsigned short)round(pow2+4);
	vars->interval_filter_const	= (unsigned short)round(pow2+2);
	vars->threshold_decay_val	= (unsigned short)round(32 / temp);
	vars->activity_threshold	= (unsigned short)(0.125 * 32); // 125mg
	vars->min_step_interval		= (unsigned short)round(temp*10);
	vars->max_step_interval		= (unsigned short)round(temp*100);

	fprintf(stdout, "rate\t%u\nsvm_filter\t%u\nsmooth_lpf\t%u\ninterval_filter\t%u\ndecay_const\t%u\n",rate, vars->svm_filter_val,vars->smooth_lpf_bw,vars->interval_filter_const,vars->threshold_decay_val);
	fprintf(stdout, "activity_thresh\t%u\nmin_step\t%u\nmax_step\t%u\n",vars->activity_threshold,vars->min_step_interval,vars->max_step_interval);

	return;
}
