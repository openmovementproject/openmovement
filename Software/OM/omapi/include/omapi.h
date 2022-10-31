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



/** @file
 *  @ingroup   API
 *  @brief     Open Movement API
 *  @author    Dan Jackson
 *  @version   1.8.0
 *  @date      2011-
 *  @copyright BSD 2-clause license. Copyright (c) 2009-, Newcastle University, UK. All rights reserved.
 *  @details
    Open Movement API header file.

    See the documentation:
        - \ref api_init
        - \ref device_status
        - \ref settings
        - \ref download
        - \ref return_codes
        - \ref datetime
        - \ref reader
 */


/** @mainpage Open Movement API
 *  @version   1.8.0
 *  @date      2011-
 *  @copyright BSD 2-clause license. Copyright (c) 2009-, Newcastle University, UK. All rights reserved.
 *  @details
        This document describes the Open Movement application programming interface (API).
        The API provides an interface to communicate with AX3 longitudinal movement data loggers and their data files.  

    @section contents Table of contents
        - @subpage introduction
        - @subpage getting_started
        - @subpage example_code
*/

/** @page introduction Introduction to the API
 *  @details
    The Open Movement application programming interface (API) provides an interface to communicate 
        with AX3 longitudinal movement data loggers.  The devices feature a state of the art MEMS 
        3-axis accelerometer, flash-based on-board memory, a real time quartz clock, temperature 
        sensor and light sensor. 

    The API is part of Open Movement, and is licensed under on Open Source license: the BSD 2-clause license, by Newcastle University.

    The AX3 device uses a standard USB connector and enumerates on PC when connected. 
    The device is a "composite USB device" consisting of a communications device class (CDC) and a mass storage device (MSD).  
    The CDC interface presents a virtual serial port for device query and configuration -- the configuration software sends 
    simple text configuration commands over this channel.  The MSD interface presents a general purpose disk drive and file 
    system, which includes the dataset file.  
    
    The API presents a consistent view of all connected devices, with set of operations for each.  
    This provides a common foundation for higher-level interactions, such as the Graphical User Interface.

    @section design_goals Design Goals
    The API has been designed to:
        - be a simple, plain C interface.
        - be cross-platform (targeting Windows, Mac OS, Linux).
        - be statically or dynamically linked to C or C++ code.
        - be used directly from C++, or a simple object-oriented wrapper class.
        - be readily used from other languages, e.g. a .NET wrapper class through p/invoke; JNI for Java.
        - be thread-safe, in that it anticipates being called from multiple threads, and internally handles the required mutual exclusion.
        - handle errors via user-checked return values (it is expected that wrapper libraries can choose raise exceptions as suitable for a language).

    @section requirements Requirements
    The API is platform-agnostic, but parts of the implementation require some platform-specific functionality. 
    The API currently works on Windows (XP, Vista, 7, 8, 10), Mac OS X, Linux.

    The core API is written in plain C, and is easily wrapped by C++ (i.e. by an object-oriented class), or, as it can be built as a DLL with
    undecorated exports, other languages such as .NET (through p/invoke of the DLL).  Wrappers also exist for Java and Node.js.

    @see \ref getting_started

*/

/** @page getting_started Getting Started with the API
 *  @details

    @section api API
    The API is described in 7 parts.  Presented here in a reasonable order for learning:
        -# \ref api_init       \copybrief api_init     
        -# \ref device_status  \copybrief device_status
        -# \ref settings       \copybrief settings     
        -# \ref data_download  \copybrief data_download     
        -# \ref return_codes   \copybrief return_codes 
        -# \ref datetime       \copybrief datetime     
        -# \ref reader         \copybrief reader       

    @see \ref example_code "Example code" that covers a range of uses of the API.

 */

/*
    @section example_code Example code
    Code examples are included that cover a range of uses of the API.
    See the following files in the "Examples" section:
        -# test     - A command-line tool to self-test all connected devices, and controls the LED to indicate any failures.
        -# deploy   - A command-line tool to batch setup of all fully-charged and cleared devices, includes delayed activation and time synchronization. 
        -# download - A command-line tool to download from all devices that containing data. 
        -# clear    - A command-line tool to clear all attached devices.
        -# convert  - A command-line tool to convert a specified binary data file to a CSV text file.

<!--
    A brief overview of each is included below.
    @see \ref introduction
    @see The API header file: omapi.h

    \section s_api_init API initialization and device discovery
    \em <b>See main article:</b> \ref api_init
    \copydoc api_init

    \section s_device_status Device status
    \em <b>See main article:</b> \ref device_status
    \copydoc device_status

    \section s_settings Device settings and metadata
    \em <b>See main article:</b> \ref settings
    \copydoc settings

    \section s_download Data download
    \em <b>See main article:</b> \ref data_download
    \copydoc data_download

    \section s_return_codes Return codes
    \em <b>See main article:</b> \ref return_codes
    \copydoc return_codes

    \section s_datetime Date/time type
    \em <b>See main article:</b> \ref datetime
    \copydoc datetime

    \section s_reader Binary data file reader
    \em <b>See main article:</b> \ref reader
    \copydoc reader
-->

*/

/** @page example_code Example Code
 *  @details
 *
 *  Code examples are included that cover a range of uses of the API.
 *  See the following example files:
 *      -# test.c     \copybrief test.c
 *      -# deploy.c   \copybrief deploy.c
 *      -# download.c \copybrief download.c
 *      -# clear.c    \copybrief clear.c
 *      -# convert.c  \copybrief convert.c
 *
 *  The examples are linked in main.c, a single command-line tool that runs the examples.
 *
 */


 /* Include guard */
 #ifndef OM_H
 #define OM_H

#include <stdlib.h>

/* Plain C for undecorated function names in DLL */
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @cond _
 * On Windows, if not statically linked, we define the Windows DLL import/export declaration
 */
#ifndef OM_EXPORT
    #if defined(_WIN32) && defined(_WINDLL)
	    #define OM_EXPORT __declspec(dllexport)         /**< Exporting to a DLL (must also ensure setup to be __cdecl). @hideinitializer */
    #elif defined(_WIN32) && defined(OMAPI_DYNLIB_IMPORT)
    	#define OM_EXPORT __declspec(dllimport)         /**< Importing from DLL. @hideinitializer */
    #else
        #define OM_EXPORT                               /**< A standard, static link. @hideinitializer */
    #endif
#endif
/**@endcond*/



/**
 * @defgroup api_init API initialization and device discovery
 * Functions to support API initialization and device discovery.
 * The API must be initialized before first use with OmStartup(), and shut down after use with OmShutdown().
 *
 * To be notified when a device is added or removed from the system, the user can register a 'callback' function with OmSetDeviceCallback().
 *
 * Alternatively, users may call OmGetDeviceIds() at any time to list all of the currently connected devices.
 *
 * @see Example code test.c, download.c, clear.c and deploy.c for examples of how to use the initialization and device discovery functions of the API.
 * @{
 */

/**
 * A numeric code for current API version defined in this header file.
 * @remark This can be used to detect a DLL version incompatibility in OmStartup().
 * @see OmStartup()
 */
#define OM_VERSION 108


/**
 * Initializes the Open Movement API.
 * Allocates required resources for the API, initializes its state, and initiates device discovery. 
 * @note OmStartup() Must be called before most other functions in the API.  
 *       Exceptions are: OmSetLogStream(), OmSetLogCallback(), OmErrorString(), OmSetDeviceCallback(), OmSetDownloadCallback(), and all \a OmReaderXXX() functions.
 * @note If the \a OmDeviceCallback has already been set, it will be called for all devices identified at start-up.
 * @note Once successfully initialized, the caller must call OmShutdown() when finished with the API.
 * @param version The version number of the API required (use OM_VERSION).
 * @return \a OM_OK if successful, error code otherwise.
 * @see OmShutdown()
 */
OM_EXPORT int OmStartup(int version);


/**
 * Shuts down the Open Movement API.
 * This call frees resources used by the API.
 * @note Any incomplete downloads will be cancelled (and, if specified, the \a OmDownloadCallback function will be called).
 * @note Once called, functions that require OmStartup() may not be called unless OmStartup() is successfully called again.
 * @return \a OM_OK if successful, an error code otherwise.
 * @see OmStartup()
 */
