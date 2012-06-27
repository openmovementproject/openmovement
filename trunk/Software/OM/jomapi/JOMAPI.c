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


// JNI_OnLoad
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    state.javaVM = vm;
    return JNI_VERSION_1_4;
}


// JNI_OnUnload
JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved) {
    ;
}



JNIEXPORT jint JNICALL Java_JOMAPI_OmStartup
  (JNIEnv *env, jclass jObj, jint version)
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
    
    return OmStartup(version);
  }

  
JNIEXPORT jint JNICALL Java_JOMAPI_OmShutdown
  (JNIEnv *env, jclass jObj)
  { return OmShutdown(); }

  
JNIEXPORT jint JNICALL Java_JOMAPI_OmSetLogStream
  (JNIEnv *env, jclass jObj, jint fd)
  { return OmSetLogStream(fd); }
  
  
JNIEXPORT jint JNICALL Java_JOMAPI_OmGetDeviceIds
  (JNIEnv *env, jclass jObj, jintArray deviceIds, jint maxDevices)
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


JNIEXPORT jint JNICALL Java_JOMAPI_OmGetVersion
  (JNIEnv *env, jclass jObj, jint deviceId, jintArray firmwareVersionArray, jintArray hardwareVersionArray)
  {
    int firmwareVersion = 0, hardwareVersion = 0;
    int retval = OmGetVersion(deviceId, &firmwareVersion, &hardwareVersion);
    // Out: firmwareVersion
    {
        jint outval[1];
        outval[0] = firmwareVersion;
        (*env)->SetIntArrayRegion(env, firmwareVersionArray, 0, 1, outval);
    }
    // Out: hardwareVersion
    {
        jint outval[1];
        outval[0] = hardwareVersion;
        (*env)->SetIntArrayRegion(env, hardwareVersionArray, 0, 1, outval);
    }
    return retval;
  }
  
JNIEXPORT jint JNICALL Java_JOMAPI_OmGetBatteryLevel
  (JNIEnv *env, jclass jObj, jint deviceId)
  { return OmGetBatteryLevel(deviceId); }

JNIEXPORT jint JNICALL Java_JOMAPI_OmSelfTest
  (JNIEnv *env, jclass jObj, jint deviceId)
  { return OmSelfTest(deviceId); }

JNIEXPORT jint JNICALL Java_JOMAPI_OmGetMemoryHealth
  (JNIEnv *env, jclass jObj, jint deviceId)
  { return OmGetMemoryHealth(deviceId); }
  
JNIEXPORT jint JNICALL Java_JOMAPI_OmGetBatteryHealth
  (JNIEnv *env, jclass jObj, jint deviceId)
  { return OmGetBatteryHealth(deviceId); }
  
JNIEXPORT jint JNICALL Java_JOMAPI_OmGetAccelerometer
  (JNIEnv *env, jclass jObj, jint deviceId, jintArray axesArray)
  {
    int x = 0, y = 0, z = 0;
    int retval = OmGetAccelerometer(deviceId, &x, &y, &z);
    // Out: x, y, z
    {
        jint outval[3];
        outval[0] = x;
        outval[1] = y;
        outval[2] = z;
        (*env)->SetIntArrayRegion(env, axesArray, 0, 3, outval);
    }
    return retval;
  }

JNIEXPORT jint JNICALL Java_JOMAPI_OmGetTime
  (JNIEnv *env, jclass jObj, jint deviceId, jlongArray timeArray)
  {
    OM_DATETIME time = 0;
    int retval;
    retval = OmGetTime(deviceId, &time);
    // Out: time
    {
        jlong outval[1];
        outval[0] = (jlong)time;
        (*env)->SetLongArrayRegion(env, timeArray, 0, 1, outval);
    }
    return retval;
  }
  
JNIEXPORT jint JNICALL Java_JOMAPI_OmSetTime
  (JNIEnv *env, jclass jObj, jint deviceId, jlong time)
  {
    int retval;
    retval = OmSetTime(deviceId, (OM_DATETIME)time);
    return retval;
  }
    
JNIEXPORT jint JNICALL Java_JOMAPI_OmSetLed
  (JNIEnv *env, jclass jObj, jint deviceId, jint ledState)
  { return OmSetLed(deviceId, ledState); }
  
// TODO: OmIsLocked(int deviceId, out int hasLockCode);
// TODO: OmSetLock(int deviceId, ushort code);
// TODO: OmUnlock(int deviceId, ushort code);
// TODO: OmSetEcc(int deviceId, int state);
// TODO: OmGetEcc(int deviceId, int state);
// TODO: OmCommand(int deviceId, String command, String buffer, size_t bufferSize, String expected, unsigned int timeoutMs, char **parseParts, int parseMax);
// TODO: OmGetDelays(int deviceId, out uint startTime, out uint stopTime);
// TODO: OmSetDelays(int deviceId, uint startTime, uint stopTime);
// TODO: OmGetSessionId(int deviceId, out uint sessionId);
// TODO: OmSetSessionId(int deviceId, uint sessionId);

