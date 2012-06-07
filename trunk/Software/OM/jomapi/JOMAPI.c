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

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    return JNI_VERSION_1_4;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved) {
    ;
}

/*
static JavaVM *gJavaVM = NULL;

//env->GetJavaVM(&gJavaVM);

static void myCallback()
{
    if (gJavaVM != NULL)
    {
        JNIEnv *env;
        gJavaVM->AttachCurrentThread((void**)&env, NULL);
        if (env != NULL)
        {
            jclass theClass = env->FindClass("JOMAPI");
            if (theClass == NULL)
            {
                jthrowable ex = env->ExceptionOccurred();
                if (ex)
                {
                    env->ExceptionDescribe();
                    env->ExceptionClear();
                }
            }
            else
            {
                jmethodID methodId = env->GetStaticMethodID(theClass, "callback", "(I)V");
                if (methodId == 0)
                {
                    jthrowable ex = env->ExceptionOccurred();
                    if (ex)
                    {
                        env->ExceptionDescribe();
                        env->ExceptionClear();
                    }
                }
                else
                {
                    env->CallStaticVoidMethod(theClass, methodId);
                }
            }
        }
    }
}
*/


JNIEXPORT jint JNICALL Java_JOMAPI_OmStartup
  (JNIEnv *env, jclass jObj, jint version)
  { return 0; }//OmStartup(version); }

JNIEXPORT jint JNICALL Java_JOMAPI_OmShutdown
  (JNIEnv *env, jclass jObj)
  { return OmShutdown(); }

JNIEXPORT jint JNICALL Java_JOMAPI_OmSetLogStream
  (JNIEnv *env, jclass jObj, jint fd)
  { return OmSetLogStream(fd); }

  
#ifdef __cplusplus
}
#endif 

