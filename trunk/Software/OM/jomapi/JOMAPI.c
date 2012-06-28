/* 
 * Copyright (c) 2009-2012, Newcastle University, UK.
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

// Java JNI OMAPI Layer
// Dan Jackson, 2012

#include "JOMAPI.h"
#include "omapi.h"

#ifdef __cplusplus
extern "C" {
#endif 


// Defines
#define SET_OUT_INT(env, array, value) if (array != NULL) { jint outval[1]; outval[0] = value; (*env)->SetIntArrayRegion(env, array, 0, 1, outval); }
#define SET_OUT_LONG(env, array, value) if (array != NULL) { jlong outval[1]; outval[0] = value; (*env)->SetLongArrayRegion(env, array, 0, 1, outval); }


// Local state structure
typedef struct
{
    // VM
    JavaVM *javaVM;
    // Class
    jclass classJomapi;
    // Callbacks
    jmethodID midLogCallback;
    jmethodID midDeviceCallback;
    jmethodID midDownloadCallback;
} jomapi_state;

// Local state
static jomapi_state state = {0};


// JNI_OnLoad
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    state.javaVM = vm;
    return JNI_VERSION_1_4;
}


// JNI_OnUnload
JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved) {
    ;
}


// Local log callback
static void logCallback(void *reference, const char *message)
{
    JNIEnv *env = NULL;
    
    if (state.midLogCallback == 0) { return; }
    
    (*state.javaVM)->AttachCurrentThread(state.javaVM, (void**)&env, NULL);
    if (env == NULL) { return; }
    
    //void logCallback(String message)
	{
        jstring messageString = ((*env)->NewStringUTF(env, message));
        jvalue args[1];
        args[0].l = (jobject)messageString;
        (*env)->CallStaticVoidMethod(env, state.classJomapi, state.midLogCallback, args);
        (*env)->DeleteLocalRef(env, messageString);
	}
    
    (*state.javaVM)->DetachCurrentThread(state.javaVM);
}


// Local device callback
static void deviceCallback(void *reference, int deviceId, OM_DEVICE_STATUS deviceStatus)
{
    JNIEnv *env = NULL;
    
    if (state.midDeviceCallback == 0) { return; }
    
    (*state.javaVM)->AttachCurrentThread(state.javaVM, (void**)&env, NULL);
    if (env == NULL) { return; }

    //void deviceCallback(int deviceId, int deviceStatus)
	{
        jvalue args[2];
        args[0].i = (jint)deviceId;
        args[1].i = (jint)deviceStatus;
        (*env)->CallStaticVoidMethod(env, state.classJomapi, state.midDeviceCallback, args);
	}
    
    (*state.javaVM)->DetachCurrentThread(state.javaVM);
}


// Local download callback
static void downloadCallback(void *reference, int deviceId, OM_DOWNLOAD_STATUS downloadStatus, int downloadValue)
{
    JNIEnv *env = NULL;
    
    if (state.midDownloadCallback == 0) { return; }
    
    (*state.javaVM)->AttachCurrentThread(state.javaVM, (void**)&env, NULL);
    if (env == NULL) { return; }
    
    //void downloadCallback(int deviceId, int downloadStatus, int downloadValue)
	{
        jvalue args[3];
        args[0].i = (jint)deviceId;
        args[1].i = (jint)downloadStatus;
        args[2].i = (jint)downloadValue;
        (*env)->CallStaticVoidMethod(env, state.classJomapi, state.midDownloadCallback, args);
	}
    
    (*state.javaVM)->DetachCurrentThread(state.javaVM);
}


JNIEXPORT jint JNICALL Java_JOMAPI_OmStartup(JNIEnv *env, jclass jObj, jint version)
{
    // Get handle to the JVM
    (*env)->GetJavaVM(env, &state.javaVM);
    
    // Fetch the Java class callback methods
    if (state.javaVM != NULL)
	{
        state.classJomapi = (*env)->FindClass(env, "JOMAPI");
        if (state.classJomapi != NULL)
		{
            state.midLogCallback = (*env)->GetStaticMethodID(env, state.classJomapi, "logCallback", "(Ljava/lang/String;)V"); //void logCallback(String message)
            state.midDeviceCallback = (*env)->GetStaticMethodID(env, state.classJomapi, "deviceCallback", "(II)V");           //void deviceCallback(int deviceId, int deviceStatus)
            state.midDownloadCallback = (*env)->GetStaticMethodID(env, state.classJomapi, "downloadCallback", "(III)V");      //void downloadCallback(int deviceId, int downloadStatus, int downloadValue)
		}
/*        
        else
		{
            jthrowable ex = (*env)->ExceptionOccurred(env);
            if (ex)
			{
                (*env)->ExceptionDescribe(env);
                (*env)->ExceptionClear(env);
			}
		}
*/
	}
    
    // Register our own OM callback functions
    OmSetLogCallback(logCallback, NULL);
    OmSetDeviceCallback(deviceCallback, NULL);
    OmSetDownloadCallback(downloadCallback, NULL);
    
    return OmStartup((int)version);
}