// TODO: OmGetMetadata(int deviceId, [MarshalAs(UnmanagedType.LPStr)] StringBuilder metadata);

JNIEXPORT jint JNICALL Java_JOMAPI_OmSetMetadata
  (JNIEnv *env, jclass jObj, jint deviceId, jstring metadataString, jint size)
  {
    int retval;
    jboolean iscopy;
    const char *metadata = (*env)->GetStringUTFChars(env, metadataString, NULL);
    if (metadata == NULL) { return OM_E_OUT_OF_MEMORY; } // OutOfMemory already throuwn
    retval = OmSetMetadata(deviceId, metadata, size);
    (*env)->ReleaseStringUTFChars(env, metadataString, metadata);    
    return retval;
  }

JNIEXPORT jint JNICALL Java_JOMAPI_OmGetLastConfigTime
  (JNIEnv *env, jclass jObj, jint deviceId, jlongArray timeArray)
  {
    OM_DATETIME time = 0;
    int retval;
    retval = OmGetLastConfigTime(deviceId, &time);
    // Out: time
    {
        jlong outval[1];
        outval[0] = (jlong)time;
        (*env)->SetLongArrayRegion(env, timeArray, 0, 1, outval);
    }
    return retval;
  }
  
JNIEXPORT jint JNICALL Java_JOMAPI_OmEraseDataAndCommit
  (JNIEnv *env, jclass jObj, jint deviceId, jint eraseLevel)
  { return OmEraseDataAndCommit(deviceId, eraseLevel); }
  
JNIEXPORT jint JNICALL Java_JOMAPI_OmGetAccelConfig
  (JNIEnv *env, jclass jObj, jint deviceId, jintArray rateArray, jintArray rangeArray)
  {
    int rate = 0, range = 0;
    int retval = OmGetAccelConfig(deviceId, &rate, &range);
    // Out: rate
    {
        jint outval[1];
        outval[0] = rate;
        (*env)->SetIntArrayRegion(env, rateArray, 0, 1, outval);
    }
    // Out: range
    {
        jint outval[1];
        outval[0] = range;
        (*env)->SetIntArrayRegion(env, rangeArray, 0, 1, outval);
    }
    return retval;
  }
    
JNIEXPORT jint JNICALL Java_JOMAPI_OmSetAccelConfig
  (JNIEnv *env, jclass jObj, jint deviceId, jint rate, jint range)
  { return OmSetAccelConfig(deviceId, rate, range); }
  
// TODO: OmSetDownloadCallback(OmDownloadCallback downloadCallback, IntPtr reference);
// TODO: OmGetDataFilename(int deviceId, [MarshalAs(UnmanagedType.LPStr)] StringBuilder filenameBuffer);
// TODO: OmGetDataRange(int deviceId, out int dataBlockSize, out int dataOffsetBlocks, out int dataNumBlocks, out uint startTime, out uint endTime);
// TODO: OmBeginDownloading(int deviceId, int dataOffsetBlocks, int dataLengthBlocks, string destinationFile);
// TODO: OmQueryDownload(int deviceId, out OM_DOWNLOAD_STATUS downloadStatus, out int value);
// TODO: OmWaitForDownload(int deviceId, out OM_DOWNLOAD_STATUS downloadStatus, out int value);	
  
JNIEXPORT jint JNICALL Java_JOMAPI_OmCancelDownload
  (JNIEnv *env, jclass jObj, jint deviceId)
  { return OmCancelDownload(deviceId); }
  
JNIEXPORT jstring JNICALL Java_JOMAPI_OmErrorString
  (JNIEnv *env, jclass jObj, jint status)
  {
    const char *str = OmErrorString(status);
    return ((*env)->NewStringUTF(env, str));
  }

// TODO: IntPtr OmReaderOpen(string binaryFilename);
// TODO: int OmReaderDataRange(IntPtr reader, out int dataBlockSize, out int dataOffsetBlocks, out int dataNumBlocks, out uint startTime, out uint endTime);
// TODO: string OmReaderMetadata(IntPtr reader, out int deviceId, out uint sessionId);
// TODO: int OmReaderDataBlockPosition(IntPtr reader);
// TODO: int OmReaderDataBlockSeek(IntPtr reader, int dataBlockNumber);
// TODO: int OmReaderNextBlock(IntPtr reader);
// TODO: IntPtr OmReaderBuffer(IntPtr reader);
// TODO: uint OmReaderTimestamp(IntPtr reader, int index, out ushort fractional);
// TODO: int OmReaderGetValue(IntPtr reader, OM_READER_VALUE_TYPE valueType);
// TODO: OM_READER_HEADER_PACKET OmReaderRawHeaderPacket(IntPtr reader);
// TODO: OM_READER_DATA_PACKET OmReaderRawDataPacket(IntPtr reader);
// TODO: void OmReaderClose(IntPtr reader);
  
  
#ifdef __cplusplus
}
#endif 

