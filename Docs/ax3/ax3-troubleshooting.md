# AX3/AX6 Troubleshooting

## Connection Troubleshooting

1. **What is the connection/charging arrangement between the device and the computer?**

    Devices should only be connected directly to a computer or a USB hub that is externally powered with its own supply -- not a *passive* hub without its own power, and not left on a hub that is later removed from power. The devices can become discharged if left connected to a USB power source is not providing sufficient power, such as multiple devices connected to a passive USB hub, or a powered hub that is no longer powered, or left attached to a computer that goes to sleep and might provide less power.

2. **What exactly does the LED light do when you connect the device to a computer?**

    It should first flash through various colours for a short while, then settle to just a slowly pulsing yellow or white light.

    If it does not (e.g. no light, or goes to a solid yellow or green, or momentarily disappears in a way that is not part of a gradual fade off/on) there is likely a connection or communication problem, please check:

    * Is there a gentle click when you firmly insert the connector? (Without this, it may not make a proper connection to the data pins.)
   
    * Does trying another cable make a difference? (Cables/contacts can easily be broken or may have different tolerances.)
   
    * Does connecting to a different USB port (directly on the computer) make a difference? (Windows drivers are run for a specific device and port, using another port can sometimes fix a temporary glitch.)
   
    * Completely power-off/shutdown everything, wait a little, then restart -- does this make a difference? (This is not just standby/suspend/hibernate but a full power-off/restart, and any external USB hubs should also be disconnected from power to reset them. Although a cliché, this does often fix many temporary issues, e.g. if a USB port has gone "over current" it may have a temporary thermal fuse "blown" until power is disconnected).
   
    * By closely looking inside the device connector, is there any debris (such as grit or fluff) or grease or damage?  If so: does cleaning with a sharp blow or gentle use of a fine point make a difference? (But do not connect while there is moisture in the port, as this could corrode the contacts).
   
    * By gently moving the device around in the light, you should see the five shiny rectangular contacts on the central part of the connector (shorter side) -- is there is any sign of corrosion or grease preventing contact? (If the device has been externally connected to power while conductive liquid was in the port, such as non-pure water, it would be possible to cause corrosion on the connector). 

3. **Does trying the device and software on a completely different computer make a difference?**

    In particular: if it's not working on an organization-managed PC, does trying on a personal laptop make a difference?

4. **Does the device LED slowly pulse yellow/white for a sustained amount of time?** (Check over at least 15 seconds)

    If not, and everything else above was already checked, there may be an issue with it.  For further diagnosis, please note exactly what the device LED _is_ doing 
   
    _The following points are only likely to be worth considering if the device LED is behaving normally in this way_.

5. Under Windows Explorer's *This PC* where you see your computer's drives, **does the AX device appear as a drive?**

    * If so: you can manually copy off the data file `CWA-DATA.CWA`, if required.
   
    * If not: might your computer have any strict anti-virus software or security policies about access to removable USB drives? 

