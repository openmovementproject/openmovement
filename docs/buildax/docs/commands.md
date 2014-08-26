
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

Settings are expressed in the form `setting.type.x=y` and are used by the
router to determine where to route messages. A 'messages' in this context is a 
data packet (sent by the environment sensors) received by the device, which 
must be routed to various outputs.

Each `setting.type` describes a data output, or "stream":

 Type      | Description
 --------- | -------------
 usb       | USB CDC (Communications Device Class) Serial connection 
 file      | Logging of data packets to a file on the SD card
 udp       | Network packet-forwarding over UDP (User Datagram Protocol)
 telnet    | Telnet protocol (virtual terminal connection) provided as a networked alternative to the serial interface
 websocket | Websocket packet-forwarding (used in the [Web Interface](user-guide.md#sensors) to provide real-time data)


Each stream has two configurable options: `stream` and `mode`.

+ **Stream**: Enable/disable the data output. Off: `stream=0`, On: `stream=1`
+ **Mode**: Output as plaintext (`mode=0`) or slip-encoded binary (`mode=1`)

The router will print a summary to confirm the command after you type it.

Here is an example with commands used to determine the state of the UDP stream,
enable it, and set it to binary mode:

````
>	setting.udp.stream
	Udp:
	Mode: 0
	Setting: Turned off

>	setting.udp.stream=1
	Udp:
	Mode: 0
	Setting: Turned on

>	setting.udp.mode=1
	Udp:
	Mode: 1
	Setting: Turned on
````

It is possible to configure the router with options which are 
potentially useless- streaming data over Telnet in binary, for instance, 
would be undesirable in most situations. 