OM_EXPORT int OmShutdown(void);


/**
 * Sets the stream to use for log messages.
 * @note This can be called at any time.
 * @param fd The file descriptor to use. Specify -1 for none; fileno(stderr) for standard error; or fileno(fopen("log.txt", "wt")) for a file.
 * @return \a OM_OK if successful, an error code otherwise.
 * @since 1.2
 */
OM_EXPORT int OmSetLogStream(int fd);


/**
 * Log callback function type.
 * Called for API log messages.
 * Callback functions take a user-defined reference pointer, and a log message.
 * @see OmSetLogCallback
 * @since 1.2
 */
typedef void(*OmLogCallback)(void *, const char *);


/**
 * Sets the callback function that is called whenever an API log message is written.
 * @param logCallback The function to call when a log message is written, or \a NULL to remove the callback.
 * @param[in] reference  A user-defined reference to pass to the callback function (or \a NULL if unwanted).
 * @return \a OM_OK if successful, an error code otherwise.
 * @since 1.2
 */
OM_EXPORT int OmSetLogCallback(OmLogCallback logCallback, void *reference);


/**
 * Device states used in the OmDeviceCallback handler.
 * @see OmDeviceCallback, OmSetDeviceCallback
 */
typedef enum
{
    OM_DEVICE_REMOVED,                  /**< Device is being removed, or is already removed */
    OM_DEVICE_CONNECTED                 /**< Device has been connected */
} OM_DEVICE_STATUS;


/**
 * Device change callback function type.
 * Called for device addition or removal.
 * Callback functions take a user-defined reference pointer, a device ID, and a state code.
 * @see OmSetDeviceCallback
 */
typedef void(*OmDeviceCallback)(void *, int, OM_DEVICE_STATUS);


/**
 * Sets the callback function that is called whenever a device is added or removed.
 * @param deviceCallback The function to call when a device is added or removed, or \a NULL to remove the callback.
 * @param[in] reference  A user-defined reference to pass to the callback function (or \a NULL if unwanted).
 * @return \a OM_OK if successful, an error code otherwise.
 * @see download.c for an example.
 */
OM_EXPORT int OmSetDeviceCallback(OmDeviceCallback deviceCallback, void *reference);


/**
 * Obtains the device IDs of all connected devices.  
 * Call \code OmGetDeviceIds(NULL, 0) \endcode to get the number of devices connected.  
 * @param[out] deviceIds A pointer to a buffer used to return the device IDs.  May be \a NULL if \a maxDevices = 0.  
 * @param maxDevices The maximum number of device IDs the supplied buffer can store.  
 * @retval >=0 the actual number of devices present, which may be more or less than \a maxDevices passed.
 * @retval <0 An error code.
 * @see clear.c, deploy.c, test.c for examples of use.
 */
OM_EXPORT int OmGetDeviceIds(int *deviceIds, int maxDevices);


/**@}*/



/* A single integer value is used to pack the date/time for a simpler, cross-language, API. 
 * (Defined here before its documentation, grouped with the other time functions below)
 */
typedef unsigned long OM_DATETIME;



/**
 * @defgroup device_status Device status
 * Functions to support querying the device version, battery level, health and time, and setting of the time and LED indicator. 
 * 
 * The version of the firmware and hardware on the device, an internal self-test status, and Flash memory health can be queried with OmGetVersion(), OmSelfTest(), OmGetMemoryHealth().
 *
 * The devices incorporate a rechargeable battery, the charge level and battery health can be determined with OmGetBatteryLevel() and OmGetBatteryHealth().
 *
 * The on-board real time clock can be accessed with the functions: OmGetTime(), OmSetTime().
 * 
 * The on-board triaxial accelerometer can be accessed to give an instantaneous reading with: OmGetAccelerometer().
 *
 * The device's 7-colour external LED indicator can be directly controlled with OmSetLed().
 *
 * The anti-tamper lock can be queried and controlled with OmIsLocked(), OmSetLock(), OmUnlock().
 *
 * Error-correcting code status can be set and queried with OmSetEcc() and OmGetEcc().
 *
 * Advanced commands can be sent with OmCommand().
 *
 * @see Example code test.c, for an example of how to use the device status functions of the API.
 * @{
 */

/**
 * Returns the firmware and hardware versions of the specified device.
 * @param deviceId Identifier of the device.
 * @param[out] firmwareVersion Pointer to a value to hold the firmware version, or \a NULL if not required.
 * @param[out] hardwareVersion Pointer to a value to hold the hardware version, or \a NULL if not required.
 * @return \a OM_OK if successful, an error code otherwise.
 */
OM_EXPORT int OmGetVersion(int deviceId, int *firmwareVersion, int *hardwareVersion);


/**
* Return the full USB serial string identity for the specified device.
* @param deviceId Identifier of the device.
* @param[out] serialBuffer A buffer to receive the string of the device'sUSB serial string identity (of size OM_MAX_PATH).
* @since 1.8
*/
OM_EXPORT int OmGetDeviceSerial(int deviceId, char *serialBuffer);


/**
* Return the path to specified device's communication port.
* @param deviceId Identifier of the device.
* @param[out] portBuffer A buffer to receive the path to the device's communication port (of size OM_MAX_PATH).
* @see OmGetDevicePath()
* @since 1.7
*/
OM_EXPORT int OmGetDevicePort(int deviceId, char *portBuffer);


/**
* Return the path to specified device's filesystem.
* @param deviceId Identifier of the device.
* @param[out] pathBuffer A buffer to receive the path to the device's filsystem (of size OM_MAX_PATH).
* @see OmGetDataFilename()
* @since 1.7
*/
OM_EXPORT int OmGetDevicePath(int deviceId, char *pathBuffer);


/**
 * Queries the specified device for the current battery charging level.
 * @param deviceId Identifier of the device.
 * @return A non-negative value indicates the battery percentage level (0-99% = charging, 100% = fully charged).
 *         A negative value indicates an error code for an issue within the API.
 */
OM_EXPORT int OmGetBatteryLevel(int deviceId);


/**
 * Performs a firmware-specific self-test on the specified device (e.g. test peripherals).
 * @remark A positive, non-zero self-test diagnostic code should be treated as an opaque, firmware-specific value indicating failure.
 * @param deviceId Identifier of the device.
 * @return \a OM_OK if a the device successfully completed its self-test.
 *         A positive value indicates the device failed its self-test with the returned diagnostic code. 
 *         A negative value indicates an error code for an issue within the API.
 */
OM_EXPORT int OmSelfTest(int deviceId);


/**
 * Determine the health of the NAND flash memory on the specified device.
 * The device incorporates a wear-levelling 'Flash Translation Layer', error-correcting codes (ECC), and
 *   data checksums to minimize any data loss.
 * To do this, the device must maintain a set of spare memory blocks on each memory 'plane'.  
 * This call retrieves the number of spare blocks on the NAND memory on the plane with the least spare blocks.
 * @param deviceId Identifier of the device.
 * @return A non-negative value indicates the health of the NAND memory (a larger number is better, 0 indicates the device is unusable).
 *         A negative value indicates an error code for an issue within the API.
 */
OM_EXPORT int OmGetMemoryHealth(int deviceId);

#define OM_MEMORY_HEALTH_ERROR   1  /**< Threshold at or below which the OmGetMemoryHealth() result should be treated as a failure. @since 1.2 */
#define OM_MEMORY_HEALTH_WARNING 8  /**< Threshold at or below which the OmGetMemoryHealth() result should be treated as a warning. @since 1.2 */


/**
 * Determine the health of the battery on the specified device.  
 * The device maintains an internal count of significant recharge cycles.  
 * @remark The device is rated to preserve 90% of its capacity (sufficient for 14-day data collection) after 500 recharge cycles.  
 * @param deviceId Identifier of the device.
 * @return A non-negative value indicates the number of significant recharge cycles of the battery (a smaller number is better).
 *         A negative value indicates an error code for an issue within the API.
 */
OM_EXPORT int OmGetBatteryHealth(int deviceId);


