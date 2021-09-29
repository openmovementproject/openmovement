# AX3/AX6 Troubleshooting

## Connection Troubleshooting

1. **What version of OmGui software are you using?**

   Older versions of OmGui will not work on devices with larger IDs.  If it is not recent (e.g. 1.0.0.43), does installing [the current version](https://github.com/digitalinteraction/openmovement/wiki/AX3-GUI#downloading-and-installing) make a difference? 

2. **What is the connection/charging arrangement between the device and the computer?**

   Devices should only be connected directly to a computer or a USB hub that is externally powered with its own supply -- not a *passive* hub without its own power, and not left on a hub that is later removed from power. The devices can become discharged if left connected to a USB power source is not providing sufficient power, such as multiple devices connected to a passive USB hub, or a powered hub that is no longer powered, or left attached to a computer that goes to sleep and might provide less power.

3. **What exactly does the LED light do when you connect the device to a computer?**

   It should first flash through various colours for a short while, then settle to just a slowly pulsing yellow or white light.

   If it does not (e.g. no light, or goes to a solid yellow or green, or momentarily disappears in a way that is not part of a gradual fade off/on) there is likely a connection or communication problem, please check:

   * Is there a gentle click when you firmly insert the connector? (Without this, it may not make a proper connection to the data pins.)
   
   * Does trying another cable make a difference? (Cables/contacts can easily be broken or may have different tolerances.)
   
   * Does connecting to a different USB port (directly on the computer) make a difference? (Windows drivers are run for a specific device and port, using another port can sometimes fix a temporary glitch.)
   
   * Completely power-off/shutdown everything, wait a little, then restart -- does this make a difference? (This is not just standby/suspend/hibernate but a full power-off/restart, and any external USB hubs should also be disconnected from power to reset them. Although a cliché, this does often fix many temporary issues, e.g. if a USB port has gone "over current" it may have a temporary thermal fuse "blown" until power is disconnected).
   
   * By closely looking inside the device connector, is there any debris (such as grit or fluff)?  If so: does removing with a sharp blow or gentle use of a fine point (but do not connect while there is moisture in the port) make a difference?
   
   * By gently moving the device around in the light, you should see the five shiny rectangular contacts on the central part of the connector (shorter side) -- is there is any sign of corrosion or grease preventing contact? (If the device has been externally connected to power while conductive liquid was in the port, such as non-pure water, it would be possible to cause corrosion on the connector). 

4. **Does trying the device and software on a completely different computer make a difference?**

   In particular: if it's not working on an organization-managed PC, does trying on a personal laptop make a difference?


*(The following will only apply once the device is connected and the LED pulses yellow/white.)*

5. Under Windows Explorer's *This PC* where you see your computer's drives, **does the AX device appear as a drive?**

   * If so: you can manually copy off the data file `CWA-DATA.CWA`, if required.
   
   * If not: might your computer have any strict anti-virus software or security policies about access to removable USB drives? 

6. Open *Device Manager*, **does an entry appear listed under the *Ports* category for the device?** (Or sometimes under *Portable Devices*) 
   
   If not, then there may be a driver issue, please try the OmGUI installation again, ensuring it is as a user with administrative rights as this attempts to install a driver (not usually be needed if you're running Windows 10) -- did the installation (with driver) complete without any issues?

7. **What does the OmGui Detailed log say?**

   Please obtain an *OmGui Detailed log* as described in the next section.


## OmGui Detailed log

Please try the following to extract a detailed log from *OmGui* about what it can see of the device:

1. Ensure *OmGui* is initially closed.
   
2. Open the *Run* window (press <kbd>Windows</kbd>+<kbd>R</kbd>)
   
3. Copy the line below and paste it in the Run box and press Enter to start OmGui with more verbose log messages (this assumes OmGui was installed in the default location):

   ```cmd
   CMD /C "SET OMDEBUG=2 && START "" "%ProgramFiles(x86)%\Open Movement\OM GUI\OmGui.exe""
   ```
   
4. Use OmGui as before until the problem occurs.
   
5. Open the View/Log (<kbd>Alt</kbd>+<kbd>V</kbd>, <kbd>L</kbd>) window at the very bottom, and resize it to be a little larger
   
6. Attach the device and wait around 10 seconds
   
7. Click in the Log window and select all of the text and copy it to the clipboard (<kbd>Ctrl</kbd>+<kbd>Home</kbd>, <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>End</kbd>, <kbd>Ctrl</kbd>+<kbd>C</kbd>)
   
8. Please paste the log output (<kbd>Ctrl</kbd>+<kbd>V</kbd>) in a new text file or document to save it.
   
If the log entry contains any `LOG: - MISMATCH:` lines, this indicating an issue with device ID consistency, please see the next section to manually verify the device ID.


## Manually checking device ID consistency

If you receive an error `The correct download file name cannot be established (device identifier not verified)` please obtain an *OmGUI Detailed Log* as described above (if not already done so).  The log entry may contain `LOG: - MISMATCH:`, indicating an issue with device ID.  

To manually verify device IDs, with a single connected device, please check the following four numbers (these should be the same):

1. *External ID:* The number written on the side of the device (AX3: the number after the `17-` or `18-` prefix; AX6: 7-digit numbers should start with a `60`)

2. *USB ID:* In *Device Manager*, under *Ports*, and the *COM* device under that, right-click / *Properties* / *Details* / *Property: Parent* -- the number that appears as the *Value* after the first part of the address `USB\VID_04D8&PID_0057\#####_` (where `#####` is `CWA17` or `AX664`)

3. *Filesystem ID:* Locate the disk drive that appears under *This PC* when you connected the device, right-click the drive and select *Properties*, the highlighted field, note the *Volume Label*, it should start `AX#_` (where `#` is `3` or `6`)

4. *Data-file ID:* The ID in the current data file on the device.  This is a bit difficult to extract! Press <kbd>Windows</kbd>+<kbd>R</kbd> to open the *Run* box, and copy and paste the following command to open a window and give you a number (replace `D:` with the drive letter for your device from above if necessary):

   ```cmd
   PowerShell -Command "& {$s=[System.IO.File]::OpenRead('D:\CWA-DATA.CWA');$b=New-Object byte[] 13;$c=$s.Read($b,0,13);$s.close();Write-Output(16777216*$b[12]+65536*$b[11]+256*$b[6]+$b[5]);[Console]::ReadKey()}"
   ```

If these numbers are inconsistent, you could try *resetting the device ID* in the next section.


## Resetting the device ID

If it appears as if the device ID has somehow been incorrectly reprogrammed, you could try this procedure to reset the ID.  

**IMPORTANT:** This will *reformat* the device, deleting any data on there.  Please be certain it does not have the only copy of any data you'd like to keep.  You can manually move off data from the drive by locating the device's drive letter in *File Explorer* and move the `CWA-DATA.CWA` file to a safe location.
 
1.	Download the .ZIP file: [AX3-Bootloaders](https://github.com/digitalinteraction/openmovement/blob/master/Downloads/AX3/AX3-Bootloaders.zip?raw=true)

2.	Open the .ZIP file and extract the program `HidBootLoader.exe`.

3.	Ensure no devices are connected and that OmGui is NOT running.

4.	Run (double-click) `HidBootLoader.exe`.

5.	Check the *Port* field is clear (if not, make a note of what it says)

6.	Connect a device that you’d like to replace the ID on and does not contain any data (this procedure will wipe the drive) and wait a second or so.

7.	If enabled, press the *Run* button and wait a couple of seconds or so.

8.	Check the *Port* field now displays a `COM` port (if it had a value before, use the drop-down arrow if necessary to ensure that it now has a different value)

9.	In the *Command* field, copy and paste the following line -- change `12345` to match the ID number on the outside of the device case:

    ```
    DEVICE=12345|FORMAT QC|LED 5
    ```

10. Press *Send*

11. Wait several seconds while the device LED is red, it should eventually turn *Magenta* (a purple blue/red mixture).

12. Disconnect the device

13. Close the bootloader software

14. The device should now have the correct ID when inserted again


## Removing a mount point

(This is for advanced use only, and unlikely to apply/work).

Perhaps an old *mount point* is interfering somehow - you could try clearing the mount point:
 
1. Make sure OmGui is not running, and the problematic device IS connected

2. Start an *elevated* command prompt (with administrative permission), *either* of these methods:

   * Press Start, type `Command`, right-click the *Command Prompt* search result and select *Run as Administrator*
   
   * Press <kbd>Windows</kbd>+<kbd>R</kbd>, type `cmd`, <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>Enter</kbd>, select *Yes*
   
3. Type (or copy/paste, the `|` symbol may be on <kbd>Shift</kbd>+<kbd>\</kbd>):

   ```cmd
   cmd /k mountvol
   ```

4. The command may list the device's current volume, e.g.: `C:\Mount\AX3_#####\`

5. If so, type: (replace the path with whatever the previous command showed)

   ```cmd
   mountvol C:\Mount\AX3_#####\ /D 
   ```

6. Assuming no error message was shown, the mount point was removed: you can disconnect the device and close the window.

