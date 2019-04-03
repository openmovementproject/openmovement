/* 
 * Copyright (c) 2009-2013, Newcastle University, UK.
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

// Stream Parser
// Dan Jackson, 2012-2013

#ifndef STREAMPARSER_H
#define STREAMPARSER_H

#include "FileStream.h"


/** 
 * Macro to create a packed date/time value from components.
 * @hideinitializer
 */
#define DATETIME_FROM_YMDHMS(year, month, day, hours, minutes, seconds) \
                ( (((unsigned long)((year) % 100) & 0x3f) << 26) \
                | (((unsigned long)(month)        & 0x0f) << 22) \
                | (((unsigned long)(day)          & 0x1f) << 17) \
                | (((unsigned long)(hours)        & 0x1f) << 12) \
                | (((unsigned long)(minutes)      & 0x3f) <<  6) \
                | (((unsigned long)(seconds)      & 0x3f)      ) \
                )
#define DATETIME_YEAR(dateTime)    ((unsigned int)((unsigned char)(((dateTime) >> 26) & 0x3f)) + 2000) /**< Extract the year from a packed date/time value. @hideinitializer */
#define DATETIME_MONTH(dateTime)   ((unsigned char)(((dateTime) >> 22) & 0x0f))  /**< Extract the month (1-12) from a packed date/time value.   @hideinitializer */
#define DATETIME_DAY(dateTime)     ((unsigned char)(((dateTime) >> 17) & 0x1f))  /**< Extract the day (1-31) from a packed date/time value.     @hideinitializer */
#define DATETIME_HOURS(dateTime)   ((unsigned char)(((dateTime) >> 12) & 0x1f))  /**< Extract the hours (0-23) from a packed date/time value.   @hideinitializer */
#define DATETIME_MINUTES(dateTime) ((unsigned char)(((dateTime) >>  6) & 0x3f))  /**< Extract the minutes (0-59) from a packed date/time value. @hideinitializer */
#define DATETIME_SECONDS(dateTime) ((unsigned char)(((dateTime)      ) & 0x3f))  /**< Extract the seconds (0-59) from a packed date/time value. @hideinitializer */


/** Maximum stream identifier */
#define MAX_STREAMS 128


/** Reader Handle */
typedef void *ReaderHandle;


/**
 * Opens a binary data file for reading.
 * Parses the file header and places the stream at the first block of data.
 * @param binaryFilename The file name of the binary file to open.
 * @see ReaderNextBlock(), ReaderClose()
 * @return If successful, a handle to the reader object, otherwise \a NULL
 */
ReaderHandle ReaderOpen(const char *binaryFilename);


/**
 * Return the current block index of the reader.
 * @param reader The handle to the reader.
 * @return If non-negative, the block position within the file, an error code otherwise.
 */
int ReaderDataBlockPosition(ReaderHandle reader);


/**
 * Seeks the file reader to the specified data block.
 * @param reader The handle to the reader.
 * @param seekBlock If positive, the data block index fr the start of the file (after any header blocks); if negative, the data block index frthe end of the file.
 * @return \a 0 if successful, an error code otherwise.
 */
int ReaderDataBlockSeek(ReaderHandle reader, int dataBlockNumber);


/**
 * Reads the next block of data fr the binary file.
 * @param reader The handle to the reader.
 * @retval >0 a positive number of samples in the buffer successfully read. 
 * @retval 0 this block is unreadable or not of the required stream, but additional blocks remain (call ReaderNextBlock() again).
 * @retval -1, the end-of-file has been reached.
 * @retval Otherwise, an error code.
 * @see ReaderBufferX(), ReaderTimestamp()
 */
int ReaderNextBlock(ReaderHandle reader);


/**
 * Gets information on the data type.
 * Currently: the number of channels.
 * @param reader The handle to the reader.
 * @return 0 if successful
 * @return Otherwise, an error code.
 * @see ReaderTimestamp()
 */
int ReaderDataType(ReaderHandle reader, unsigned char *packetType, unsigned char *stream, unsigned char *channels);
int ReaderChannelPacking(ReaderHandle reader, unsigned char *stream, unsigned char *dataSize);