/**
 * Gets the specified device's current accelerometer values.
 * A sample consists of three consecutive signed 16-bit numbers (representing the X-, Y- and Z-axes), with units of 1/256 g. 
 * @param deviceId Identifier of the device.
 * @param[out] x Pointer to receive the x-axis value.
 * @param[out] y Pointer to receive the y-axis value.
 * @param[out] z Pointer to receive the z-axis value.
 * @return \a OM_OK if successful, an error code otherwise.
 */
OM_EXPORT int OmGetAccelerometer(int deviceId, int *x, int *y, int *z);


/**
 * Queries the specified device's internal real time clock.
 * @param deviceId Identifier of the device.
 * @param[out] time Pointer to a value to receive the current date/time.
 * @return \a OM_OK if successful, an error code otherwise.
 */
OM_EXPORT int OmGetTime(int deviceId, OM_DATETIME *time);


/**
 * Sets the specified device's internal real time clock.
 * @param deviceId Identifier of the device.
 * @param time the date/time to set.
 * @return \a OM_OK if successful, an error code otherwise.
 */
OM_EXPORT int OmSetTime(int deviceId, OM_DATETIME time);


/**
 * Enumeration of LED colours.
 * @note In the \a OM_LED_AUTO setting, when connected, the device fades from off to yellow when charging; 
 *       and from off to white when charged.  The device will fade to and from red instead of off when the 
 *       computer has recently written data to the mass storage drive (but this should not typically be observed through API access).
 * @remark Values are from the bit pattern: 0b00000RGB.
 * @see OmSetLed()
 */
typedef enum
{
    OM_LED_AUTO = -1,           /**< Automatic device-controlled LED to indicate state (default). */
    OM_LED_OFF = 0,             /**< rgb(0,0,0) Off     */
    OM_LED_BLUE = 1,            /**< rgb(0,0,1) Blue    */
    OM_LED_GREEN = 2,           /**< rgb(0,1,0) Green   */
    OM_LED_CYAN = 3,            /**< rgb(0,1,1) Cyan    */
    OM_LED_RED = 4,             /**< rgb(1,0,0) Red     */
    OM_LED_MAGENTA = 5,         /**< rgb(1,0,1) Magenta */
    OM_LED_YELLOW = 6,          /**< rgb(1,1,0) Yellow  */
    OM_LED_WHITE = 7            /**< rgb(1,1,1) White   */
} OM_LED_STATE;


/**
 * Sets the specified device's LED colour.
 * @remark The default LED status, \a OM_LED_AUTO, allows the device to control its LED to indicate its state. 
 * @param deviceId Identifier of the device.
 * @param ledState LED state.
 * @return \a OM_OK if successful, an error code otherwise.
 */
OM_EXPORT int OmSetLed(int deviceId, OM_LED_STATE ledState);


/**
 * Checks whether the device is currently locked.
 * @param deviceId Identifier of the device.
 * @param[out] hasLockCode Pointer to a value that receives a flag indicating whether the device has a non-zero lock code and will be locked whenever connected (OM_FALSE or OM_TRUE).
 * @return The current lock state: \a OM_FALSE if unlocked, \a OM_TRUE if locked, an error code otherwise.
 * @since 1.2
 */
OM_EXPORT int OmIsLocked(int deviceId, int *hasLockCode);


/**
 * Sets the lock code for the device when it is connected.
 * The device will not be locked immediately, only when next connected.
 * @param deviceId Identifier of the device.
 * @param code Lock code to use. 0 = no lock, 0xffff is reserved for internal use.
 * @return The 'initial lock' state: \a OM_FALSE if unlocked, \a OM_TRUE if locked.  Or an error code otherwise.
 * @since 1.2
 */
OM_EXPORT int OmSetLock(int deviceId, unsigned short code);


/**
 * Sets the lock code for the device when it is connected.
 * The device will not be locked immediately, only when next connected.
 * @param deviceId Identifier of the device.
 * @param code Unlock code to use (0 is reserved to indicate no lock code, 0xffff is reserved for internal use).
 * @return The current lock state: \a OM_FALSE if unlocked, \a OM_TRUE if the unlock code was not accepted, an error code otherwise.
 * @since 1.2
 */
OM_EXPORT int OmUnlock(int deviceId, unsigned short code);


/**
 * Sets the error-correcting code flag for the specified device.
 * @param deviceId Identifier of the device.
 * @param state \a OM_TRUE to enable ECC, \a OM_FALSE to disable ECC.
 * @return \a OM_OK if successful, an error code otherwise.
 * @since 1.2
 */
OM_EXPORT int OmSetEcc(int deviceId, int state);


/**
 * Gets the error-correcting code flag for the specified device.
 * @param deviceId Identifier of the device.
 * @return \a OM_TRUE if ECC enabled, \a OM_FALSE if ECC disabled, an error code otherwise.
 * @since 1.2
 */
OM_EXPORT int OmGetEcc(int deviceId);


/**
 * Issues a direct command over the CDC port for a particular device.
 * Waits for a line with the specified response (until the timeout is hit).
 * If the response is found, and parsing is required (parseParts != NULL), it parses the response (at '=' and ',' places) up to 'parseMax' token.
 * @note This method is not generally recommended -- incorrect results could lead to unspecified behaviour.
 * @param deviceId Identifier of the device.
 * @param command The command string to send (typically followed with CRLF).
 * @param[out] buffer A buffer to hold the response.
 * @param bufferSize The size (in bytes) of the output buffer.
 * @param expected The expected response prefix, or \a NULL if not specified (command will timeout).
 * @param timeoutMs The time, in milliseconds, after which the command will time-out and return.
 * @param[out] parseParts A buffer to hold the parsed response, or \a NULL if parsing is not required.
 * @param parseMax The maximum number of entries the \a parsePoints buffer can hold.
 * @return The offset in the buffer of the start of the expected response, if found; or the offset in the buffer of the start of the error response string; otherwise the length of the unrecognized response.
 * @since 1.2
 */
OM_EXPORT int OmCommand(int deviceId, const char *command, char *buffer, size_t bufferSize, const char *expected, unsigned int timeoutMs, char **parseParts, int parseMax);


/**@}*/



/**
 * @defgroup settings Device settings and meta-data
 * Functions to support reading and writing device settings and other meta-data, including clearing the device data storage.
 *
 * The recording time can be set to a precise time window, see: OmGetDelays() and OmSetDelays().
 *
 * The devices can be assigned a unique 'session identifier' to be used for a data set, see: OmGetSessionId() and OmSetSessionId().
 *
 * User-defined data can be stored in a 'scratch buffer', see: OmGetMetadata() and OmSetMetadata().
 *
 * Calls to OmSetDelays(), OmSetSessionId() and OmSetMetadata() are written to not alter existing session data on the device; 
 * instead they modify a (volatile at disconnect) stored state on the device and only take full effect only when the data is 
 * committed with OmEraseDataAndCommit().  Note that queries consult the volatile storage, so the non-permanent values will be read
 * if they've not yet been committed.
 *
 * @see Example code clear.c and deploy.c, for examples of how to use the device settings and metadata functions of the API.
 * @{
 */

/**
 * Gets the specified device's delayed activation start and stop times.
 * @param deviceId Identifier of the device.
 * @param[out] startTime Pointer to a value to receive the start date/time.
 * @param[out] stopTime Pointer to a value to receive the stop date/time.
 * @return \a OM_OK if successful, an error code otherwise.
 */
OM_EXPORT int OmGetDelays(int deviceId, OM_DATETIME *startTime, OM_DATETIME *stopTime);


/**
 * Sets the specified device's delayed activation start and stop times to use for a new recording session.
 * To set the device to always record:
 *      \code OmSetDelays(deviceId, OM_DATETIME_ZERO, OM_DATETIME_INFINITE); \endcode
 * To set the device to never record:
 *      \code OmSetDelays(deviceId, OM_DATETIME_INFINITE, OM_DATETIME_INFINITE); \endcode
 * @note This API call does not alter existing session data on the device, and only takes full effect when OmEraseDataAndCommit() is called.
 * @remark When not connected, a device would stop recording, even within the recording time window, if either storage or battery were to be exhausted. 
 *         When not recording, devices keep themselves in a low power mode to conserve battery. 
 * @param deviceId Identifier of the device.
 * @param startTime The start date and time.
 * @param stopTime The stop date and time.
 * @return \a OM_OK if successful, an error code otherwise.
 * @see OmEraseDataAndCommit()
 */