JNIEXPORT jint JNICALL Java_JOMAPI_OmShutdown(JNIEnv *env, jclass jObj)
{
	return OmShutdown();
}

JNIEXPORT jint JNICALL Java_JOMAPI_OmSetLogStream(JNIEnv *env, jclass jObj, jint fd)
{
	return OmSetLogStream((int)fd);
}
  
JNIEXPORT jint JNICALL Java_JOMAPI_OmGetDeviceIds(JNIEnv *env, jclass jObj, jintArray deviceIds, jint maxDevices)
{
    int *intDeviceIds;
    int retval;
    
    // Limit to array length and maximum possible devices
    jsize len = (*env)->GetArrayLength(env, deviceIds);
    if (maxDevices > len) { maxDevices = len; }
    if (maxDevices > 0xffff) { maxDevices = 0xffff; }
    
    // Make a true 'int' buffer for the arrays (in case of 64-bit C-int vs. 32-bit jint)
    intDeviceIds = (int *)malloc(maxDevices * sizeof(int));
    if (intDeviceIds == NULL)
	{
        (*env)->ThrowNew(env, (*env)->FindClass(env, "java/lang/Exception"), "Out of memory allocating native buffer for device IDs.");
        return OM_E_OUT_OF_MEMORY;
	}
    
    // Retrieve device IDs
    retval = OmGetDeviceIds(intDeviceIds, 0xffff);
    
    // Fill caller's return buffer
    if (retval >= 0)
	{
        int i;
        
        // Get array
        jint *elements = (*env)->GetIntArrayElements(env, deviceIds, 0);
        
        // Clip return length
        if (maxDevices > retval) { maxDevices = retval; }
        
        // Copy to jint array from C-int array
        for (i = 0; i < maxDevices; i++)
		{
            elements[i] = intDeviceIds[i];
		}
        
        // Release array
        (*env)->ReleaseIntArrayElements(env, deviceIds, elements, 0);
	}
    
    // Free local buffer
    free(intDeviceIds);
    
    return retval;
}

JNIEXPORT jint JNICALL Java_JOMAPI_OmGetVersion(JNIEnv *env, jclass jObj, jint deviceId, jintArray firmwareVersionArray, jintArray hardwareVersionArray)
{
    int firmwareVersion = 0, hardwareVersion = 0;
    int retval = OmGetVersion(deviceId, &firmwareVersion, &hardwareVersion);
	SET_OUT_INT(env, firmwareVersionArray, firmwareVersion);
	SET_OUT_INT(env, hardwareVersionArray, hardwareVersion);
    return retval;
}
  
JNIEXPORT jint JNICALL Java_JOMAPI_OmGetBatteryLevel(JNIEnv *env, jclass jObj, jint deviceId)
{
	return OmGetBatteryLevel(deviceId);
}

