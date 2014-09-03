
[//]: # (Developer API)

# BuildAX LRS Developer API

The BuildAX LRS can be programatically configured via a remote connection. API calls to the BuildAX LRS are made as RESTful HTTP requests. For you, 
this means that setting state on the LRS is performed via `POST` requests, 
and `GET` requests are used to retrieve data.

The examples in this document use the `wget` command to communicate with the 
web server on the BuildAX LRS. On Linux `wget` is usually installed by default,
but can be downloaded for other OSes (including MacOSX and Windows) from the 
[GNU website](https://www.gnu.org/software/wget/).

An equivalent software with much the same functionality as wget is `curl`, 
which may be used in preference to `wget`.


## Logging In

The Web API uses a unique session key (a _cookie_) to authenticate users. All 
API sessions start when this session key is requested by making a POST request
to the BuildAX LRS.

The user agent must first authenticate with the LRS and obtain a session
cookie before further REST API requests can be made:

Method | Resource       | Parameters | Response
------ | -------------- | ---------- | ----------
`POST` | /WWW/login.htm | user, pass | 303 redirect with SID= session cookie.


Example `wget` command (a backslash is used to separate the command onto 
multiple lines):

````
	wget --save-cookies cookies.txt \
		--delete-after \
		--no-http-keep-alive \
		--post-data 'user=admin&pass=password' \
		--keep-session-cookies \
		http://your-bax/WWW/login.htm
````

Equivalent `curl` command:

````
	curl --silent --output /dev/null \
        -c cookies.txt \
        --data 'user=admin&pass=password' \
        http://your-bax/WWW/login.htm
````

## Changing Settings

Settings are passed to the server in the same format as the text command mode.
Please refer to [commands](commands.md#settings) for a full description
of the settings commands which may be sent to the LRS. 

Settings can be concatenated with an ampersand (&) in the POST body to change 
multiple settings simultaneously. 


Method | Resource           | Parameters                                      | Response 
------ | ------------------ | ----------------------------------------------- | ----------
`POST` | /WWW/a\_settng.htm | Setting arguments in the format specified above | `200 OK` Set-Cookie: update=ok 


	wget --load-cookies cookies.txt \
		--post-data 'setting.usb.stream=0' \
		http://your-bax/WWW/a\_settng.htm

Valid settings types are: `file`, `usb`, `telnet`, and `udp`.


## Fetching Data

Data fetching is performed via GET request, so parameters are passed in the
URL.

	http://your-bax/fetch?type=BT&start=449622000&end=485434800

There are three parameters in a fetch request: a type, and the start/end range
specifiers.

Method | Resource | Valid Parameters | Response 
------ | -------- | ---------------- | ----------
`GET`  | `/fetch` | type, start, end | `200 OK` (and download) 


### Fetch Types

The **type** parameter may be specified as one of the following:

 Type | Fetch mode
 ---- | ------------
 BT   | Binary / Time range fetch
 TT   | Text / Time
 BF   | Binary / File
 TF   | Text / File
 BS   | Binary / Sample range _(note: text/sample range is not implemented)_
 LS   | Last N Samples _(note: specify an empty 'end' parameter with this request)_


### Fetch Parameters

The parameters **start** and **end** are always present, but their value
changes based on the type of request.

 Fetch Type        | Range specifiers
 ----------------- | ------------------
**Time Range**     | Start and end are specified in _seconds after the epoch 2000-01-01_ matching the RTC timestamp
**File Range**     | Start and end are data file numbers as stored on the external flash card storage
**Sample Range**   | Start and end are a range of packets numbers received from the sensors
**Last N Samples** | Start is the number of most recent samples. Specify end as an empty string.

### Time Format

To calculate the time range, take the Unix timestamp for the time required (a
standard time representation) and subtract the Unix timestamp for the date
`2000-01-01`. Eg:

````
	Current Unix Time – (2000-01-01 00:00) = BuildAX RTC Timestamp
	       1407940334 –      946684800     = 461255534
````

The reason for this difference is that the hardware real-time-clock (RTC) on
the BuildAX LRS works with an epoch of the year 2000 and a 32-bit timestamp,
meaning dates up to 2068 can be represented internally.

## Example `wget` commands

_Remember that you must first run the [Login](#logging-in) command to get a
session cookie!_

Get data file 0 from the SD card (no more than 5 files (10MB) may be requested in one download):

	wget --load-cookies cookies.txt \
		--no-http-keep-alive \
		--content-disposition \
		http://your-bax/fetch?type=BF\&start=0\&end=0

Get last 500 samples:

	wget --load-cookies cookies.txt \
		--no-http-keep-alive \
		--content-disposition \
		http://your-bax/fetch?type=LS&start=500&end=

Get data within a time range:

	wget --load-cookies cookies.txt \
		--no-http-keep-alive \
		--content-disposition \
		http://your-bax/fetch?type=BT\&start=449622000\&end=485434800


One possible 'gotcha' is that the ampersand (&) symbol has a special meaning
in most command terminals, and must be escaped with a backslash (as in the 
above examples) so that the command is not forked.