OM_EXPORT int OmSetDelays(int deviceId, OM_DATETIME startTime, OM_DATETIME stopTime);


/**
 * Queries the specified device's session identifier.
 * @param deviceId Identifier of the device.
 * @param[out] sessionId Pointer to a value to receive the session ID.
 * @return \a OM_OK if successful, an error code otherwise.
 */
OM_EXPORT int OmGetSessionId(int deviceId, unsigned int *sessionId);


/**
 * Sets the specified device's session identifier to be used at the next recording session.
 * @note This API call does not alter existing session data on the device, and only takes full effect when OmEraseDataAndCommit() is called.
 * @param deviceId Identifier of the device.
 * @param sessionId A value to set as the session ID.
 * @return \a OM_OK if successful, an error code otherwise.
 * @see OmEraseDataAndCommit()
 */
OM_EXPORT int OmSetSessionId(int deviceId, unsigned int sessionId);


/**
 * The number of bytes of metadata scratch area on the device.
 * @internal The 448 bytes comes from the device's 14 chunks of 32 bytes each.
 */
#define OM_METADATA_SIZE 448


/**
 * Gets the contents of the specified device's metadata scratch buffer.
 * @remark A suggested decoding of name/value pairs that preserves characters outside the non-control-ASCII range is URL-decoding to UTF-8 strings.
 * @param deviceId Identifier of the device.
 * @param[out] metadata A pointer to a buffer to receive the metadata, must be at least (OM_METADATA_SIZE + 1) as the value will be null-terminated.
 * @return \a OM_OK if successful, an error code otherwise.
 */
OM_EXPORT int OmGetMetadata(int deviceId, char *metadata);


/**
 * Sets the specified device's metadata scratch buffer to be used for the next recording session.  
 * @note This API call does not alter existing session data on the device, and only takes full effect when OmEraseDataAndCommit() is called -- after which, all existing metadata on the device will be replaced.
 * @remark A suggested encoding of name/value pairs that preserves characters outside the non-control-ASCII range is URL-encoding from UTF-8 strings.
 * @param deviceId Identifier of the device.
 * @param[out] metadata A pointer to the metadata to store, or \a NULL if clearing the metadata.
 * @param size The size of the specified metaData.  Can be 0 to clear the metadata.  If larger than OM_METADATA_SIZE, additional bytes will be ignored.
 * @see OmEraseDataAndCommit()
 * @return \a OM_OK if successful, an error code otherwise.
 */
OM_EXPORT int OmSetMetadata(int deviceId, const char *metadata, int size);


/**
 * Returns the date and time that the specified device was last configured.
 * @param deviceId Identifier of the device.
 * @param[out] time Pointer to a value to receive the last configuration date and time.
 * @return \a OM_OK if successful, an error code otherwise.
 */
OM_EXPORT int OmGetLastConfigTime(int deviceId, OM_DATETIME *time);


/**
 * Erase levels for OmEraseDataAndCommit() function.
 * @see OmEraseDataAndCommit
 * @since 1.2
 */
typedef enum
{
    OM_ERASE_NONE = 0,                  /**< Data file not erased but metadata just updated (this is not recommended as it could lead to a data/metadata mismatch). */
    OM_ERASE_DELETE = 1,                /**< Data file is removed and a new one created with the current metadata. */
    OM_ERASE_QUICKFORMAT = 2,           /**< Device file-system is re-created and a new data file is created with the current metadata. */
    OM_ERASE_WIPE = 3                   /**< All blocks on the NAND flash memory are cleared, the file-system is cleanly re-created, and a new data file is created with the current metadata. */
} OM_ERASE_LEVEL;


/**
 * Erases the specified device storage and commits the metadata and settings.  
 * To fully wipe a device, the caller can execute:
 * \code
 *     OmSetSessionId(deviceId, 0); 
 *     OmSetMetadata(deviceId, NULL, 0); 
 *     OmSetDelays(deviceId, OM_DATETIME_INFINITE, OM_DATETIME_INFINITE); 
 *     OmSetAccelConfig(deviceId, OM_ACCEL_DEFAULT_RATE, OM_ACCEL_DEFAULT_RANGE);
 *     OmEraseDataAndCommit(deviceId, OM_ERASE_WIPE); 
 * \endcode
 * And similarly, to cleanly re-configure a device, the same sequence can be called with non-zero values. 
 * @note To modify the file system, the device prevents the computer from accessing the data store (by temporarily disconnecting it).
 *       Therefore, the operation will report failure if a download is in progress - the download must complete or first be cancelled with OmCancelDownload().
 * @param deviceId Identifier of the device.  
 * @param eraseLevel The erase level to use, one of: OM_ERASE_NONE, OM_ERASE_DELETE, OM_ERASE_QUICKFORMAT, OM_ERASE_WIPE
 * @return \a OM_OK if successful, an error code otherwise.
 * @since 1.2
 */
OM_EXPORT int OmEraseDataAndCommit(int deviceId, OM_ERASE_LEVEL eraseLevel);


/**
 * Macro to quick-format a device and commit the metadata and settings.
 * This provides backwards compatibility for older versions of OMAPI.
 * @note To modify the file system, the device prevents the computer from accessing the data store (by temporarily disconnecting it).
 * Therefore, the operation will report failure if a download is in progress - the download must complete or first be cancelled with OmCancelDownload();
 * @param _deviceId Identifier of the device.  
 * @return \a OM_OK if successful, an error code otherwise.
 * @see OmEraseDataAndCommit()
 */
#define OmClearDataAndCommit(_deviceId) OmEraseDataAndCommit((_deviceId), OM_ERASE_QUICKFORMAT)


/**
 * Macro to commit the metadata and settings without clearing.
 * This provides backwards compatibility for older versions of OMAPI.
 * This is not recommended as it changes the metadata of an existing recording.
 * @note To modify the file system, the device prevents the computer from accessing the data store (by temporarily disconnecting it).
 * Therefore, the operation will report failure if a download is in progress - the download must complete or first be cancelled with OmCancelDownload();
 * @param _deviceId Identifier of the device.  
 * @return \a OM_OK if successful, an error code otherwise.
 * @see OmEraseDataAndCommit()
 */
#define OmCommit(_deviceId) OmEraseDataAndCommit((_deviceId), OM_ERASE_NONE)


/**
 * Default accelerometer rate configuration is 100Hz
 * @see OmSetAccelConfig()
 * @since 1.3
 */
#define OM_ACCEL_DEFAULT_RATE 100


/**
 * Default accelerometer range configuration is +/- 8 G 
 * @see OmSetAccelConfig()
 * @since 1.3
 */
#define OM_ACCEL_DEFAULT_RANGE 8


/**
 * Queries the specified device's accelerometer configuration.
 * @param deviceId Identifier of the device.
 * @param[out] rate Pointer to a value to receive the sampling rate in Hz (6 [=6.25], 12 [=12.5], 25, 50, 100, 200, 400, 800, 1600, 3200). Overloaded since 1.8 with negative ranges being "low power" mode.
 * @param[out] range Pointer to a value to receive the sampling range in +/- G (2, 4, 8, 16). Overloaded since 1.8 with the bits 16-32 being the synchronous gyro range in degrees/sec (125, 250, 500, 1000, 2000) with compatable devices.
 * @return \a OM_OK if successful, an error code otherwise.
 * @see OmSetAccelConfig()
 * @since 1.3, 1.8 for overloaded low-power and synchronous gyro range.
 */
OM_EXPORT int OmGetAccelConfig(int deviceId, int *rate, int *range);


/**
 * Sets the specified device's accelerometer configuration to be used at the next recording session.
 * @note This API call does not alter the existing settings, and only takes full effect when OmEraseDataAndCommit() is called.
 * @param deviceId Identifier of the device.
 * @param rate Sampling rate value in Hz (6 [=6.25], 12 [=12.5], 25, 50, 100, 200, 400, 800, 1600, 3200).  Overloaded since 1.8 with negative ranges being "low power" mode.
 * @param range Sampling range value in +/- G (2, 4, 8, 16). Overloaded since 1.8 with the bits 16-32 being the synchronous gyro range in degrees/sec (125, 250, 500, 1000, 2000) with compatable devices.
 * @return \a OM_OK if successful, an error code otherwise.
 * @see OmGetAccelConfig()
 * @since 1.3, 1.8 for overloaded low-power and synchronous gyro range.
 */