JNIEXPORT jint JNICALL Java_JOMAPI_OmSelfTest(JNIEnv *env, jclass jObj, jint deviceId)
{
	return OmSelfTest(deviceId);
}

JNIEXPORT jint JNICALL Java_JOMAPI_OmGetMemoryHealth(JNIEnv *env, jclass jObj, jint deviceId)
{
    return OmGetMemoryHealth(deviceId);
}
  
JNIEXPORT jint JNICALL Java_JOMAPI_OmGetBatteryHealth(JNIEnv *env, jclass jObj, jint deviceId)
{
	return OmGetBatteryHealth(deviceId);
}
  
JNIEXPORT jint JNICALL Java_JOMAPI_OmGetAccelerometer(JNIEnv *env, jclass jObj, jint deviceId, jintArray axesArray)
{
    int x = 0, y = 0, z = 0;
    int retval = OmGetAccelerometer(deviceId, &x, &y, &z);
    if (axesArray != NULL)	// Out: x, y, z
	{
		jint outval[3];
		outval[0] = x;
		outval[1] = y;
		outval[2] = z;
		(*env)->SetIntArrayRegion(env, axesArray, 0, 3, outval);
	}
    return retval;
}

JNIEXPORT jint JNICALL Java_JOMAPI_OmGetTime(JNIEnv *env, jclass jObj, jint deviceId, jlongArray timeArray)
{
    OM_DATETIME time = 0;
    int retval = OmGetTime(deviceId, &time);
	SET_OUT_LONG(env, timeArray, (jlong)time);
    return retval;
}
  
JNIEXPORT jint JNICALL Java_JOMAPI_OmSetTime(JNIEnv *env, jclass jObj, jint deviceId, jlong time)
{
    int retval = OmSetTime(deviceId, (OM_DATETIME)time);
    return retval;
}
    
JNIEXPORT jint JNICALL Java_JOMAPI_OmSetLed(JNIEnv *env, jclass jObj, jint deviceId, jint ledState)
{
	return OmSetLed(deviceId, ledState);
}
  
JNIEXPORT jint JNICALL Java_JOMAPI_OmIsLocked(JNIEnv *env, jclass jObj, jint deviceId, jintArray hasLockCodeArray)
{
    int hasLockCode = 0;
    int retval = OmIsLocked(deviceId, &hasLockCode);
	SET_OUT_LONG(env, hasLockCodeArray, hasLockCode);
    return retval;
}
  
JNIEXPORT jint JNICALL Java_JOMAPI_OmSetLock(JNIEnv *env, jclass jObj, jint deviceId, jint code)
{
	if (code < 0x0000 || code > 0xffff) { return OM_E_INVALID_ARG; }	// range-check unsigned short
	return OmSetLock(deviceId, (unsigned short)code);
}


JNIEXPORT jint JNICALL Java_JOMAPI_OmUnlock(JNIEnv *env, jclass jObj, jint deviceId, jint code)
{
	if (code < 0x0000 || code > 0xffff) { return OM_E_INVALID_ARG; }	// range-check unsigned short
	return OmUnlock(deviceId, (unsigned short)code);
}

JNIEXPORT jint JNICALL Java_JOMAPI_OmSetEcc(JNIEnv *env, jclass jObj, jint deviceId, jint state)
{
	return OmSetEcc(deviceId, state);
}

JNIEXPORT jint JNICALL Java_JOMAPI_OmGetEcc(JNIEnv *env, jclass jObj, jint deviceId)
{
	return OmGetEcc(deviceId);
}

// TODO: OmCommand(int deviceId, String command, String buffer, size_t bufferSize, String expected, unsigned int timeoutMs, char **parseParts, int parseMax);

JNIEXPORT jint JNICALL Java_JOMAPI_OmGetDelays(JNIEnv *env, jclass jObj, jint deviceId, jlongArray startTimeArray, jlongArray stopTimeArray)
{
	OM_DATETIME startTime = 0;
	OM_DATETIME stopTime = 0;
	int retval = OmGetDelays(deviceId, &startTime, &stopTime);
	SET_OUT_LONG(env, startTimeArray, startTime);
	SET_OUT_LONG(env, stopTimeArray, stopTime);
	return retval;
}

