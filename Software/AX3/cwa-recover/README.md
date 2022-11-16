# CWA Data Recovery

If you have a AX device with a corrupted filesystem, or a file that might have been corrupted, it may be possible to salvage some of the data.


## Step 1: Getting started

1. Check whether you have *Python* installed, e.g. <kbd>Windows</kbd>+<kbd>R</kbd>: `cmd /k python` -- if you get an error message, you can install Python at: [python.org/downloads](https://www.python.org/downloads/)

2. Save this page: [cwa-dump.py](https://raw.githubusercontent.com/digitalinteraction/openmovement/master/Software/AX3/cwa-recover/cwa-dump.py) (as a file on your computer named `cwa-dump.py`)

3. Save this page: [cwa-recover.py](https://raw.githubusercontent.com/digitalinteraction/openmovement/master/Software/AX3/cwa-recover/cwa-recover.py) (as a file on your computer named `cwa-recover.py`, in the same directory)

4. Open a command-line in the same folder as your downloaded files, e.g. <kbd>Windows</kbd>+<kbd>R</kbd>: `cmd /k "cd Downloads"`


## Step 2: Create a disk image from the device

If the data is still on the device: follow these steps to create an *image* file from the device -- this should be done even if you have previously copied off the file, as that data could be incomplete.  If the data was cleared from the device, you can skip this step.

We will use the `cwa-dump.py` script to read the drive to create a disk image.

1. Make sure you have a single AX device attached.

2. Run the *cwa-dump* script: `python cwa-dump.py` 

3. Wait for the script to complete -- this could take up to half an hour.  

The script will create a disk image file `cwa-dump.img` from the single attached AX device.


## Step 3: Recover a `.cwa` file

Follow these instructions to attempt to reconstruct a `.cwa` data file, either from the disk image file (`cwa-dump.img`), or a possibly-corrupted `.cwa` file downloaded from the device.  

1. Run the *cwa-recover* script, either for the disk image:

      `python cwa-recover.py`
      
   ...or, for a possibly-corrupted `.cwa` file, type the line above, plus a space, then drag-and-drop your `.cwa` file into the command prompt, so that the full command is something like this (where `CWA-DATA.CWA` is your filename):

      `python cwa-recover.py "CWA-DATA.CWA"`

2. The script will attempt to reconstruct a file: `cwa-recover.cwa`

3. Inspect the `cwa-recover.cwa` file (e.g. setting the Working Directory in *OmGui* software) to gague how successful the process was.

If you have both a disk image and a downloaded file, you can repeat this process for both files and compare the outputs (you should rename the `cwa-recover.cwa` file between each attempt so that it is not overwritten).

<!-- 

There is a possibility that in some circumstances that being able to read the underlying physical NAND block memory could increase the data recovered, as this would include NAND blocks not used by the logical drive -- however, this is not an interface provided by the current device firmware, and would complicate the recovery, e.g. for overwritten blocks 

Read sectors from a device -- for any header or data sectors found: for each session id, create a map of sequence id to dump file offset (should sort by timestamp as the sequence id can be reset).  If more than one session id is found, the user must choose which to restore.  If no header was found for a specific session id, then a dummy one can be created (but the device id should be specified).  It may be necessary to re-base the sequence id so that it starts at 0).

-->