OM_EXPORT int OmSetAccelConfig(int deviceId, int rate, int range);


/**
 * Queries the specified device's 'maximum sample' value.  This should always be set to 0.
 * @param deviceId Identifier of the device.
 * @param[out] value Pointer to a value to receive the max sample value.
 * @return \a OM_OK if successful, an error code otherwise.
 * @see OmSetMaxSamples()
 * @since 1.5
 */
OM_EXPORT int OmGetMaxSamples(int deviceId, int *value);


/**
 * Sets the specified device's 'maximum sample' value.  This should always be set to 0.
 * @note This API call does not alter the existing settings, and only takes full effect when OmEraseDataAndCommit() is called.
 * @param deviceId Identifier of the device.
 * @param value Value setting (should be 0)
 * @return \a OM_OK if successful, an error code otherwise.
 * @see OmGetMaxSamples()
 * @since 1.5
 */
OM_EXPORT int OmSetMaxSamples(int deviceId, int value);



/**@}*/


/**
 * @defgroup data_download Data download
 * Functions to provide simultaneous download from multiple devices.
 *
 * The size, time-range, and internal chunking of the data buffer on the device is determined with a call to OmGetDataRange().
 *
 * A background download of data from a device is initiated with a call to OmBeginDownloading(), and cancelled with OmCancelDownload().
 *
 * The user typically registers a 'callback' function, using OmDownloadCallback(), that will be 
 *    asynchronously called by the API with updated information about the download progress.
 *
 * Alternatively, OmQueryDownload() may be called to determine the download progress.
 *
 * Applications that wish to synchronously block until the download completes (or is cancelled or fails), can call OmWaitForDownload().
 *
 * @see Example code download.c, for an example of how to use the data download functions of the API.
 * @{
 */


/**
 * Macro for the maximum path length of a data filename.
 * @since 1.2 
 */
#define OM_MAX_PATH (256)


/**
 * Download states used in the OmDownloadCallback handler.
 * @see OmDownloadCallback, OmSetDownloadCallback
 */
typedef enum
{
    OM_DOWNLOAD_NONE,                   /**< No download information */
    OM_DOWNLOAD_ERROR,                  /**< Data download failed with an error (the value parameter to OmDownloadCallback indicates a diagnostic error code) */
    OM_DOWNLOAD_PROGRESS,               /**< Data download progress (the value parameter to OmDownloadCallback indicates progress percentage) */
    OM_DOWNLOAD_COMPLETE,               /**< Data download completed successfully */
    OM_DOWNLOAD_CANCELLED,              /**< Data download was cancelled cleanly */
} OM_DOWNLOAD_STATUS;


/**
 * Download update callback function type.
 * Called for download progress, completion, cancellation, or failure.
 * Callback functions take a user-defined reference pointer, a device ID, a status code, and a value.
 * @see OmSetDownloadCallback
 */
typedef void(*OmDownloadCallback)(void *, int, OM_DOWNLOAD_STATUS, int);


/**
 * Download chunk callback function type.
 * Called for download chunks.
 * Callback functions take a user-defined reference pointer, a device ID, a buffer pointer, a file position and a buffer size.
 * @see OmSetDownloadCallback
 * @since 1.4
 */
typedef void(*OmDownloadChunkCallback)(void *, int, void *, int, int);


/**
 * Sets the callback function that is called for download progress, completion, cancellation, or failure.
 * @param downloadCallback The function to call on download activity, or \a NULL to remove the callback.
 * @param[in] reference  A user-defined reference to pass to the callback function (or \a NULL if unwanted).
 * @return \a OM_OK if successful, an error code otherwise.
 */
OM_EXPORT int OmSetDownloadCallback(OmDownloadCallback downloadCallback, void *reference);


/**
 * Sets the chunk callback function that is called when a new download chunk is received.
 * @param downloadChunkCallback The function to call on download activity, or \a NULL to remove the callback.
 * @param[in] reference  A user-defined reference to pass to the callback function (or \a NULL if unwanted).
 * @return \a OM_OK if successful, an error code otherwise.
 * @since 1.4
 */
OM_EXPORT int OmSetDownloadChunkCallback(OmDownloadChunkCallback downloadChunkCallback, void *reference);


/**
 * Return the data file size of the specified device.
 * @param deviceId Identifier of the device.
 * @see OmBeginDownloading()
 * @since 1.4
 */
OM_EXPORT int OmGetDataFileSize(int deviceId);


/**
 * Return the path to the data file for the specified device.
 * @param deviceId Identifier of the device.
 * @param[out] filenameBuffer A buffer to receive the data filename (of size OM_MAX_PATH).
 * @see OmReaderOpen()
 * @since 1.2
 */
OM_EXPORT int OmGetDataFilename(int deviceId, char *filenameBuffer);


/**
 * Read the size, time-range, and internal chunking of the data buffer.
 * @param deviceId Identifier of the device.
 * @param[out] dataBlockSize A pointer to a value to receive the block size of the data (512 bytes), or \a NULL if not required.
 * @param[out] dataOffsetBlocks A pointer to a value to receive the offset of the data in blocks (same as the header size: 2 blocks), or \a NULL if not required.
 * @param[out] dataNumBlocks A pointer to a value to receive the size of the data buffer in blocks, or \a NULL if not required.
 * @param[out] startTime A pointer for the start time of the data, or \a NULL if not required.
 * @param[out] endTime A pointer for the end time of the data, or \a NULL if not required.
 * @return \a OM_OK if successful, an error code otherwise.
 */
OM_EXPORT int OmGetDataRange(int deviceId, int *dataBlockSize, int *dataOffsetBlocks, int *dataNumBlocks, OM_DATETIME *startTime, OM_DATETIME *endTime);


/**
 * Begin downloading the data from the current device.
 * Once the download has successfully started, the call returns immediately, and the download continues in another thread.
 * The user-specified download callback is called to notify the user about download progress, completion, cancellation, or failure.  
 * @param deviceId Identifier of the device.
 * @param dataOffsetBlocks Start offset of the download (in blocks), typically 0.
 * @param dataLengthBlocks Length of the data to download (in blocks), -1 = all.
 * @param destinationFile File to write to (will truncate any existing file).  
 *        If NULL, will read the file but not write to a file -- this can be used in 
 *        conjunction with the OmSetDownloadChunkCallback() so the application can stream the data to another location.
 * @return \a OM_OK if successfully started the download, an error code otherwise.
 * @see OmSetDownloadCallback(), OmSetDownloadChunkCallback(), OmWaitForDownload(), OmCancelDownload()
 */
OM_EXPORT int OmBeginDownloading(int deviceId, int dataOffsetBlocks, int dataLengthBlocks, const char *destinationFile);


/**
 * Begin downloading the data from the current device, passing an additional reference.
 * Once the download has successfully started, the call returns immediately, and the download continues in another thread.
 * The user-specified download callback is called to notify the user about download progress, completion, cancellation, or failure.  
 * @param deviceId Identifier of the device.
 * @param dataOffsetBlocks Start offset of the download (in blocks), typically 0.
 * @param dataLengthBlocks Length of the data to download (in blocks), -1 = all.
 * @param destinationFile File to write to (will truncate any existing file).  
 *        If NULL, will read the file but not write to a file -- this can be used in 
 *        conjunction with the OmSetDownloadChunkCallback() so the application can stream the data to another location.
 * @param reference An additional reference to pass to the download callbacks -- if NULL, the reference given when registering the callback will be used instead.
 * @return \a OM_OK if successfully started the download, an error code otherwise.
 * @see OmBeginDownloading(), OmSetDownloadCallback(), OmSetDownloadChunkCallback(), OmWaitForDownload(), OmCancelDownload()
 * @since 1.4
 */
OM_EXPORT int OmBeginDownloadingReference(int deviceId, int dataOffsetBlocks, int dataLengthBlocks, const char *destinationFile, void *reference);