JNIEXPORT jint JNICALL Java_JOMAPI_OmSetDelays(JNIEnv *env, jclass jObj, jint deviceId, jlong startTime, jlong stopTime)
{
	if (startTime < 0x00000000ul || startTime > 0xfffffffful) { return OM_E_INVALID_ARG; }	// range-check unsigned long
	if (stopTime  < 0x00000000ul || stopTime  > 0xfffffffful) { return OM_E_INVALID_ARG; }	// range-check unsigned long
	return OmSetDelays(deviceId, (OM_DATETIME)startTime, (OM_DATETIME)stopTime);
}

JNIEXPORT jint JNICALL Java_JOMAPI_OmGetSessionId(JNIEnv *env, jclass jObj, jint deviceId, jlongArray sessionIdArray)
{
	unsigned int sessionId = 0;
	int retval = OmGetSessionId(deviceId, &sessionId);
	SET_OUT_LONG(env, sessionIdArray, sessionId);
	return retval;
}

JNIEXPORT jint JNICALL Java_JOMAPI_OmSetSessionId(JNIEnv *env, jclass jObj, jint deviceId, jlong sessionId)
{
	if (sessionId < 0x00000000ul || sessionId > 0xfffffffful) { return OM_E_INVALID_ARG; }	// range-check unsigned long
	return OmSetSessionId(deviceId, (unsigned int)sessionId);
}

// TODO: OmGetMetadata(int deviceId, [MarshalAs(UnmanagedType.LPStr)] StringBuilder metadata);

JNIEXPORT jint JNICALL Java_JOMAPI_OmSetMetadata(JNIEnv *env, jclass jObj, jint deviceId, jstring metadataString, jint size)
{
    int retval;
    const char *metadata = (*env)->GetStringUTFChars(env, metadataString, NULL);
    if (metadata == NULL) { return OM_E_OUT_OF_MEMORY; } // OutOfMemory already throuwn
    retval = OmSetMetadata(deviceId, metadata, size);
    (*env)->ReleaseStringUTFChars(env, metadataString, metadata);    
    return retval;
}

JNIEXPORT jint JNICALL Java_JOMAPI_OmGetLastConfigTime(JNIEnv *env, jclass jObj, jint deviceId, jlongArray timeArray)
{
    OM_DATETIME time = 0;
    int retval = OmGetLastConfigTime(deviceId, &time);
	SET_OUT_LONG(env, timeArray, time);
    return retval;
}
  
JNIEXPORT jint JNICALL Java_JOMAPI_OmEraseDataAndCommit(JNIEnv *env, jclass jObj, jint deviceId, jint eraseLevel)
{
	return OmEraseDataAndCommit(deviceId, eraseLevel);
}
  
JNIEXPORT jint JNICALL Java_JOMAPI_OmGetAccelConfig(JNIEnv *env, jclass jObj, jint deviceId, jintArray rateArray, jintArray rangeArray)
{
    int rate = 0, range = 0;
    int retval = OmGetAccelConfig(deviceId, &rate, &range);
	SET_OUT_INT(env, rateArray, rate);
	SET_OUT_INT(env, rangeArray, range);
    return retval;
}
    
JNIEXPORT jint JNICALL Java_JOMAPI_OmSetAccelConfig(JNIEnv *env, jclass jObj, jint deviceId, jint rate, jint range)
{
	return OmSetAccelConfig(deviceId, rate, range);
}
  
// TODO: OmGetDataFilename(int deviceId, [MarshalAs(UnmanagedType.LPStr)] StringBuilder filenameBuffer);