6. Open *Device Manager*, **does an entry appear listed under the *Ports* category for the device?** (Or sometimes under *Portable Devices*) 
   
    If not, then there may be a driver issue, please try the OmGUI installation again -- or the separate installer and driver files in [AX3-Driver-Win-5.zip](https://github.com/digitalinteraction/openmovement/raw/master/Downloads/AX3/AX3-Driver-Win-5.zip), ensuring it is as a user with administrative rights as this attempts to install a driver (not usually be needed if you're running Windows 10 or 11) -- did the installation (with driver) complete without any issues?

8. If the device appears as a drive and a "port", then the software should be able to communicate with it.  Please follow the *OmGui Software Troubleshooting* guide below.


## OmGui Software Troubleshooting

The standard connection software is [OmGui](https://github.com/digitalinteraction/openmovement/wiki/AX3-GUI#downloading-and-installing).

> **Note:** If the software has not been installed correctly, see [installation troubleshooting](#installation).
> Before beginning software troubleshooting, ensure that you have first completed any [connection troubleshooting](#connection-troubleshooting) that may be required.

1. **What version of OmGui software are you using?**

    Does installing [the current version](https://github.com/digitalinteraction/openmovement/blob/master/Downloads/AX3/AX3-GUI-revisions.md) (including any alpha/beta versions that may be available) make a difference? 

2. **Has the computer been restarted?**

    Although a bit of cliché, it is really worth restarting the computer and trying again, as this can clear any issues at the driver or operating system levels.

3. **Does trying the device and software on a completely different computer make a difference?**

    If it's not working on an organization-managed PC (perhaps from restrictive security software or settings), does trying on a personal laptop make a difference?

    Also note that *OmGui* is a Windows application and, although it may run under virtualization technology (such as *Parallels* under *macOS*), it is not tested for such configurations.

4. **Is the workspace on a network drive, or is there a restricted quota or limited drive storage space?**

    Some issues with transferring data may occur if the workspace is set to a network (shared) drive.  (This is often how virtualization programs such as *Parallels* map to the host computer's files).  It may be more reliable to use a local folder as a workspace, and to transfer the files off afterwards.
   
    In addition, be sure that the workspace folder you choose has sufficent free drive storage space (and is not restricted by a quota).

5. **Standard Log**

    Select *View*/*Log* (<kbd>Alt</kbd>+<kbd>V</kbd>, <kbd>L</kbd>), the log window will appear at the very bottom.  Resize it to be a little larger by dragging the bar just above it.  Perform the actions that you are troubleshooting.  If anything interesting appears in the log window, click in the Log window and select all of the text and copy it to the clipboard (<kbd>Ctrl</kbd>+<kbd>Home</kbd>, <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>End</kbd>, <kbd>Ctrl</kbd>+<kbd>C</kbd>).  Please paste the log output (<kbd>Ctrl</kbd>+<kbd>V</kbd>) into a document to save it.

    * If you see a message `EXCEPTION: System.Xml.XmlException:` <!-- ` Root element is missing.` -->, the file that stores the previous recording settings may have been corrupted.  You can try another working folder, or remove the configuration:

        a. Open the workspace folder you are using in *Explorer*: in the *Workspace* bar, press the open folder icon *Open working folder*

        b. Locate the file: `recordSetup.xml`
      
        c. Rename the file to something else (e.g. put a `_` character at the start)

        d. Close and restart OmGui.

    * If you see a message `EXCEPTION: System.Configuration.ConfigurationErrorsException: System.Configuration.ConfigurationErrorsException:
Configuration system failed to initialize`, the program configuration has become corrupted and can be reset:

        a. Make sure OmGui is not running

        b. Open the *Start*/*Run* window (<kbd>Windows</kbd>+<kbd>R</kbd>)

        c. Copy or type (followed by <kbd>Enter</kbd>): %LOCALAPPDATA%

        d. Single-click to select a folder in there named: `Newcastle_University,_UK`

        e. Rename that folder (<kbd>F2</kbd>) to something else (e.g. put a `_` character at the start)

        f. Now restart OmGui

6. **Detailed log?**

    If the above suggestions have not resolved the issue, please obtain an [OmGui Detailed Log](#omgui-detailed-log) as described in the next section.

7. **Simplest recording**

    If the device is not making a recording as you expect, please try the simplest configuration to record *Immediately on Disconnect* at *100 Hz* and *+/- 8 *g**, disabled gyroscope (AX6 only), and select *Flash during recording*.  If the configuration is successful, remove the device for 15 seconds -- does the LED flash green, and is there any data on the device when you connect it afterwards?  

8. **Device log**

    If the device is not making a recording as you expect, you can [obtain a detailed log from the attached device](#device-log) by following the instructions below. 

10. **Resetting the device**

    (Advanced) If you are having trouble programming a device, you can [manually reset the device](#resetting-the-device) by following the instructions below. 


### Device Log

You can obtain a detailed log from the attached device, which gives a reason for any recent stops to the logging.  There are three methods that can be used to obtain this log.

**Method 1: Web Page** (full diagnostics report - recommended)

1. Open a browser that supports *Web Serial*, such as *Google Chrome* or *Edge*.
2. Visit the page: [AX Diagnostics](https://config.openmovement.dev/#diagnostics&nolog&noconfigure&title=AX+Diagnostics)
3. Ensure a single device is connected (wait around 10 seconds after connecting the device).
4. Click: *Connect serial device...* and choose the attached *AX* device.
5. Click: *Device Diagnostics* to generate the diagnostic report.
6. Click: *Download Report* to save a copy of the diagnostics report.

<details><summary><strong>Method 2: Log Utility</strong> (status and device log only)</summary>

1. Download the `.ZIP` archive: [AX3-Utils-Win-3.zip](https://raw.githubusercontent.com/digitalinteraction/openmovement/master/Downloads/AX3/AX3-Utils-Win-3.zip)
2. Open the downloaded `.ZIP` archive (e.g. open *File Explorer* and locate `Downloads`)
3. In the toolbar, select *Extract All* / *Extract*.
4. In the newly-created extracted folder, locate the file: `log.cmd`
5. Ensuring only a single device is attached, double-click `log.cmd` to run the tool.  If you are prompted with a warning, try *More Info* / *Open Anyway*.
7. Use the mouse to click and drag to select the output text, and press *Enter* to copy the text  to the clipboard.  There may be some unexpected letters or unusal symbols at the end of some of the lines, these can be ignored.  
8. Paste the text into a new text document to save it.  This should be a timestamped record of the device's "stop reasons".

</details>

<details><summary><strong>Method 3: Web-Based Terminal</strong> (device log only)</summary>

If you have problems using the `log.cmd` above, you could alternatively try:

1. Open a browser that supports *Web Serial*, such as *Google Chrome* or *Edge*.
2. Visit this page: [googlechromelabs.github.io/serial-terminal](https://googlechromelabs.github.io/serial-terminal/)
3. Ensure the single device is connected (wait around 10 seconds).
4. In the *Port* dropdown, change the serial port (to the one the device is attached to).
5. Click the *Connect* button
6. Click in the black terminal area of the page
7. Type the following, followed by the *Enter* key (note that you will not see anything appear until you press *Enter*):
   `LOG`
8. Select the output lines of text and press *Ctrl*+*C* to copy it to the clipboard.  There may be some unexpected letters or unusal symbols at the end of some of the lines, these can be ignored.  
9. Paste (*Ctrl*+*V*) the text into a new text document to save it.  This should be a timestamped record of the device's "stop reasons".

</details>


### OmGui Detailed Log

Please try the following to extract a detailed log from *OmGui* about what it can see of the device:

1. Ensure *OmGui* is initially closed and that no devices are attached.
   
2. Open the *Run* window (press <kbd>Windows</kbd>+<kbd>R</kbd>)
   
3. Copy the line below and paste it in the Run box and press Enter to start OmGui with more verbose log messages (this assumes OmGui was installed in the default location):

   ```cmd
   CMD /C "SET OMDEBUG=2 && START "" "%ProgramFiles(x86)%\Open Movement\OM GUI\OmGui.exe""
   ```
   
4. Use OmGui as before until the problem occurs.
   
5. Select *View*/*Log* (<kbd>Alt</kbd>+<kbd>V</kbd>, <kbd>L</kbd>), the log window will appear at the very bottom.  Resize it to be a little larger by dragging the bar just above it.

6. Now perform the actions that you are troubleshooting, for example, one or more of the steps:
   * Attach the device (wait around 10 seconds for it to fully connect)
   * Optional: Attempt to download data from the device
   * Optional: Attempt to configure the device
   
7. Click in the Log window and select all of the text and copy it to the clipboard (<kbd>Ctrl</kbd>+<kbd>Home</kbd>, <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>End</kbd>, <kbd>Ctrl</kbd>+<kbd>C</kbd>)
   
8. Please paste the log output (<kbd>Ctrl</kbd>+<kbd>V</kbd>) into a document to save it.

Checking the log output:

* Device ID consistency: If it contains any `LOG: - MISMATCH:` lines, please see the next section to [device ID consistency](#device-id-consistency).

* If it shows an issue with a configuration file (in particular, if you are having an issue when trying to configure a device), see above section: *Standard Log*.


## Device ID Consistency

This section is primarily for if you receive an error: *The correct download file name cannot be established (device identifier not verified)*.

> This message is given when the device is accessed through a communication channel, but the device ID through that channel does not match the ones obtained while accessing through a storage channel -- the software refuses to continue, to preserve data integrity.  This could be caused by manually changing the contents of a drive (such as the data file or volume label), manually changing the device ID, or communication problems.

1. If the device contains useful data, as an initial priority, you can try to download the data as described in [Filesystem or data problems](#filesystem-or-data-problems).

2. If every device you have tried has a similar issue, check that you are not running anti-virus/security software that could be interfering with device communication.  If in doubt, try the device on another computer without such software (e.g. a personal laptop).

3. You should fully shutdown then restart your operating system (not just sleep/hibernat and resume, but fully shutdown and turn-off, before then restarting).  This will be help ensure there is no persistent driver issue.

4. If problems persist, to help diagnose the issue, please obtain an [OmGUI Detailed Log](#omgui-detailed-log) as described above (if not already done so).  If the log entry contains `LOG: - MISMATCH:`, it indicates there is still an issue with device IDs not matching.  

5. To manually verify each source of the device's ID, with only a single connected device, please make a note of the following four numbers (these should be the same):

    1. *External ID:* The number written on the side of the device (AX3: the 4-6 digit number after the `17-` or `18-` prefix; AX6: 7-digit number that should start with a `60`).

    2. *USB ID:* In *Device Manager*, under *Ports*, and the *COM* device under that, right-click / *Properties* / *Details* / *Property: Parent* -- the number that appears as the *Value* after the first part of the address `USB\VID_04D8&PID_0057\#####_` (where `#####` is `CWA17` or `AX664`).

    3. *Filesystem ID:* Locate the disk drive that appears under *This PC* when you connected the device, right-click the drive and select *Properties*, the highlighted field, note the *Volume Label* after the `AX3_` or `AX6_` prefix.

    4. *Data-file ID:* The ID in the current data file on the device.  This is a bit difficult to extract!  Press <kbd>Windows</kbd>+<kbd>R</kbd> to open the *Run* box, and copy and paste the following command to open a window and display a number, being sure to replace `D:` with the current drive letter for your device from the previous step:

       ```cmd
       PowerShell -Command "& {$s=[System.IO.File]::OpenRead('D:\CWA-DATA.CWA');$b=New-Object byte[] 13;$c=$s.Read($b,0,13);$s.close();Write-Output(16777216*$b[12]+65536*$b[11]+256*$b[6]+$b[5]);[Console]::ReadKey()}"
       ```

6. If any of the ID numbers in the last step are inconsistent, or if you received a `LOG: - MISMATCH:` message from the detailed log, you have inconsistent device IDs.  You should follow the instructions for [resetting the device](#resetting-the-device) with a new device ID from the device's case (after any `17-` or `18-` prefix for AX3 devices).


## Resetting the device

**NOTE:** This step is for advanced use only, and should only be performed if necessary.

**IMPORTANT:** This will *reformat* the device, deleting any existing data on there.  Please be certain it does not have the only copy of any data you'd like to keep.  You can manually move off data from the drive by locating the device's drive letter in *File Explorer* and move the `CWA-DATA.CWA` file to a safe location (see: [Filesystem or data problems](#filesystem-or-data-problems) for additional information).

**Method 1: Web Page**

1. Open a browser that supports *Web Serial*, such as *Google Chrome* or *Edge*.
2. Visit the page: [AX Diagnostics](https://config.openmovement.dev/#diagnostics&nolog&noconfigure&title=AX+Diagnostics)
3. Ensure a single device is connected (wait around 10 seconds after connecting the device).
4. Click: *Connect serial device...* and choose the attached *AX* device.
5. Click: *Reset* and *OK*.
6. Only if you are resetting the device ID: enter the device ID as displayed on the device case (after any `17-` or `18-` prefix on AX3 devices).
7. Click: *OK*.
8. The device will be wiped and reset.


<details><summary><strong>Method 2: Utility Program</strong></summary>

1. Download the .ZIP file: [AX3-Bootloaders](https://github.com/digitalinteraction/openmovement/blob/master/Downloads/AX3/AX3-Bootloaders.zip?raw=true)

2. Open the .ZIP file and extract the program `HidBootLoader.exe`.

3. Ensure no devices are connected and that OmGui is NOT running.

4. Run (double-click) `HidBootLoader.exe`.

5. Check the *Port* field is clear (if not, make a note of what it says)

6. Connect the device that you’d like to reset, and which does not contain any data you need to keep (this procedure will wipe the drive), then wait a second or so.

7. If enabled, press the *Run* button and wait a couple of seconds or so.

8. Check the *Port* field now displays a `COM` port (if it had a value before, use the drop-down arrow if necessary to ensure that it now has a different value)

9. In the *Command* field, copy and paste one of the lines below.
  
    * **Not changing the device ID:** If you are just resetting the device state (and not the device ID):

       ```
       TIME 2020-01-01 00:00:00|FORMAT WC|LED 5
       ```

    * **Changing the device ID:** If you are also resetting the device ID (if it appears, from the above troubleshooting, that the device ID has somehow become incorrectly programmed):

       ```
       DEVICE=12345|TIME 2020-01-01 00:00:00|FORMAT WC|LED 5
       ```
           
       ...and you must change `12345` to match the ID number on the outside of the device case (after any "17-" or "18-" prefix).

10. Press *Send*

11. Wait several seconds while the device LED is red, it should eventually turn *Magenta* (a purple blue/red mixture).

12. Disconnect the device

13. Close the bootloader software

14. The device should now be in a reset state.

</details>

<details><summary><strong>Method 3: Web-Based Terminal</strong></summary>

If you have problems using the `HidBootLoader.exe` program above, you could alternatively try:

1. Open a browser that supports *Web Serial*, such as *Google Chrome* or *Edge*.
2. Visit this page: [googlechromelabs.github.io/serial-terminal](https://googlechromelabs.github.io/serial-terminal/)
3. Ensure the single device is connected (wait around 10 seconds).
4. In the *Port* dropdown, change the serial port (to the one the device is attached to).
5. Click the *Connect* button
6. Click in the black terminal area of the page
7. Type the following, followed by the <kbd>Enter</kbd> key (note that you will not see anything appear until you press <kbd>Enter</kbd> -- just repeat the whole line if you make any mistakes):
   ```
   ECHO 1
   ```
9. **Only if** you are changing the device ID, type the following then the <kbd>Enter</kbd> key, replacing `12345` with your device's ID as printed on the case (you can use <kbd>Backspace</kbd> to correct mistakes):
    ```
    DEVICE=12345
    ```
10. Type the following, pressing the <kbd>Enter</kbd> key at the end of each line (you can use <kbd>Backspace</kbd> to correct mistakes):
    ```
    TIME 2020-01-01 00:00:00
    FORMAT WC
    LED 5
    ```
11. Wait several seconds while the device LED is red, it should eventually turn *Magenta* (a purple blue/red mixture).
12. Disconnect the device
13. The device should now be in a reset state.

</details>


## Removing a Mount Point

**NOTE:** This step is for advanced use only, and unlikely to apply to your device and/or unlikely to work.

Perhaps an old *mount point* is interfering somehow - you could try clearing the mount point:
 
1. Make sure OmGui is not running, and the problematic device IS connected

2. Start an *elevated* command prompt (with administrative permission), *either* of these methods:

   * Press Start, type `Command`, right-click the *Command Prompt* search result and select *Run as Administrator*
   
   * Press <kbd>Windows</kbd>+<kbd>R</kbd>, type `cmd`, <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>Enter</kbd>, select *Yes*
   
3. Type (or copy/paste) followed by <kbd>Enter</kbd>:

   ```cmd
   cmd /k mountvol
   ```

4. The command may list the device's current volume, e.g.: `C:\Mount\AX3_#####\`

5. If so, type followed by <kbd>Enter</kbd>: (replace the path with whatever the previous command showed)

   ```cmd
   mountvol C:\Mount\AX3_#####\ /D 
   ```

6. Assuming no error message was shown, the mount point was removed: you can disconnect the device and close the window.


## Battery

### Charging Arrangement

Ensure devices are only charged by directly connecting to a computer or a USB hub that is always externally powered, and not connected through a "passive" hub without its own power, and also not left on a hub or computer that is later removed from power or allowed to sleep or hibernate.


### Battery Percentage Estimate

The battery percentage shown is only a rough estimate based on the voltage measurements and an idealized discharge curve - the actual discharge curve depends on the battery/age/components/tolerances, and is highly non-linear – so variation is expected from this estimate.

It quite normal for this value to initially jump when connected, to vary by a few percent once the battery is fully charged (i.e. it discharges slightly before recharging), or to estimate a maximum reading that falls short of a 100% estimate.  In all cases, the battery will be at its maximum capacity when connected to a sufficient power source for up to 2 hours.

The same estimates apply to battery discharge from a data file.  The data preview graph in OmGui allows you to select additional lines on the right-hand side to chart (you will need to resize the graph to access them all), and one of these is the estimated battery percentage.


### Battery Health

Devices have the following notice about maintaining battery health:

> Battery Conditioning: In order to protect the Lithium Ion battery in this product, devices should be stored in a fully charged state in low ambient temperatures. Devices in prolonged storage should be recharged to this level every three months.

Related to this points, there are two battery health messages in OmGUI:
 
* A "Caution: Device May Have Fully Discharged" message is based on a heuristic that the software notices a connected device had lost track of time, as this implies that the battery became fully discharged.  This does not necessarily mean there is a problem, but is primarily a reminder to explain that devices should not be left fully discharged for extended periods as lithium ion batteries could become damaged if they are stored completely depleted for a significant time - and that devices should be charged periodically to ensure this doesn't happen. 
 
* A "Warning: Device Possibly Damaged" message is also based on a heuristic, however, it is a generally reliable indicator that the device may be damaged.  It is given when a device appears to have been reset recently, which should only happen if the battery was fully discharged, and yet the battery is already reporting a high level of charge.  This situation has been observed if the battery has become damaged to the point of holding very little charge, but it might be possible that it could occur for other reasons, so the device should still [be tested](#test-recording).

The message should be cleared once the device's clock is successfully configured to the correct time, and the easiest way to do this is configure any recording with the device.  Note that the configuring software may also remember (while it is kept running) which devices caused the caution even if they're disconnected/reconnected, so you might also have to restart the software too if you're immediately plugging a device back in.  

When given a battery health warning, it is advisable to fully charge the device then run a [test recording](#test-recording).


### Battery Issues

Checklist:

1. Ensure that you have the correct [charging arrangement](#charging-arrangement), and that your device was fully charged shortly before use.

2. Note the [battery conditioning](#battery-health) information, and the battery health warnings that may be shown in *OmGui*.

3. Note that the battery percentage shown is [only a rough estimate](#battery-percentage-estimate).

4. Note the expected [maximum recording duration](https://github.com/digitalinteraction/openmovement/blob/master/Docs/ax3/ax3-faq.md#maximum-recording-duration) for different configurations, and that, as with any lithium ion battery, capacity will decrease over the lifetime of the device.

5. Check that your device was configured with the correct configuration.  In OmGui software, you can select *View* / *Device Properties* (to check the device) or *View* / *File Properties* (to check a file in the working folder).  For additional details, you can obtain a [device log](#device-log) and/or [.CWA file diagnostics](#cwa-file-diagnostics).

6. To get more detail about the battery level during captured data, the data preview graph in *OmGui* allows you to select additional lines on the right-hand side to chart.  You will need to vertically resize the graph to access them all.  One of these additional plots is the *estimated battery percentage*.  It may help look at the initial charge and how it has discharged over a recording.

7. To verify the performance of a particular device, a [test recording](#test-recording) is recommended.

### Test recording

To verify the performance of a particular device, a test recording should be made with a fully-charged device.  This should be multi-day, a three day test is recommended or, if larger, up to the duration that you'd typically want the devices to record for (taking into account the [maximum expected durations](https://github.com/digitalinteraction/openmovement/blob/master/Docs/ax3/ax3-faq.md#maximum-recording-duration)).  For example, a typical test might be 7 days at 100 Hz sampling (±8 𝑔 accelerometer range and, for the AX6, ±2000 °/s sensitivity for the gyroscope).  This can be a static recording (e.g. can be just on a desk or in a drawer).  The device's performance in a test recording will establish the performance expected for subsequent recordings.

#### Start a test recording

1. Open the [*OmGui* software](https://github.com/openmovementproject/openmovement/wiki/AX3-GUI).
2. Connect the single device you would like to test, wait a few seconds.
3. Select the device from the upper panel.
4. Wait until it is suitably charged.
5. Only if the device has existing data (the *Record...* button is not available) you may first press the [*Clear*](https://github.com/openmovementproject/openmovement/wiki/AX3-GUI#clear-any-existing-data) button to remove any existing, unwanted data.
6. Press the [*Record...*](https://github.com/openmovementproject/openmovement/wiki/AX3-GUI#configure-the-sensor-to-record) button.
7. Enter suitable configuration details, as above, for the recording settings.  Make a note of the settings you have chosen, and the *End Date/End Time* in particular.  
8. Disconnect the device to allow it to make the recording (from the *Start Date/Start Time* you configured).

#### Check a test recording

1. Wait until after the *End Date/End Time* you previously configured.
2. Open *OmGui* software
3. Connect a single device that has being tested.
4. Select the device in the upper panel
5. Use the *data preview graph* in the middle of the window to check the start and end times of the data to assess the device's performance against its configuration.  The date/time is shown as the pointer is moved over the graph.  The *Zoom* magnifier tool (left-click to zoom in, right-click to zoom out) can be used to change the scale of the data preview.
6. You may optionally want to *Clear* any unwanted data.


## Filesystem or data problems

If you receive a warning from the operating system about the filesystem (e.g. *Error Checking* / *Repair this drive* / *Do you want to scan and fix*) you should initially ignore the message -- do not allow the operating system to try to fix anything as this can cause problems.  You should attempt to continue as normal:

* **If the device contains useful data:** try to download the data as usual.  If this does not work, locate the device's drive (e.g. *File Explorer* / *This PC* / identify and open the device's drive), and try to directly copy off the `CWA-DATA.CWA` data file.  If you have any problems with copying off the file, it may still be possible to recover the data: try the [cwa-recover](https://github.com/digitalinteraction/cwa-recover/) process.  It would be useful to obtain a diagnostics report from the device: [Device Log - Method 1](#device-log).  

* **If the device does not contain useful data:** try to configure the device as normal.  If this doesn't work, follow the troubleshooting instructions above, up to [Resetting the Device](#resetting-the-device) if required.  It would be useful to obtain a diagnostics report from the device: [Device Log - Method 1](#device-log) -- before clearing the device.  

You should check that you do not have any other software unnecessarily writing to the removable drives, or interfering with their operation (e.g. potentially some antivirus/security software) that may cause filesystem corruption.

### Partially-downloaded data

`.cwa.part` files are partial `.cwa` files are created by *OmGui* as the data is being downloaded from the device.  When a download is completed successfully, the file is renamed to `.cwa`.  

If you have a `.cwa.part` file while not currently downloading data, it could be that the data transfer might have been interrupted or failed.  Retry the download in *OmGui*.  If this fails, check the *Log* window (opened via the *View* menu), to see if there are any messages.  A download could fail, for example, if the USB connection was interrupted, or because of filling the workspace disk drive capacity or quota (which may be reported in the *Download* status as `Error (0xFFFFFFF7)`, and you can choose another working folder, preferably on the local computer to prevent any issues with network drives).  If the problem persists, locate the device's drive (e.g. *File Explorer* / *This PC* / identify and open the device's drive), and directly copy the `CWA-DATA.CWA` data file to your working folder.  

If you have a `.cwa.part` file, but no longer have the original data on a device, you can rename a `.cwa.part` file to `.cwa`, and the data from an interrupted download will be indistinguishable from a recording that was interrupted prematurely. 

### `.cwa` File Diagnostics

To examine the metadata from a `.cwa` file:

1. In your browser, visit the page: [AX Diagnostics](https://config.openmovement.dev/#diagnostics&nolog&noconfigure&title=AX+Diagnostics)
2. Click: *File Diagnostics*.
3. Select your `.cwa` file and press *Open* to generate the diagnostic report.
4. Click: *Download Report* to save a copy of the diagnostics report.


## Installation

The standard software to configure AX devices is the Open Movement Graphical User Interface: *OmGUI* (for Windows).  

* [OmGUI Documentation](https://github.com/digitalinteraction/openmovement/wiki/AX3-GUI)

> **See also:** Software for [large-scale deployments](https://github.com/digitalinteraction/openmovement/blob/master/Docs/ax3/ax3-research.md#large-scale-deployments) and [Cross-platform software (including Mac and Linux)](https://github.com/digitalinteraction/openmovement/wiki/AX3-GUI#cross-platform-software-including-mac-and-linux).

To install *OmGui*:

1. Using an Internet browser on a *Windows* PC, visit the sofware download page:

    * [OmGui Software Download](https://github.com/digitalinteraction/openmovement/blob/master/Downloads/AX3/AX3-GUI-revisions.md)

2. Click the topmost link to the latest version listed for *AX3-GUI*, which will download a `.zip` file archive.

3. Once the download has finished, locate your `Downloads` folder and open (double-click) the downloaded `AX3-GUI-??.zip` file.

4. Inside the archive, locate and open (double-click) the `AX3-GUI-??.exe` installer file to start the installer.

5. If you are prompted with *Windows protected your PC* (or similar message), this is because the operating system is reluctant to run unrecognized executable code from the Internet.  To continue to the installer, click *More info* and *Run anyway*.

6. The installer needs to run with Administrator permissions, so you may be prompted with *User Account Control* - *Do you want to allow this app from an unknown publisher to make changes to your device?*  To continue to the installer, click *Yes*.

7. Proceed through the installer.  For the default installation options, click *Next* / *Next* / *Next*. 

8. You can choose to install the driver with the *Install AX3 Driver* option.  This may only be required on older versions of Windows.  Press *Install* to continue the instalation.

    * If you have installation issues related to enabling/installing *.NET 3.5*, see the next section: [.NET 3.5](#net-35)

    * If you had the *Install AX3 Driver* option selected, you may be prompted again with a *User Account Control* message (*Yes*), and the *AX3 Driver Setup Wizard* (*Next* / *Next* / *Install*).  You may be prompted with *Windows Security* - *Would you like to install this device software?* (*Install* / *Next* / *Finish*).

9. Click *Finish*.  If you had the *Launch OmGui* option selected, the *OmGUI* software should now run.  Otherwise, open the Windows Start Menu, *All apps* (or *All Programs*), *OmGui* folder, *OmGui* shortcut.  For any issues with the software after successful installation, see [OmGui Software Troubleshooting](#omgui-software-troubleshooting).


If you have installation issues with the above (e.g. if related to permissions, such as a user account without administrative rights) consider the "no installer" variants listed on the [OmGui Software Download](https://github.com/digitalinteraction/openmovement/blob/master/Downloads/AX3/AX3-GUI-revisions.md) page.  These packages are `.zip` archives containing executable content, so you may need to "download anyway" and/or "More Info/Run Anyway".  On older versions of Windows, you may also need to install the [AX Driver](https://github.com/digitalinteraction/openmovement/blob/master/Downloads/AX3/AX3-Driver-Win-5.zip) to use the devices.  You will also need to ensure the [.NET 3.5 component is enabled](#net-35).


### .NET 3.5

OmGui requires the *.NET 3.5* Windows component to be enabled on the system.  

If it is not already enabled, there is more information on installing and troubleshooting .NET 3.5 at: [Install the .NET Framework 3.5](https://learn.microsoft.com/en-us/dotnet/framework/install/dotnet-35-windows)

There are various ways to enable the *.NET 3.5* component:

1. Open *Windows Features* by pressing <kbd>Windows</kbd>+<kbd>R</kbd>, and entering: `appwiz.cpl` -- then click *Turn Windows features on or off* and select *.NET Framework 3.5 (includes .NET 2.0 and 3.0)*, and press *OK*.
2. Alternatively, a manual method would be to:
    * Open *Task Manager* (<kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>Esc</kbd>)
    * Select *(Run) New task* (under the *File* menu or on the toolbar)
    * Type: `DISM.EXE /Online /Add-Capability /CapabilityName:NetFx3`
    * Click *Create this task with administrative privileges*, and press *OK*.
3. Alternatively, through the online installer: https://www.microsoft.com/en-gb/download/details.aspx?id=21
4. Alternatively, through the offline installer: https://dotnet.microsoft.com/en-us/download/dotnet-framework/net35-sp1

If you receive error `0x800f0906`, `0x800f0907`, `0x800f081f`, or `0x800F0922`, see: [.NET Framework 3.5 installation errors](https://learn.microsoft.com/en-GB/troubleshoot/windows-client/application-management/dotnet-framework-35-installation-error)

In addition to the above information: if you get error `0x800f081f` try the *Manual* method above; if you get error `0x800f0906`, this may be related to your managed computer having a specific system update source.  This is an issue for your IT administrators.  If you have Administrator permissions, and are allowed to do so, you could change your system update source:

1. Press <kbd>Windows</kbd>+<kbd>R</kbd>, type: `gpedit.msc`
2. Select *Computer Configuration* / *Administrative Templates* / *System* / *Specify settings for optional component installation and component repair* / *Enabled* / *Contact Windows Update directly to download repair content instead of Windows Server Update Services (WSUS)*
3. Open *Task Manager* (<kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>Esc</kbd>)
4. Select *(Run) New task* (under the *File* menu or on the toolbar)
5. Type: `gpupdate /force`
6. Click *Create this task with administrative privileges*, and press *OK*.

