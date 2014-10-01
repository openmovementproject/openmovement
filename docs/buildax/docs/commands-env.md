
 [//]: # (Some of this document is duplicated from Developer guide.txt in BAX2.0 firmware repo)


# Commands

This section describes the commands which can be issued to a BuildAX ENV
sensor. For information on connecting to the ENV sensors, please refer to 
the [Connection Guide](connecting.md#connecting-to-a-buildax-env-sensor).

Command strings are followed by `[CR][LF]` (carriage return, line feed) 
in all cases; meaning that you should press the `Enter` key to send a command 
after typing it. 

After most commands complete, the sensor will print its new operating status,
and the list of basic commands. This will look like:

```
    Settings:
    Device:42609215
    Name:dig0001
    packet_header = 66
    device_group = 96
    device_id = 37397
    batt_min = 2200
    tx_power = 20
    tx_channel = 87
    pir_threshold = 20
    pir_suspect_val = 10
    pir_suspect_count = 8
    pir_suspect_release = 8
    pir_disarm_time = 24
    pir_led_time = 8
    sw_led_time = 8
    sensor_settle_time = 8
    credit_interval = 468
    max_pir_credits = 10
    max_switch_credits = 10
    sensor_interval = 2343
    num_tx_repeats = 1
    Commands:save | defaults | name | renew | reset | sample | stream | exit (esc)
```



---

# BuildAX ENV Command List
## Basic commands

The following commands may be sent over the UART connection to perform an
action on the device:

---
### `name`

**Summary:** Set the device name to transmit on pairing

**Example:** `name=dig-bax0001`

``` 
    Done.
```

---
### `save`

**Summary:** Save current settings to non-volatile memory

**Example:** `save`

```
    Saved.
```

---
### `defaults`

**Summary:** Reset all settings to defaults and save them

**Example:** `defaults`

```
    Done.
```


---
### `renew`

**Summary:** Reset all settings, id and encryption data

**Example:** `renew`

```
    Done.
```


---
### `reset`
**Summary:** Reset the device, unsaved changes discarded

**Example:** `reset`

```
    Startup.
    Initial battery: 1663mV
    Setting loaded ok.
```


---
### `sample`
**Summary:** Sample the sensors and print their values

**Example:** `sample`

```
    Sample:
    Temp:24.7
    Humidity:38.3, 302
    Light:145
    Batt:1663
    PIR:165
    SW:1
```


---
### `stream`
**Summary:** Stream the sensor values in the same format as above

**Example:** `stream`

```
    <Button/key exit>
    Sample:
    Temp:24.7
    Humidity:38.7, 316
    Light:145
    Batt:1663
    PIR:159
    SW:1
```


---
### `exit`
**Summary:** Exit command mode, use current (possibly unsaved) settings now

**Example:** `exit`

```
    Ok.

    Turn off
```


---

## Complex commands

These commands are complex in nature and should be used only by users who 
fully understand the devices functionality, or are familiar with the source code.


 Command                  | Description
 ------------------------ | --------------                                                                      
 `packet_header=66      ` | ASCII 'B' (0x42, 66) code to identify BAX packets                                   
 `device_group=162      ` | Integer form of bits 16 to 24 of identifier, used for subnet mask                   
 `device_id=62199       ` | Integer form of bottom 16 bits of address                                           
 `batt_min=2100         ` | Minimum battery state before sensor disables itself (millivolts mV)                            
 `tx_power=10           ` | Transmit power in dBm                                                               
 `pir_threshold=20      ` | High threshold for known movement                                                   
 `pir_suspect_val=6     ` | Low threshold for suspected movement                                                
 `pir_suspect_count=4   ` | Number of suspected movements to reach threshold and make event                     
 `pir_suspect_release=8 ` | Time in each suspect count persists for                                             &dagger;
 `pir_disarm_time=8     ` | Time during which the PIR sensor will not create a new event (if triggered)         &dagger;
 `pir_led_time=8        ` | Time for which the PIR led illuminates for                                          &dagger;
 `sw_led_time=8         ` | Time for which the magnetic switch led illuminated for                              &dagger;
 `sensor_settle_time=8  ` | Time after led illumination before continuing to monitor sensor events              &dagger;
 `credit_interval=234   ` | Time between granting event transmit credits                                        &dagger;
 `max_pir_credits=5     ` | Maximum number of PIR transmit credits                                              
 `max_switch_credits=5  ` | Maximum number of magnetic switch TX (transmit) credits                                        
 `sensor_interval=234   ` | Time between sampling and transmitting environment sensors                          &dagger;
 `num_tx_repeats=1      ` | Number of environment sensor packets to send (repeats)                              


<span class="alert alert-info"> 
**&dagger; Times** Times are in Ticks which are ~128 ms. To work out the ticks
required from a time _in seconds_, multiply by 1000 then divide by 128 
(rounding any decimal places to the nearest integer).
</span>

### Example Timings

The following values are multiples of 128ms ticks, rounded to the nearest integer.
Exact timing is not possible with extremely low-power hardware such as the ENV,
so there will be some drift over time. This also helps to prevent radio data packet
collisions in deployments with many sensors, as no two sensors will drift by the 
same amount.


 Time       | Value (ticks)
 ---------- | ----------------
 1 second   | 8
 5 seconds  | 39
 30 seconds | 234
 1 minute   | 469
 5 minutes  | 2344
 30 minutes | 14063


