
[//]: # (Router command list)

# Router Commands

All of the configuration actions which are available through the web interface
may also be used as text-interface commands. For information on how to access 
the text-mode command terminal, please see [Connecting to the Router](connecting.md).

Command summary for the router:

 Command | Action
 ------- | --------
 help    | Display this command list
 status  | Show current router configuration, including streaming settings and connectivity (IP address)
 time    | Display and set the RTC (Real Time Clock) on the router, used for time-stamping incoming data
 reset   | Reset the router (reload all settings from the SD card)
 mount   | Show SD card mount status (mounted/unmounted)
 format  | Format the SD card. **All data on the SD card will be permanently erased**
 save    | Write settings file out to disk
 setting | Change a setting. See [Settings](#settings) for more information
 exit    | Disconnect from Telnet (will not disconnect a serial connection)


Each command must be provided on a new line. To modify a value, provide the
command with an equals ('=') sign and the new value. For example:

````
	setting.file.stream=0
	mount=1
````

... will stop logging data to file, and mount the [USB Mass Storage](connecting.md#usb-mass-storage)
device so the SD card can be accessed by a PC over the USB connection.


## Settings

Settings are expressed in the format `setting.[type].[mode|stream]=[0|1]` and
can be concatenated with an ampersand (&) in the POST body to change multiple
settings simultaneously. Where mode=0, a stream will output in plaintext (else
1 for binary). Where stream=0, logging to this stream will be disabled (enabled
when 1).