JNIEXPORT jint JNICALL Java_JOMAPI_OmGetDataRange(JNIEnv *env, jclass jObj, jint deviceId, jintArray dataBlockSizeArray, jintArray dataOffsetBlocksArray, jintArray dataNumBlocksArray, jlongArray startTimeArray, jlongArray endTimeArray)
{
    int dataBlockSize = 0, dataOffsetBlocks = 0, dataNumBlocks = 0;
	OM_DATETIME startTime = 0, endTime = 0;
    int retval = OmGetDataRange(deviceId, &dataBlockSize, &dataOffsetBlocks, &dataNumBlocks, &startTime, &endTime);
	SET_OUT_INT(env, dataBlockSizeArray, dataBlockSize);
	SET_OUT_INT(env, dataOffsetBlocksArray, dataOffsetBlocks);
	SET_OUT_INT(env, dataNumBlocksArray, dataNumBlocks);
	SET_OUT_LONG(env, startTimeArray, startTime);
	SET_OUT_LONG(env, endTimeArray, endTime);
    return retval;
}
  
JNIEXPORT jint JNICALL Java_JOMAPI_OmBeginDownloading(JNIEnv *env, jclass jObj, jint deviceId, jint dataOffsetBlocks, jint dataLengthBlocks, jstring destinationFileString)
{
    int retval;
    const char *destinationFile = (*env)->GetStringUTFChars(env, destinationFileString, NULL);
    if (destinationFile == NULL) { return OM_E_OUT_OF_MEMORY; } // OutOfMemory already throuwn
	retval = OmBeginDownloading(deviceId, dataOffsetBlocks, dataLengthBlocks, destinationFile);
    (*env)->ReleaseStringUTFChars(env, destinationFileString, destinationFile);
    return retval;
}

JNIEXPORT jint JNICALL Java_JOMAPI_OmQueryDownload(JNIEnv *env, jclass jObj, jint deviceId, jintArray downloadStatusArray, jintArray valueArray)
{
    OM_DOWNLOAD_STATUS downloadStatus = 0;
	int value = 0;
    int retval = OmQueryDownload(deviceId, &downloadStatus, &value);
	SET_OUT_INT(env, downloadStatusArray, downloadStatus);
	SET_OUT_INT(env, valueArray, value);
    return retval;
	
}

JNIEXPORT jint JNICALL Java_JOMAPI_OmWaitForDownload(JNIEnv *env, jclass jObj, jint deviceId, jintArray downloadStatusArray, jintArray valueArray)
{
    OM_DOWNLOAD_STATUS downloadStatus = 0;
	int value = 0;
    int retval = OmWaitForDownload(deviceId, &downloadStatus, &value);
	SET_OUT_INT(env, downloadStatusArray, downloadStatus);
	SET_OUT_INT(env, valueArray, value);
    return retval;
}
  
JNIEXPORT jint JNICALL Java_JOMAPI_OmCancelDownload(JNIEnv *env, jclass jObj, jint deviceId)
{
	return OmCancelDownload(deviceId);
}
  
JNIEXPORT jstring JNICALL Java_JOMAPI_OmErrorString(JNIEnv *env, jclass jObj, jint status)
{
    const char *error = OmErrorString(status);
	jstring errorString = ((*env)->NewStringUTF(env, error));
    //(*env)->DeleteLocalRef(env, errorString);		// Is it safe to delete our reference for return value?
	return errorString;
}

JNIEXPORT jlong JNICALL Java_JOMAPI_OmReaderOpen(JNIEnv *env, jclass jObj, jstring binaryFilenameString)
{
    OmReaderHandle reader = NULL;
    const char *binaryFilename = (*env)->GetStringUTFChars(env, binaryFilenameString, NULL);
    if (binaryFilename == NULL) { return OM_E_OUT_OF_MEMORY; } // OutOfMemory already throuwn
	reader = OmReaderOpen(binaryFilename);
    (*env)->ReleaseStringUTFChars(env, binaryFilenameString, binaryFilename);
    return (jlong)reader;
}