/**
 * This function queries the status of the specified device's asynchronous download.
 * @remark Using the user-specified download callback would be sufficient for most applications.
 * @param deviceId Identifier of the device.
 * @param[out] downloadStatus Current status of the download (whether none, in-progress, completed, cancelled or failed).
 * @param[out] downloadValue A pointer to be used to store a value associated with the download status.
 *             If the \a downloadStatus is \a OM_DOWNLOAD_PROGRESS, this stores the percent progress. 
 *             If the \a downloadStatus is \a OM_DOWNLOAD_ERROR, this stores a diagnostic code.  
 *             Can be \a NULL if not required.
 * @return \a OM_OK if successfully obtained the download status, an error code otherwise.
 * @see OmSetDownloadCallback()
 */
OM_EXPORT int OmQueryDownload(int deviceId, OM_DOWNLOAD_STATUS *downloadStatus, int *downloadValue);


/**
 * This function waits for the specified device's asynchronous download to finish.
 * The call returns immediately if a download is not in progress, or will blocks and return when the download completes, is cancelled, or fails. 
 * @remark This call is useful for single-threaded, non-interactive applications. 
 * @remark The user-specified download callback, if specified, is still called (and is useful for progress reports).
 * @param deviceId Identifier of the device.
 * @param[out] downloadStatus Final status of the download (whether completed, cancelled or failed).
 * @param[out] downloadValue A pointer to be used to store a final value for the download - if the \a downloadStatus 
 *             is \a OM_DOWNLOAD_ERROR, this stores a diagnostic code.  Can be \a NULL if not required.
 * @return \a OM_OK if successfully waited for the download, an error code otherwise.
 * @see OmSetDownloadCallback(), OmBeginDownloading()
 */
OM_EXPORT int OmWaitForDownload(int deviceId, OM_DOWNLOAD_STATUS *downloadStatus, int *downloadValue);


/**
 * Cancel the current download of the data on the specified device.
 * @remark If the download is still in progress, this will cancel the download (which will trigger a call to the user's download callback, if specified).
 * @param deviceId Identifier of the device.
 * @return \a OM_OK if successfully cancelled the download, an error code otherwise.
 * @see OmSetDownloadCallback, OmBeginDownloading()
 */
OM_EXPORT int OmCancelDownload(int deviceId);

/**@}*/



/**
 * @defgroup return_codes Return codes
 * API return and error codes.
 * The return values to most API functions are plain \c int types. 
 *
 * When successful, the functions return zero or a positive integer result. 
 * When unsuccessful, the functions return a negative integer result. 
 *
 * OmErrorString() returns a string representation of an error code.
 * @remark An enumerated type is not used as the API return values because some functions return successfully with (non-negative) numeric results. 
 * @{
 */

#define OM_TRUE                     1       /**< Return code: 'True' boolean value. @since 1.2 */
#define OM_FALSE                    0       /**< Return code: 'False' boolean value. @since 1.2 */
#define OM_OK                       0       /**< Return code: Success. */
#define OM_E_FAIL                   -1      /**< Return code: Unspecified failure. An error occurred that didn't have a more specific category. */
#define OM_E_UNEXPECTED             -2      /**< Return code: Unexpected error. An error occurred which was not anticipated by the code. */
#define OM_E_NOT_VALID_STATE        -3      /**< Return code: API not in a valid state. For example, it is uninitialized, or an operation cannot be completed because a download is currently in progress. This error should be avoidable on the client side. */
#define OM_E_OUT_OF_MEMORY          -4      /**< Return code: Out of memory error. A memory allocation failed. */
#define OM_E_INVALID_ARG            -5      /**< Return code: Invalid argument error. For example, an out-of-range value. */
#define OM_E_POINTER                -6      /**< Return code: Invalid pointer. For example, a 'NULL pointer'. */
#define OM_E_NOT_IMPLEMENTED        -7      /**< Return code: Requested functionality not implemented, either at the API level on this platform or the firmware version on the device. */
#define OM_E_ABORT                  -8      /**< Return code: Operation was aborted. (Not used) */
#define OM_E_ACCESS_DENIED          -9      /**< Return code: Access was denied opening, reading from, or writing to a device.  This can be returned if the device has been removed, or through some permissions problem. This could also be returned if the specified device is currently busy executing a command called from another thread. If the caller attempts to run a function on a specific device whilst another function is being run on that same device, E_ACCESS_DENIED will be returned.  This is true if called from another thread or another process. */
#define OM_E_INVALID_DEVICE         -10     /**< Return code: Device identifier was invalid. For example, if the device has been removed. */
#define OM_E_UNEXPECTED_RESPONSE    -11     /**< Return code: Device response was not as expected. */
#define OM_E_LOCKED                 -12     /**< Return code: Device is locked and the requested operation cannot be performed until it is unlocked. */
#define OM_SUCCEEDED(value) ((value) >= 0)  /**< Macro to check the specified return value for success. @return true for a successful (non-negative) value, false otherwise. */
#define OM_FAILED(value) ((value) < 0)      /**< Macro to check the specified return value for failure. @return true for a failed (negative) value, false otherwise. */


/**
 * Returns an error string for the specified API return code.
 * @param status An API return value.
 * @return Error string for the specified status.
 */
OM_EXPORT const char *OmErrorString(int status);

/**@}*/



/**
 * @defgroup datetime Date/time type
 * A representation of a date and time used by the API functions.
 * A single integer value is used to pack the date/time for a simpler, cross-language, API. 
 *
 * An OM_DATETIME value can be created from date/time components using the OM_DATETIME_FROM_YMDHMS() macro.
 *
 * An OM_DATETIME value can be split back into date/time components using macros: 
 *   OM_DATETIME_YEAR(), OM_DATETIME_MONTH(), OM_DATETIME_DAY(), 
 *   OM_DATETIME_HOURS(), OM_DATETIME_MINUTES(), OM_DATETIME_SECONDS().
 *
 * @remark The date/time elements are packed into a single 32-bit value: (MSB) YYYYYYMM MMDDDDDh hhhhmmmm mmssssss (LSB).
 * @see Example code deploy.c, for an example of how to use the date/time functions of the API in conjunction with the C time functions.
 * @{
 */

/**
 * @typedef OM_DATETIME
 * A single integer value is used to pack the date/time for a simpler, cross-language, API.
 */
/*typedef unsigned long OM_DATETIME;*/  /* (defined earlier in the file) */

/** 
 * Macro to create a packed date/time value from components.
 * @hideinitializer
 */
#define OM_DATETIME_FROM_YMDHMS(year, month, day, hours, minutes, seconds) \
                ( (((OM_DATETIME)((year) % 100) & 0x3f) << 26) \
                | (((OM_DATETIME)(month)        & 0x0f) << 22) \
                | (((OM_DATETIME)(day)          & 0x1f) << 17) \
                | (((OM_DATETIME)(hours)        & 0x1f) << 12) \
                | (((OM_DATETIME)(minutes)      & 0x3f) <<  6) \
                | (((OM_DATETIME)(seconds)      & 0x3f)      ) \
                )
#define OM_DATETIME_YEAR(dateTime)    ((unsigned int)((unsigned char)(((dateTime) >> 26) & 0x3f)) + 2000) /**< Extract the year from a packed date/time value. @hideinitializer */
#define OM_DATETIME_MONTH(dateTime)   ((unsigned char)(((dateTime) >> 22) & 0x0f))  /**< Extract the month (1-12) from a packed date/time value. @hideinitializer */
#define OM_DATETIME_DAY(dateTime)     ((unsigned char)(((dateTime) >> 17) & 0x1f))  /**< Extract the day (1-31) from a packed date/time value. @hideinitializer */
#define OM_DATETIME_HOURS(dateTime)   ((unsigned char)(((dateTime) >> 12) & 0x1f))  /**< Extract the hours (0-23) from a packed date/time value. @hideinitializer */
#define OM_DATETIME_MINUTES(dateTime) ((unsigned char)(((dateTime) >>  6) & 0x3f))  /**< Extract the minutes (0-59) from a packed date/time value. @hideinitializer */
#define OM_DATETIME_SECONDS(dateTime) ((unsigned char)(((dateTime)      ) & 0x3f))  /**< Extract the seconds (0-59) from a packed date/time value. @hideinitializer */
#define OM_DATETIME_MIN_VALID OM_DATETIME_FROM_YMDHMS(2000,  1,  1,  0, 0,   0)     /**< The minimum valid date/time value. @hideinitializer */
#define OM_DATETIME_MAX_VALID OM_DATETIME_FROM_YMDHMS(2063, 12, 31, 23, 59, 59)     /**< The maximum valid date/time value. @hideinitializer */
#define OM_DATETIME_ZERO (0)                                                        /**< Special date/time value for "infinitely early". @see OmSetDelays() @hideinitializer */
#define OM_DATETIME_INFINITE ((OM_DATETIME)-1)                                      /**< Special date/time value for "infinitely late". @see OmSetDelays() @hideinitializer */

