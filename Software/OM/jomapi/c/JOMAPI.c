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

#include "openmovement_JOMAPI.h"
#include "omapi.h"

#ifdef __cplusplus
extern "C" {
#endif 


// Utilitiy define for integer 'out' parameters (hackily simulated with single-element arrays)
#define SET_OUT_INT(env, array, value) \
	if (array != NULL && (*env)->GetArrayLength(env, array) >= 1) \
	{ \
		jint outval[1]; \
		outval[0] = value; \
		(*env)->SetIntArrayRegion(env, array, 0, 1, outval); \
	}
	
// Utilitiy define for long 'out' parameters (hackily simulated with single-element arrays)
#define SET_OUT_LONG(env, array, value) \
	if (array != NULL && (*env)->GetArrayLength(env, array) >= 1) \
	{ \
		jlong outval[1]; \
		outval[0] = value; \
		(*env)->SetLongArrayRegion(env, array, 0, 1, outval); \
	}
	
// Utilitiy define for String 'out' parameters (hackily simulated with StringBuffer objects)
#define SET_OUT_STRINGBUFFER(env, bufferObject, buffer) \
	if (bufferObject != NULL) \
	{ \
		jclass sbClass = (*env)->GetObjectClass(env, bufferObject); \
		jmethodID midReplace = (*env)->GetMethodID(env, sbClass, "append", "(IILjava/lang/String;)Ljava/lang/StringBuffer;"); \
		jstring tempString = (*env)->NewStringUTF(env, buffer); \
        jvalue args[3]; \
		args[0].i = 0; \
		args[1].i = 0x7fffffffL; \
        args[2].l = (jobject)tempString; \
		(*env)->CallObjectMethodA(env, bufferObject, midReplace, args); \
		(*env)->DeleteLocalRef(env, tempString);	/* Assume it is safe to delete our local reference? */ \
	}

// Utility define to throw an exception
#define THROW_EXCEPTION(env, exceptionClass, message) { (*env)->ThrowNew(env, (*env)->FindClass(env, exceptionClass), message); }
	
// Utility define to catch, report and clear an exception
#define CATCH_REPORT_CLEAR(env) \
	{ \
		jthrowable ex = (*env)->ExceptionOccurred(env); \
		if (ex) \
		{ \
			(*env)->ExceptionDescribe(env); \
			(*env)->ExceptionClear(env); \
		} \
	}


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
        (*env)->CallStaticVoidMethodA(env, state.classJomapi, state.midLogCallback, args);
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
        (*env)->CallStaticVoidMethodA(env, state.classJomapi, state.midDeviceCallback, args);
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
        (*env)->CallStaticVoidMethodA(env, state.classJomapi, state.midDownloadCallback, args);
	}
    
    (*state.javaVM)->DetachCurrentThread(state.javaVM);
}


JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmStartup(JNIEnv *env, jclass jObj, jint version)
{
    // Get handle to the JVM
    (*env)->GetJavaVM(env, &state.javaVM);
    
    // Fetch the Java class callback methods
    if (state.javaVM != NULL)
	{
        state.classJomapi = (*env)->FindClass(env, "openmovement/JOMAPI");
        if (state.classJomapi != NULL)
		{
            state.midLogCallback = (*env)->GetStaticMethodID(env, state.classJomapi, "logCallback", "(Ljava/lang/String;)V"); //void logCallback(String message)
            state.midDeviceCallback = (*env)->GetStaticMethodID(env, state.classJomapi, "deviceCallback", "(II)V");           //void deviceCallback(int deviceId, int deviceStatus)
            state.midDownloadCallback = (*env)->GetStaticMethodID(env, state.classJomapi, "downloadCallback", "(III)V");      //void downloadCallback(int deviceId, int downloadStatus, int downloadValue)
		}
	}
    
    // Register our own OM callback functions
    OmSetLogCallback(logCallback, NULL);
    OmSetDeviceCallback(deviceCallback, NULL);
    OmSetDownloadCallback(downloadCallback, NULL);
    
    return OmStartup((int)version);
}

JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmShutdown(JNIEnv *env, jclass jObj)
{
	return OmShutdown();
}

JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmSetLogStream(JNIEnv *env, jclass jObj, jint fd)
{
	return OmSetLogStream((int)fd);
}
  
JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmGetDeviceIds(JNIEnv *env, jclass jObj, jintArray deviceIds, jint maxDevices)
{
    int *intDeviceIds;
    int retval;
    
    // Limit to array length and maximum possible devices
    jsize arrayLen = 0;
    if (deviceIds != NULL) { arrayLen = (*env)->GetArrayLength(env, deviceIds); }
    if (maxDevices < 0) { maxDevices = 0; }
    if (maxDevices > arrayLen) { maxDevices = arrayLen; }
    if (maxDevices > 0xffff) { maxDevices = 0xffff; }
    
    // Make a true 'int' buffer for the arrays (in case of 64-bit C-int vs. 32-bit jint)
    intDeviceIds = NULL;
    if (deviceIds != NULL && maxDevices > 0)
    {
        intDeviceIds = (int *)malloc(maxDevices * sizeof(int));
        if (intDeviceIds == NULL)
        {
            THROW_EXCEPTION(env, "java/lang/OutOfMemoryError", "Out of memory allocating native buffer for device IDs.");
            return OM_E_OUT_OF_MEMORY;
        }
    }
    
    // Retrieve device IDs
    retval = OmGetDeviceIds(intDeviceIds, maxDevices);
    
    if (intDeviceIds != NULL)
    {
        // Fill caller's return buffer
        if (deviceIds != NULL && retval >= 0)
        {
            int count;
            int i;
            
            // Get array
            jint *elements = (*env)->GetIntArrayElements(env, deviceIds, 0);
            
            // Clip return length
            count = retval;
            if (count > maxDevices) { count = maxDevices; }
            
            // Copy to jint array from C-int array
            for (i = 0; i < count; i++)
            {
                elements[i] = intDeviceIds[i];
            }
            
            // Release array
            (*env)->ReleaseIntArrayElements(env, deviceIds, elements, 0);
        }
        
        // Free local buffer
        free(intDeviceIds);
    }
    
    return retval;
}

JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmGetVersion(JNIEnv *env, jclass jObj, jint deviceId, jintArray firmwareVersionArray, jintArray hardwareVersionArray)
{
    int firmwareVersion = 0, hardwareVersion = 0;
    int retval = OmGetVersion(deviceId, &firmwareVersion, &hardwareVersion);
	SET_OUT_INT(env, firmwareVersionArray, firmwareVersion);
	SET_OUT_INT(env, hardwareVersionArray, hardwareVersion);
    return retval;
}
  
JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmGetBatteryLevel(JNIEnv *env, jclass jObj, jint deviceId)
{
	return OmGetBatteryLevel(deviceId);
}

JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmSelfTest(JNIEnv *env, jclass jObj, jint deviceId)
{
	return OmSelfTest(deviceId);
}

JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmGetMemoryHealth(JNIEnv *env, jclass jObj, jint deviceId)
{
    return OmGetMemoryHealth(deviceId);
}
  
JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmGetBatteryHealth(JNIEnv *env, jclass jObj, jint deviceId)
{
	return OmGetBatteryHealth(deviceId);
}
  
JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmGetAccelerometer(JNIEnv *env, jclass jObj, jint deviceId, jintArray xArray, jintArray yArray, jintArray zArray)
{
    int x = 0, y = 0, z = 0;
    int retval = OmGetAccelerometer(deviceId, &x, &y, &z);
	SET_OUT_INT(env, xArray, x);
	SET_OUT_INT(env, yArray, y);
	SET_OUT_INT(env, zArray, z);
    return retval;
}

JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmGetTime(JNIEnv *env, jclass jObj, jint deviceId, jlongArray timeArray)
{
    OM_DATETIME time = 0;
    int retval = OmGetTime(deviceId, &time);
	SET_OUT_LONG(env, timeArray, (jlong)time);
    return retval;
}
  
JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmSetTime(JNIEnv *env, jclass jObj, jint deviceId, jlong time)
{
    int retval = OmSetTime(deviceId, (OM_DATETIME)time);
    return retval;
}
    
JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmSetLed(JNIEnv *env, jclass jObj, jint deviceId, jint ledState)
{
	return OmSetLed(deviceId, ledState);
}
  
JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmIsLocked(JNIEnv *env, jclass jObj, jint deviceId, jintArray hasLockCodeArray)
{
    int hasLockCode = 0;
    int retval = OmIsLocked(deviceId, &hasLockCode);
	SET_OUT_LONG(env, hasLockCodeArray, hasLockCode);
    return retval;
}
  
JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmSetLock(JNIEnv *env, jclass jObj, jint deviceId, jint code)
{
	if (code < 0x0000 || code > 0xffff) { return OM_E_INVALID_ARG; }	// range-check unsigned short
	return OmSetLock(deviceId, (unsigned short)code);
}


JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmUnlock(JNIEnv *env, jclass jObj, jint deviceId, jint code)
{
	if (code < 0x0000 || code > 0xffff) { return OM_E_INVALID_ARG; }	// range-check unsigned short
	return OmUnlock(deviceId, (unsigned short)code);
}

JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmSetEcc(JNIEnv *env, jclass jObj, jint deviceId, jint state)
{
	return OmSetEcc(deviceId, state);
}

JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmGetEcc(JNIEnv *env, jclass jObj, jint deviceId)
{
	return OmGetEcc(deviceId);
}

// TODO: OmCommand()
/*
JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmCommand(JNIEnv *env, jclass jObj, jint deviceId, jstring command, jbyteArray buffer, jint bufferSize, jstring expected, jint timeoutMs, jobjectArray parseParts, jint parseMax)
{
	// OmCommand(int deviceId, String command, String buffer, size_t bufferSize, String expected, unsigned int timeoutMs, char **parseParts, int parseMax);
}
*/

JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmGetDelays(JNIEnv *env, jclass jObj, jint deviceId, jlongArray startTimeArray, jlongArray stopTimeArray)
{
	OM_DATETIME startTime = 0;
	OM_DATETIME stopTime = 0;
	int retval = OmGetDelays(deviceId, &startTime, &stopTime);
	SET_OUT_LONG(env, startTimeArray, startTime);
	SET_OUT_LONG(env, stopTimeArray, stopTime);
	return retval;
}

JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmSetDelays(JNIEnv *env, jclass jObj, jint deviceId, jlong startTime, jlong stopTime)
{
	if (startTime < 0x00000000ul || startTime > 0xfffffffful) { return OM_E_INVALID_ARG; }	// range-check unsigned long
	if (stopTime  < 0x00000000ul || stopTime  > 0xfffffffful) { return OM_E_INVALID_ARG; }	// range-check unsigned long
	return OmSetDelays(deviceId, (OM_DATETIME)startTime, (OM_DATETIME)stopTime);
}

JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmGetSessionId(JNIEnv *env, jclass jObj, jint deviceId, jlongArray sessionIdArray)
{
	unsigned int sessionId = 0;
	int retval = OmGetSessionId(deviceId, &sessionId);
	SET_OUT_LONG(env, sessionIdArray, sessionId);
	return retval;
}

JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmSetSessionId(JNIEnv *env, jclass jObj, jint deviceId, jlong sessionId)
{
	if (sessionId < 0x00000000ul || sessionId > 0xfffffffful) { return OM_E_INVALID_ARG; }	// range-check unsigned long
	return OmSetSessionId(deviceId, (unsigned int)sessionId);
}

JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmGetMetadata(JNIEnv *env, jclass jObj, jint deviceId, jobject metadataBufferObject)
{
	int retval;
	char metadataBuffer[OM_METADATA_SIZE + 1] = {0};
	retval = OmGetMetadata(deviceId, metadataBuffer);
	SET_OUT_STRINGBUFFER(env, metadataBufferObject, metadataBuffer);
	return retval;
}

JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmSetMetadata(JNIEnv *env, jclass jObj, jint deviceId, jstring metadataString, jint size)
{
    int retval;
    const char *metadata = (*env)->GetStringUTFChars(env, metadataString, NULL);
    if (metadata == NULL) { return OM_E_OUT_OF_MEMORY; } // OutOfMemory already throuwn
    retval = OmSetMetadata(deviceId, metadata, size);
    (*env)->ReleaseStringUTFChars(env, metadataString, metadata);    
    return retval;
}

JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmGetLastConfigTime(JNIEnv *env, jclass jObj, jint deviceId, jlongArray timeArray)
{
    OM_DATETIME time = 0;
    int retval = OmGetLastConfigTime(deviceId, &time);
	SET_OUT_LONG(env, timeArray, time);
    return retval;
}
  
JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmEraseDataAndCommit(JNIEnv *env, jclass jObj, jint deviceId, jint eraseLevel)
{
	return OmEraseDataAndCommit(deviceId, eraseLevel);
}
  
JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmGetAccelConfig(JNIEnv *env, jclass jObj, jint deviceId, jintArray rateArray, jintArray rangeArray)
{
    int rate = 0, range = 0;
    int retval = OmGetAccelConfig(deviceId, &rate, &range);
	SET_OUT_INT(env, rateArray, rate);
	SET_OUT_INT(env, rangeArray, range);
    return retval;
}
    
JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmSetAccelConfig(JNIEnv *env, jclass jObj, jint deviceId, jint rate, jint range)
{
	return OmSetAccelConfig(deviceId, rate, range);
}
  
JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmGetDataFilename(JNIEnv *env, jclass jObj, jint deviceId, jobject filenameBufferObject)
{
	int retval;
	char filenameBuffer[OM_MAX_PATH + 1] = {0};
	retval = OmGetDataFilename(deviceId, filenameBuffer);
	SET_OUT_STRINGBUFFER(env, filenameBufferObject, filenameBuffer);
	return retval;
}

JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmGetDataRange(JNIEnv *env, jclass jObj, jint deviceId, jintArray dataBlockSizeArray, jintArray dataOffsetBlocksArray, jintArray dataNumBlocksArray, jlongArray startTimeArray, jlongArray endTimeArray)
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
  
JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmBeginDownloading(JNIEnv *env, jclass jObj, jint deviceId, jint dataOffsetBlocks, jint dataLengthBlocks, jstring destinationFileString)
{
    int retval;
    const char *destinationFile = (*env)->GetStringUTFChars(env, destinationFileString, NULL);
    if (destinationFile == NULL) { return OM_E_OUT_OF_MEMORY; } // OutOfMemory already throuwn
	retval = OmBeginDownloading(deviceId, dataOffsetBlocks, dataLengthBlocks, destinationFile);
    (*env)->ReleaseStringUTFChars(env, destinationFileString, destinationFile);
    return retval;
}

JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmQueryDownload(JNIEnv *env, jclass jObj, jint deviceId, jintArray downloadStatusArray, jintArray valueArray)
{
    OM_DOWNLOAD_STATUS downloadStatus = 0;
	int value = 0;
    int retval = OmQueryDownload(deviceId, &downloadStatus, &value);
	SET_OUT_INT(env, downloadStatusArray, downloadStatus);
	SET_OUT_INT(env, valueArray, value);
    return retval;
	
}

JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmWaitForDownload(JNIEnv *env, jclass jObj, jint deviceId, jintArray downloadStatusArray, jintArray valueArray)
{
    OM_DOWNLOAD_STATUS downloadStatus = 0;
	int value = 0;
    int retval = OmWaitForDownload(deviceId, &downloadStatus, &value);
	SET_OUT_INT(env, downloadStatusArray, downloadStatus);
	SET_OUT_INT(env, valueArray, value);
    return retval;
}
  
JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmCancelDownload(JNIEnv *env, jclass jObj, jint deviceId)
{
	return OmCancelDownload(deviceId);
}
  
JNIEXPORT jstring JNICALL Java_openmovement_JOMAPI_OmErrorString(JNIEnv *env, jclass jObj, jint status)
{
    const char *error = OmErrorString(status);
	jstring errorString = ((*env)->NewStringUTF(env, error));
    //(*env)->DeleteLocalRef(env, errorString);		// Is it safe to delete our local reference for return value?
	return errorString;
}

JNIEXPORT jlong JNICALL Java_openmovement_JOMAPI_OmReaderOpen(JNIEnv *env, jclass jObj, jstring binaryFilenameString)
{
    OmReaderHandle reader = NULL;
    const char *binaryFilename = (*env)->GetStringUTFChars(env, binaryFilenameString, NULL);
    if (binaryFilename == NULL) { return OM_E_OUT_OF_MEMORY; } // OutOfMemory already throuwn
	reader = OmReaderOpen(binaryFilename);
    (*env)->ReleaseStringUTFChars(env, binaryFilenameString, binaryFilename);
    return (jlong)reader;
}

JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmReaderDataRange(JNIEnv *env, jclass jObj, jlong reader, jintArray dataBlockSizeArray, jintArray dataOffsetBlocksArray, jintArray dataNumBlocksArray, jlongArray startTimeArray, jlongArray endTimeArray)
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

JNIEXPORT jstring JNICALL Java_openmovement_JOMAPI_OmReaderMetadata(JNIEnv *env, jclass jObj, jlong reader, jintArray deviceIdArray, jlongArray sessionIdArray)
{
    int deviceId;
	unsigned int sessionId;
	const char *metadata = OmReaderMetadata((OmReaderHandle)reader, &deviceId, &sessionId);
	jstring metadataString = ((*env)->NewStringUTF(env, metadata));
	SET_OUT_INT(env, deviceIdArray, deviceId);
	SET_OUT_INT(env, sessionIdArray, sessionId);
    //(*env)->DeleteLocalRef(env, metadataString);		// Is it safe to delete our local reference for return value?
	return metadataString;
}

JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmReaderDataBlockPosition(JNIEnv *env, jclass jObj, jlong reader)
{
	return OmReaderDataBlockPosition((OmReaderHandle)reader);
}

JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmReaderDataBlockSeek(JNIEnv *env, jclass jObj, jlong reader, jint dataBlockNumber)
{
	return OmReaderDataBlockSeek((OmReaderHandle)reader, dataBlockNumber);
}

JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmReaderNextBlock(JNIEnv *env, jclass jObj, jlong reader)
{
	return OmReaderNextBlock((OmReaderHandle)reader);
}

JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmReaderBufferCopy(JNIEnv *env, jclass jObj, jlong reader, jshortArray bufferArray)
{
	const short *buffer;
	int count;
	jsize arrayLen;
	if (reader == 0) { return OM_E_POINTER; }
	if (bufferArray == 0) { return OM_E_POINTER; }
	buffer = OmReaderBuffer((OmReaderHandle)reader);
	if (buffer == NULL) { return OM_E_UNEXPECTED; }
	count = OM_MAX_SAMPLES;
    arrayLen = (*env)->GetArrayLength(env, bufferArray);
    if (arrayLen < count) { count = arrayLen; }
	(*env)->SetShortArrayRegion(env, bufferArray, 0, count, buffer);
	return OM_OK;
}

JNIEXPORT jlong JNICALL Java_openmovement_JOMAPI_OmReaderTimestamp(JNIEnv *env, jclass jObj, jlong reader, jint index, jintArray fractionalArray)
{
	OM_DATETIME timestamp = 0;
	unsigned short fractional = 0;
	timestamp = OmReaderTimestamp((OmReaderHandle)reader, index, &fractional);
	SET_OUT_INT(env, fractionalArray, fractional);
	return timestamp;
}

JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmReaderGetValue(JNIEnv *env, jclass jObj, jlong reader, jint valueType)
{
	return OmReaderGetValue((OmReaderHandle)reader, (OM_READER_VALUE_TYPE)valueType);
}

JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmReaderRawHeaderPacketCopy(JNIEnv *env, jclass jObj, jlong reader, jbyteArray headerPacketArray)
{
	const OM_READER_HEADER_PACKET *headerPacket;
	int count;
	jsize arrayLen;
	if (reader == 0) { return OM_E_POINTER; }
	if (headerPacketArray == 0) { return OM_E_POINTER; }
	headerPacket = OmReaderRawHeaderPacket((OmReaderHandle)reader);
	if (headerPacket == NULL) { return OM_E_UNEXPECTED; }
	count = OM_MAX_HEADER_SIZE;
    arrayLen = (*env)->GetArrayLength(env, headerPacketArray);
    if (arrayLen < count) { count = arrayLen; }
	(*env)->SetByteArrayRegion(env, headerPacketArray, 0, count, (const char *)headerPacket);
	return OM_OK;
}

JNIEXPORT jint JNICALL Java_openmovement_JOMAPI_OmReaderRawDataPacketCopy(JNIEnv *env, jclass jObj, jlong reader, jbyteArray dataPacketArray)
{
	const OM_READER_DATA_PACKET *dataPacket;
	int count;
	jsize arrayLen;
	if (reader == 0) { return OM_E_POINTER; }
	if (dataPacketArray == 0) { return OM_E_POINTER; }
	dataPacket = OmReaderRawDataPacket((OmReaderHandle)reader);
	if (dataPacket == NULL) { return OM_E_UNEXPECTED; }
	count = OM_MAX_DATA_SIZE;
    arrayLen = (*env)->GetArrayLength(env, dataPacketArray);
    if (arrayLen < count) { count = arrayLen; }
	(*env)->SetByteArrayRegion(env, dataPacketArray, 0, count, (const char *)dataPacket);
	return OM_OK;
}

JNIEXPORT void JNICALL Java_openmovement_JOMAPI_OmReaderClose(JNIEnv *env, jclass jObj, jlong reader)
{
	OmReaderClose((OmReaderHandle)reader);
}
  
  
#ifdef __cplusplus
}
#endif 