JNIEXPORT jint JNICALL Java_JOMAPI_OmReaderDataRange(JNIEnv *env, jclass jObj, jlong reader, jintArray dataBlockSizeArray, jintArray dataOffsetBlocksArray, jintArray dataNumBlocksArray, jlongArray startTimeArray, jlongArray endTimeArray)
{
    int dataBlockSize = 0, dataOffsetBlocks = 0, dataNumBlocks = 0;
	OM_DATETIME startTime = 0, endTime = 0;
    int retval = OmReaderDataRange((OmReaderHandle)reader, &dataBlockSize, &dataOffsetBlocks, &dataNumBlocks, &startTime, &endTime);
	SET_OUT_INT(env, dataBlockSizeArray, dataBlockSize);
	SET_OUT_INT(env, dataOffsetBlocksArray, dataOffsetBlocks);
	SET_OUT_INT(env, dataNumBlocksArray, dataNumBlocks);
	SET_OUT_LONG(env, startTimeArray, startTime);
	SET_OUT_LONG(env, endTimeArray, endTime);
    return retval;
}

JNIEXPORT jstring JNICALL Java_JOMAPI_OmReaderMetadata(JNIEnv *env, jclass jObj, jlong reader, jintArray deviceIdArray, jlongArray sessionIdArray)
{
    int deviceId;
	unsigned int sessionId;
	jstring metadataString;
	const char *metadata = OmReaderMetadata((OmReaderHandle)reader, &deviceId, &sessionId);
	SET_OUT_INT(env, deviceIdArray, deviceId);
	SET_OUT_INT(env, sessionIdArray, sessionId);
	jstring metadataString = ((*env)->NewStringUTF(env, metadata));
    //(*env)->DeleteLocalRef(env, metadataString);		// Is it safe to delete our reference for return value?
	return metadataString;
}

JNIEXPORT jint JNICALL Java_JOMAPI_OmReaderDataBlockPosition(JNIEnv *env, jclass jObj, jlong reader)
{
	return OmReaderDataBlockPosition((OmReaderHandle)reader);
}

JNIEXPORT jint JNICALL Java_JOMAPI_OmReaderDataBlockSeek(JNIEnv *env, jclass jObj, jlong reader, jint dataBlockNumber)
{
	return OmReaderDataBlockSeek((OmReaderHandle)reader, dataBlockNumber);
}

JNIEXPORT jint JNICALL Java_JOMAPI_OmReaderNextBlock(JNIEnv *env, jclass jObj, jlong reader)
{
	return OmReaderNextBlock((OmReaderHandle)reader);
}

JNIEXPORT jlong JNICALL Java_JOMAPI_OmReaderBuffer(JNIEnv *env, jclass jObj, jlong reader)
{
	return (jlong)OmReaderBuffer((OmReaderHandle)reader);
}

JNIEXPORT jlong JNICALL Java_JOMAPI_OmReaderTimestamp(JNIEnv *env, jclass jObj, jlong reader, jint index, jintArray fractionalArray)
{
	OM_DATETIME timestamp = 0;
	unsigned short fractional = 0;
	timestamp = OmReaderTimestamp(index, &fractional);
	SET_OUT_INT(env, fractionalArray, fractional);
	return timestamp;
}

JNIEXPORT jint JNICALL Java_JOMAPI_OmReaderGetValue(JNIEnv *env, jclass jObj, jlong reader, jint valueType)
{
	return OmReaderGetValue((OmReaderHandle)reader, (OM_READER_VALUE_TYPE)valueType);
}

// TODO: OM_READER_HEADER_PACKET OmReaderRawHeaderPacket(IntPtr reader);

// TODO: OM_READER_DATA_PACKET OmReaderRawDataPacket(IntPtr reader);

JNIEXPORT void JNICALL Java_JOMAPI_OmReaderClose(JNIEnv *env, jclass jObj, jlong reader)
{
	OmReaderClose((OmReaderHandle)reader);
}
  
  
#ifdef __cplusplus
}
#endif 