/**
 * Obtains a pointer to the buffer of unpacked samples read by ReaderNextBlock().
 * The number of samples available is specified by the return value of the previous ReaderNextBlock() call.
 * The buffer consists of consecutive signed 16-bit numbers representing each channel in turn. 
 * @note This call will not work for 32-bit data. The buffer contents are only guaranteed until the next 'ReaderBuffer*' call.
 * @param reader The handle to the reader.
 * @return A pointer to the buffer of samples just read.
 * @see ReaderTimestamp()
 */
short *ReaderBufferRawShorts(ReaderHandle reader);


/**
 * Obtains a pointer to the buffer of unpacked samples read by ReaderNextBlock().
 * The number of samples available is specified by the return value of the previous ReaderNextBlock() call.
 * The buffer consists of consecutive signed 32-bit numbers representing each channel in turn. 
 * @note The buffer contents are only guaranteed until the next 'ReaderBuffer*' call.
 * @param reader The handle to the reader.
 * @return A pointer to the buffer of samples just read.
 * @see ReaderTimestamp()
 */
int *ReaderBufferRawInts(ReaderHandle reader);


/**
 * Obtains a pointer to the buffer of processed, unpacked samples from ReaderNextBlock().
 * The number of samples available is specified by the return value of the previous ReaderNextBlock() call.
 * The buffer consists of consecutive floating point numbers representing each channel in turn. 
 * @note The buffer contents are only guaranteed until the next 'ReaderBuffer*' call.
 * @param reader The handle to the reader.
 * @return A pointer to the buffer of samples just read.
 * @see ReaderTimestamp()
 */
float *ReaderBufferFloats(ReaderHandle reader);


/**
 * Determines the timestamp of the specified sample in the buffer read by ReaderNextBlock().
 *
 * Actual readings from the real-time-clock are stored once per block (with precise offset information).
 * As the true sampling rate is subject to error (it is subject to the accelerometer device's internal sampling)
 *   this function must interpolate over these timestamps to produce the actual real-time value for each sample.
 *
 * @param reader The handle to the reader.
 * @param index The sample index, must be >= 0 and < the number of samples returned from ReaderNextBlock().
 * @return The time in seconds since the epoch (1970-01-01 00:00:00).
 */
double ReaderTimeSerial(ReaderHandle reader, int index);


/**
 * Determines the timestamp of the specified sample in the buffer read by ReaderNextBlock().
 *
 * Actual readings from the real-time-clock are stored once per block (with precise offset information).
 * As the true sampling rate is subject to error (it is subject to the accelerometer device's internal sampling)
 *   this function must interpolate over these timestamps to produce the actual real-time value for each sample.
 *
 * @param reader The handle to the reader.
 * @param index The sample index, must be >= 0 and < the number of samples returned from ReaderNextBlock().
 * @param[out] fractional A pointer to a value to hold the 1/65536th of a second fractional time offset, or \a NULL if not required.
 * @return The packed date/time value of the sample at the start of the buffer, or 0 if none (e.g. an invalid index).
 */
unsigned long ReaderTimestamp(ReaderHandle reader, int index, unsigned short *fractional);


/**
 * Accesses the contents of a raw data packet. 
 * This would not typically be used by an API client.
 * The packet is loaded by calls to ReaderNextBlock(), and remains valid until the next call of ReaderNextBlock() or ReaderClose() for the same reader handle.
 * \note If ReaderNextBlock() returns 0, the reader has detected a checksum failure in the buffers contents, and any data values should be treated with caution.
 * @param reader The handle to the reader.
 * @return A pointer to the bytes in the raw data packet, or \a NULL if none is available.
 */
void *ReaderRawDataPacket(ReaderHandle reader);


/**
 * Closes the specified reader handle.
 * Frees any resources allocated to the reader.
 * @param reader The handle to the reader to close.
 * @see ReaderOpen()
 */
void ReaderClose(ReaderHandle reader);


const filestream_fileheader_t *ReaderMetadata(ReaderHandle reader, unsigned long long *firstTimestamp, unsigned long long *lastTimestamp);



#endif
