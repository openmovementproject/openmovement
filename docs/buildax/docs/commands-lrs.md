
[//]: # (Command summary for BuildAX LRS firmware version 1.2)

# Commands

This document summarizes the serial command set of the BuildAX-LRS. The serial 
command set is available through the remote `telnet` interface and through
the USB CDC (Communication Device Class) serial port. The command set and 
responses are subject to change as the open source BuildAX project expands. 

The command strings are followed by `[CR][LF]` (carriage return, line feed) 
in all cases; meaning that you should press the `Enter` key to send a command 
after typing it. 

_(On Unix platforms, only an `[LF]` is sent- this is detected and handled accordingly.)_

For information on how to access the text-mode command terminal, please see the
[Connection Guide](connecting.md).


## Settings

Settings commands are expressed in the form `setting.type.x=y` and are used by the
LRS to determine where to route messages. A 'message' in this context is a 
data packet sent by the environment sensors and received by the device, which 
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

The LRS will print a summary to confirm the command after it is typed.

It is possible to configure the LRS with options which are 
potentially useless- streaming data over Telnet in binary, for instance, 
would be undesirable in most situations. 

---
# BuildAX LRS Command List
## Basic commands

These commands are for the normal user and provide access to the devices usual functions.

Commands below are shown in the form: _command_, **summary**, `example` (and response).

---
### `help`

**Summary:** Requests LRS to print a summary of common commands

**Example:** `help`

````
	HELP: help|status|time|reset|mount|exit|save|setting
````

---
### `status`

**Summary:** Request a printout of current status and settings

**Example:** `status`

````
	BAX, HW:1.1, FW:1.1
	NAME: BAX-434C32
	Start time: 2014/09/03,14:57:01
	Time:       2014/09/03,15:26:43
	SysVolts: 4850mV
	VddVolts: 3321mV
	SysTemp: 39.5?C
	Settings: From eeprom

	Username:admin
	Password:password
	Si44 Radio:  On
	Subnet:4200
	Mask:FF00

	ETHERNET: Connected
	MAC:02:04:A3:43:4C:32
	IP: 10.67.34.50
	MSK:255:255:240:0

	USB:
	Mode: 0
	Setting: Turned on
	Status:  Streaming

	File: 
	Mode: 1
	Setting: Turned off
	Status:  Card present

	Telnet:
	Mode:0
	Setting: Turned on
	Status:  Streaming

	Udp:
	Mode: 0
	Setting: Turned on
	Status:  Enabled

	Websocket:
	Mode: 0
	Setting: Turned on
	Status:  Enabled

````


---
### `time`

**Summary:** Request a printout of or set the time

**Example:** `time` or `time=2014/08/14,10:48:54`

````
	Time: 2014/08/14,10:48:54
````


---
### `save`

**Summary:** Request all open filed to be saved. This ensures all the cluster chains are written to disk. Files in use will be re-opened.

**Example:** `save`

````
	Saved files
````


---
### `reset`

**Summary:** Request a reset. Unsaved settings are lost. Open files are closed in orderly fashion

**Example:** `reset`

````
	Resetting
````


---
### `exit`

**Summary:** Request to close the terminal. Not possible for USB connections

**Example:** `exit`

````
	Telnet session closed 
````
or 
````
	Cannot exit this Terminal
````


---
### `setting.save`

**Summary:** Saves the current settings to non-volatile memory. Without this command, the settings are lost on reset or power loss.

**Example:** `setting.save`

````
	Saved settings
````


---
### `setting.username`

**Summary:** Change the device username

**Example:** `setting.username=newuser`

````
	Username:newuser
	Password:password
````


---
### `setting.password`

**Summary:** Change the device password

**Example:** `setting.username=newpassword`

````
	Username:admin
	Password:newpassword
````


---
### `setting.name`

**Summary:** Change the device name. This is also the NETBIOS name. NETBIOS names must be 15 or fewer characters.

**Example:** `setting.name=BAX_ROUTER6`

````
	Name: BAX_ROUTER6
````


---
### `setting.session`

**Summary:** Changes the text session number. The current file will be used until a save command is issued or the device autosaves. Permitted range is 0 to 65535.

**Example:** `setting.session=12345`

````
	Text session: 12345
````


---
### `setting.usb.stream`

**Summary:** Enable of disable the stream of the USB serial port. If turned off, ALL COMMUNICATIONS ARE SUSPENDED including the response to this setting

**Example:** `setting.usb.stream=1`

````
	USB:
	Mode: 0
	Setting: Turned on
	Status: Streaming
````


---
### `setting.usb.mode`

**Summary:** Change the stream output mode to text (0) or binary slip encoded (1)

**Example:** `setting.usb.mode=0`

````
	USB:
	Mode: 0
	Setting: Turned on
	Status: Streaming
````


---
### `setting.telnet.stream`

**Summary:** Enable or disable the stream of the telnet server (0 is off)

**Example:** `setting.telnet.stream=0`

````
	Telnet
	Mode:0
	Setting: Turned on
	Status: Enabled
````


---
### `setting.telnet.mode`

**Summary:** Change the stream mode of the telnet connection to text (0) or binary slip (1)

**Example:** `setting.telnet.mode=0`

````
	Telnet
	Mode:0
	Setting: Turned on
	Status: Enabled
````


---
### `setting.file.stream`

**Summary:** Enable or disable logging to file

**Example:** `setting.file.stream=0`

````
	File:
	Mode: 0
	Setting: Turned off
	Status: Card present
````


---
### `setting.file.mode`

**Summary:** Change the stream mode of the log file to text (0) or binary raw (1)

**Example:** `setting.file.mode=1`

````
	File:
	Mode: 1
	Setting: Turned off
	Status: Card present
````


---
### `setting.udp.stream`

**Summary:** Enable or disable the udp output (0=off)

**Example:** `setting.udp.stream=0`

````
	Udp:
	Mode: 0
	Setting: Turned on
	Status: Enabled
````


---
### `setting.udp.mode`

**Summary:** Change the stream mode of the telnet connection to text (0) or binary slip (1)

**Example:** `setting.udp.mode=0`

````
	Udp:
	Mode: 0
	Setting: Turned on
	Status: Enabled
````


---
### `setting.websocket.stream`

**Summary:** Enable or disable the websocket output (0=off)

**Example:** `setting.websocket.stream=0`

````
	Websocket:
	Mode: 0
	Setting: Turned on
	Status: Enabled
````

---
## Complex commands

These commands are complex in nature and should be used only by users who 
fully understand the devices functionality, or who are familiar with 
the source code.

---
### `mount`

**Summary:** Request to mount the memory card so it is available to the PC connection. If data logging is running, this is not safe. When logging the disk will appear write protected to the operating system.

**Example:** `mount=1` or `mount=0`

The first number represents the mount state, the second number is the write protect state.

````
	Mounted: 1, 0
````
or 
````
	Mounted: 0, 0
````


---
### `format`

**Summary:** Request an SD card format. ALL DATA WILL BE LOST.
The number on failure is the internal error code.

**Example:** `format`

````
	Formatted
````
or 
````
	Format failed: 00
````


---
### `hash`

**Summary:** Request the binary hash for opening a UDP session. Debug purposes only

**Example:** `hash`

````
	3C24B1A1C87BAA6290049ACCB387A556AA58C1299B38C099E57F5C68ECECC0EB6207B8A6309D99272E565BDFE80842B302E1936D4031ADFB26C3A77DAAA5395EA8AF66348949F2A4CB6CB15F31F97A5C0C3E
````


---
### `setting.bax.subnet`

**Summary:** Change the subnet of the radio, accepts hex input.

**Example:** `setting.bax.subnet=4201`

````
	Si44 Radio: On
	Subnet:4201
	Mask:FF00
````


---
### `setting.bax.mask`

**Summary:** Change the OR mask of the radio subnet, accepts hex input. The mask is used in the same way as an ethernet subnet mask upon the subnet value.

**Example:** `setting.bax.mask=FF01`

````
	Si44 Radio: On
	Subnet:4201
	Mask:FF01
````


---
### `mac!`

**Summary:** This is used to change the device mac address. Value is not latched unless saved and device is reset. IEEE format used.

**Example:** `mac!=12:34:56:78:90:ab`

````
	ETHERNET: Disconnected
	MAC:12:34:56:78:90:AB
	IP: 169.254.1.1
	MSK:255:255:0:0
````


---
### `bax_test`

**Summary:** Sets the bax radio to go into a pre-defined test mode. Reception is halted (0=test off, 1=CW carrier, 2=PN9 modulation pattern).

**Example:** `bax_test=1`

````
	Bax->cw
````