/** @private The size of a buffer to hold a string representation of an OM_DATETIME */
#define OM_DATETIME_BUFFER_SIZE (20)

/** @private Convert a date/time number from a string ("YYYY-MM-DD hh:mm:ss") */
OM_EXPORT OM_DATETIME OmDateTimeFromString(const char *value);

/** @private Convert a date/time number to a string ("YYYY-MM-DD hh:mm:ss") */
//#define OmDateTimeToString(value, buffer) (((value) < OM_DATETIME_MIN_VALID) ? "0" : (((value) > OM_DATETIME_MAX_VALID) ? "-1" : (sprintf(buffer, "%04u-%02u-%02u %02u:%02u:%02u", OM_DATETIME_YEAR(value), OM_DATETIME_MONTH(value), OM_DATETIME_DAY(value), OM_DATETIME_HOURS(value), OM_DATETIME_MINUTES(value), OM_DATETIME_SECONDS(value)), buffer)))
OM_EXPORT char *OmDateTimeToString(OM_DATETIME value, char *buffer);


/**@}*/



/** 
 * @defgroup reader Binary data file reader
 * Functions to read a binary data file.
 * 
 * A handle to a binary data file reader is obtained with OmReaderOpen() and closed with OmReaderClose().
 *
 * The file is read in blocks by repeatedly calling OmReaderNextBlock(), and the unpacked data is
 *   accessed through OmReaderBuffer(), and the precise sample timestamps are interpolated with OmReaderTimestamp().
 *
 * Advanced uses are supported with direct access to the buffer through OmReaderRawHeaderPacket() and OmReaderRawDataPacket().
 *
 * @see Example code convert.c, for an example of how to use the binary file reader functions of the API.
 * @{
 */

/**
 * Handle to a reader object. 
 * Obtained by calling OmReaderOpen(), and disposed of with OmReaderClose().
 * @see OmReaderOpen(), OmReaderClose()
 */
typedef void * OmReaderHandle;


/**
 * Macro for the maximum number of samples in a block.
 * @since 1.3.1
 */
#define OM_MAX_SAMPLES (120)


/**
 * Macro for the maximum number of bytes in a header packet.
 * @since 1.3.1
 */
#define OM_MAX_HEADER_SIZE (2 * 512)


/**
 * Macro for the maximum number of bytes in a data packet.
 * @since 1.3.1
 */
#define OM_MAX_DATA_SIZE (512)


/**
 * Opens a binary data file for reading.
 * Parses the file header and places the stream at the first block of data.
 * @param binaryFilename The file name of the binary file to open.
 * @see OmReaderNextBlock(), OmReaderClose()
 * @return If successful, a handle to the reader object, otherwise \a NULL
 */
OM_EXPORT OmReaderHandle OmReaderOpen(const char *binaryFilename);


/**
 * Read the size, time-range, and internal chunking of the binary file.
 * @param reader The handle to the reader.
 * @param[out] dataBlockSize A pointer to a value to receive the block size of the data (512 bytes), or \a NULL if not required.
 * @param[out] dataOffsetBlocks A pointer to a value to receive the offset of the data in blocks (same as the header size: 2 blocks), or \a NULL if not required.
 * @param[out] dataNumBlocks A pointer to a value to receive the size of the data buffer in blocks, or \a NULL if not required.
 * @param[out] startTime A pointer for the start time of the data, or \a NULL if not required.
 * @param[out] endTime A pointer for the end time of the data, or \a NULL if not required.
 * @return \a OM_OK if successful, an error code otherwise.
 */
OM_EXPORT int OmReaderDataRange(OmReaderHandle reader, int *dataBlockSize, int *dataOffsetBlocks, int *dataNumBlocks, OM_DATETIME *startTime, OM_DATETIME *endTime);


/**
 * Read the device id, session id and metadata of the binary file.
 * @param reader The handle to the reader.
 * @param[out] deviceId A pointer to a value to receive the device id, or \a NULL if not required.
 * @param[out] sessionId A pointer to a value to receive the session id, or \a NULL if not required.
 * @return A pointer to a value to receive the pointer to the metadata, or \a NULL if the reader handle is not valid.
 */
OM_EXPORT const char *OmReaderMetadata(OmReaderHandle reader, int *deviceId, unsigned int *sessionId);


/**
 * Return the current block index of the reader.
 * @param reader The handle to the reader.
 * @return If non-negative, the block position within the file, an error code otherwise.
 */
OM_EXPORT int OmReaderDataBlockPosition(OmReaderHandle reader);


/**
 * Seeks the file reader to the specified data block.
 * @param reader The handle to the reader.
 * @param dataBlockNumber If positive, the data block index from the start of the file (after any header blocks); if negative, the data block index from the end of the file.
 * @return \a OM_OK if successful, an error code otherwise.
 */
OM_EXPORT int OmReaderDataBlockSeek(OmReaderHandle reader, int dataBlockNumber);


/**
 * Reads the next block of data from the binary file.
 * @param reader The handle to the reader.
 * @retval >0 a positive number of samples in the buffer successfully read. 
 * @retval 0 this block is unreadable but additional blocks remain (call OmReaderNextBlock() again).
 * @retval -1, the end-of-file has been reached.
 * @retval Otherwise, an error code.
 * @see OmReaderBuffer(), OmReaderTimestamp()
 */
OM_EXPORT int OmReaderNextBlock(OmReaderHandle reader);


/**
 * Obtains a pointer to the buffer of samples read, and unpacked, by OmReaderNextBlock().
 * The number of samples available is specified by the return value of the previous OmReaderNextBlock() call.
 * Each sample consists of three consecutive signed 16-bit numbers (representing the X-, Y- and Z-axes), with units of 1/256 g. 
 * @note The buffer contents are only guaranteed until the next call to OmReaderNextBlock() or OmReaderClose().
 * @param reader The handle to the reader.
 * @return A pointer to the buffer of samples just read.
 * @see OmReaderTimestamp()
 */
OM_EXPORT short *OmReaderBuffer(OmReaderHandle reader);


/**
 * Determines the timestamp of the specified sample in the buffer read by OmReaderNextBlock().
 *
 * Actual readings from the real-time-clock are stored once per block (with precise offset information).
 * As the true sampling rate is subject to error (it is subject to the accelerometer device's internal sampling)
 *   this function must interpolate over these timestamps to produce the actual real-time value for each sample.
 *
 * @param reader The handle to the reader.
 * @param index The sample index, must be >= 0 and < the number of samples returned from OmReaderNextBlock().
 * @param[out] fractional A pointer to a value to hold the 1/65536th of a second fractional time offset, or \a NULL if not required.
 * @return The packed date/time value of the sample at the start of the buffer, or 0 if none (e.g. an invalid index).
 */
OM_EXPORT OM_DATETIME OmReaderTimestamp(OmReaderHandle reader, int index, unsigned short *fractional);


/**
 * Reader value indexes for OmReaderGetValue() function.
 * @see OmReaderGetValue
 * @since 1.3
 */
typedef enum
{
/** \cond */
    OM_VALUE_DEVICEID = 3, 
    OM_VALUE_SESSIONID = 4, 
    OM_VALUE_SEQUENCEID = 5, 
/** \endcond */
    OM_VALUE_LIGHT = 7,                 /**< Raw light sensor reading. */
/** \cond */
    OM_VALUE_TEMPERATURE = 8,           
    OM_VALUE_EVENTS = 9, 
    OM_VALUE_BATTERY = 10, 
    OM_VALUE_SAMPLERATE = 11,
    OM_VALUE_LIGHT_LOG10LUXTIMES10POWER3 = 107, /**< log10(lux) * 10^3   therefore   lux = pow(10.0, log10LuxTimes10Power3 / 1000.0) */
/** \endcond */
    OM_VALUE_TEMPERATURE_MC = 108,      /**< Temperature sensor reading in milli-centigrade */
/** \cond */
    OM_VALUE_BATTERY_MV = 110,
    OM_VALUE_BATTERY_PERCENT = 210,
/** \endcond */
	OM_VALUE_AXES = 12,                 /**< Number of axes per sample.  Synchronous axes are [GxGyGz]AxAyAz[[MxMyMz]], 3=A, 6=GA, 9=GAM */
	OM_VALUE_SCALE_ACCEL = 13,          /**< Scaling: number of units for 1g: CWA=256, AX6=2048 (+/-16g), 4096 (+/-8g), 8192 (+/-4g), 16384 (+/-2g) */
	OM_VALUE_SCALE_GYRO = 14,           /**< Scaling: number of degrees per second that (2^15=)32768 represents: AX6= 2000, 1000, 500, 250, 125, 0=off. */
	OM_VALUE_SCALE_MAG = 15,			/**< Scaling: number of units for 1uT: AX6=16 */
	OM_VALUE_ACCEL_AXIS = 16,			/**< Axis index for Accel-X (-Y and -Z follow), AX3=0, AX6(A)=0, AX6(GA/GAM)=3, not-present=-1 */
	OM_VALUE_GYRO_AXIS = 17,			/**< Axis index for Gyro-X (-Y and -Z follow), AX6(GA/GAM)=0, not-present=-1 */
	OM_VALUE_MAG_AXIS = 18,				/**< Axis index for Mag-X (-Y and -Z follow), AX6(GAM)=6, not-present=-1 */
} OM_READER_VALUE_TYPE;


/**
 * Returns a specific value type from the buffer read by OmReaderNextBlock().
 *
 * @param reader The handle to the reader.
 * @param valueType The value to return.
 * @return The raw value requested for the current block.
 * @see OM_READER_VALUE_TYPE
 * @since 1.3
 */
OM_EXPORT int OmReaderGetValue(OmReaderHandle reader, OM_READER_VALUE_TYPE valueType);


/**
 * Internal structure of a binary file header block.
 * This structure would not typically be used by an API client.
 */
#pragma pack(push, 1)
typedef struct
{
	unsigned short packetHeader;	    /**< @ 0 +2 ASCII "MD", little-endian (0x444D) */
	unsigned short packetLength;	    /**< @ 2 +2 Packet length (1020 bytes, with header (4) = 1024 bytes total) */
	unsigned char reserved1;            /**< @ 4 +1 (1 byte reserved) */
	unsigned short deviceId;            /**< @ 5 +2 Device identifier (low 16-bits) */
	unsigned int sessionId;             /**< @ 7 +4 Unique session identifier */
	unsigned short reserved2;           /**< @11 +2 (2 bytes reserved) */
	OM_DATETIME loggingStartTime;       /**< @13 +4 Start time for delayed logging */
	OM_DATETIME loggingEndTime;         /**< @17 +4 Stop time for delayed logging */
	unsigned int loggingCapacity;       /**< @21 +4 Preset maximum number of samples to collect, 0 = unlimited */
    unsigned char reserved3[11];        /**< @25 +11 (11 bytes reserved) */
	unsigned char samplingRate;		    /**< @36 +1 Sampling rate */
	unsigned int lastChangeTime;        /**< @37 +4 Last change metadata time */
	unsigned char firmwareRevision;		/**< @41 +1 Firmware revision number */
	signed short timeZone;				/**< @42 +2 Time Zone offset from UTC (in minutes), 0xffff = -1 = unknown */
    unsigned char reserved4[20];        /**< @44 +20 (20 bytes reserved) */
    unsigned char annotation[OM_METADATA_SIZE]; /**< @64 +448 Scratch buffer / meta-data (448 characters) */
    unsigned char reserved[512];        /**< @512 +512 Reserved for post-collection scratch buffer / meta-data (512 characters) */
} OM_READER_HEADER_PACKET;
#pragma pack(pop)


/**
 * Accesses the contents of a raw header packet.
 * The packet will have already been loaded by the call to OmReaderOpen(), and will remain valid until OmReaderClose() is called for the same reader handle.
 * This would not typically be used by an API client.
 * @param reader The handle to the reader.
 * @return A pointer to the bytes in the raw header packet.
 */
OM_EXPORT OM_READER_HEADER_PACKET *OmReaderRawHeaderPacket(OmReaderHandle reader);


/**
 * Internal structure of a binary file data block.
 * This structure would not typically be used by an API client.
 */
#pragma pack(push, 1)
typedef struct
{
	unsigned short packetHeader;	    /**< @ 0 +2  ASCII "AX", little-endian (0x5841) */
	unsigned short packetLength;	    /**< @ 2 +2  Packet length (508 bytes, with header (4) = 512 bytes total) */
	unsigned short deviceFractional;	/**< @ 4 +2  Top bit set: 15-bit fraction of a second for the time stamp, the timestampOffset was already adjusted to minimize this assuming ideal sample rate; Top bit clear: 15-bit device identifier, 0 = unknown; */
    unsigned int sessionId;			    /**< @ 6 +4  Unique session identifier, 0 = unknown */
    unsigned int sequenceId;		    /**< @10 +4  Sequence counter, each packet has a new number (reset if restarted) */
    OM_DATETIME timestamp;			    /**< @14 +4  Last reported RTC value, 0 = unknown */
	unsigned short light;			    /**< @18 +2  Last recorded light sensor value in raw units, 0 = none */
	unsigned short temperature;		    /**< @20 +2  Last recorded temperature sensor value in raw units, 0 = none */
	unsigned char  events;			    /**< @22 +1  Event flags since last packet, b0 = resume logging, b1 = single-tap event, b2 = double-tap event, b3-b7 = reserved for diagnostic use) */
	unsigned char  battery;			    /**< @23 +1  Last recorded battery level in raw units, 0 = unknown */
	unsigned char  sampleRate;		    /**< @24 +1  Sample rate code, (3200/(1<<(15-(rate & 0x0f)))) Hz */
	unsigned char  numAxesBPS;		    /**< @25 +1  0x32 (top nibble: number of axes = 3; bottom nibble: packing format - 2 = 3x 16-bit signed, 0 = 3x 10-bit signed + 2-bit exponent) */
	signed short   timestampOffset;	    /**< @26 +2  Relative sample index from the start of the buffer where the whole-second timestamp is valid */
    unsigned short sampleCount;		    /**< @28 +2  Number of accelerometer samples (80 or 120) */
	unsigned char rawSampleData[480];   /**< @30 +480 Raw sample data.  Each sample is either 3x 16-bit signed values (x, y, z) or one 32-bit packed value (The bits in bytes [3][2][1][0]: eezzzzzz zzzzyyyy yyyyyyxx xxxxxxxx, e = binary exponent, lsb on right) */
	unsigned short checksum;            /**< @510 +2 Checksum of packet (16-bit word-wise sum of the whole packet should be zero) */
} OM_READER_DATA_PACKET;
#pragma pack(pop)


/**
 * Accesses the contents of a raw data packet. 
 * This would not typically be used by an API client.
 * The packet is loaded by calls to OmReaderNextBlock(), and remains valid until the next call of OmReaderNextBlock() or OmReaderClose() for the same reader handle.
 * \note If OmReaderNextBlock() returns 0, the reader has detected a checksum failure in the buffers contents, and any data values should be treated with caution.
 * @param reader The handle to the reader.
 * @return A pointer to the bytes in the raw data packet, or \a NULL if none is available.
 */
OM_EXPORT OM_READER_DATA_PACKET *OmReaderRawDataPacket(OmReaderHandle reader);


/**
 * Closes the specified reader handle.
 * Frees any resources allocated to the reader.
 * @param reader The handle to the reader to close.
 * @see OmReaderOpen()
 */
OM_EXPORT void OmReaderClose(OmReaderHandle reader);

/**@}*/



#ifdef __cplusplus
}
#endif


#endif
