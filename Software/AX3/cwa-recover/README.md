# CWA Data Recovery

If you have a AX device with a corrupted filesystem, it may be possible to salvage some of the data.

The `cwa-dump.py` script reads the logical drive to create a disk image; the `cwa-recover.py` script reads the disk image an attempts to reconstruct a data file.

1. If you don't already have it, install: [Python 3](https://www.python.org/downloads/)

2. Save this page: [cwa-dump.py](https://raw.githubusercontent.com/digitalinteraction/openmovement/master/Software/AX3/cwa-recover/cwa-dump.py) (as a file on your computer named `cwa-dump.py`)

3. Save this page: [cwa-recover.py](https://raw.githubusercontent.com/digitalinteraction/openmovement/master/Software/AX3/cwa-recover/cwa-recover.py) (as a file on your computer named `cwa-recover.py`, in the same directory)

4. Open a command-line in the same folder as your downloaded files, e.g. Windows+R: `cmd /k "cd Downloads"`

5. Make sure you have a single AX3 device attached and the AX3 drivers (they will be installed if *OmGUI* has been installed).

6. Run the *cwa-dump* script: `python cwa-dump.py` 

7. The script should create a disk image file `cwa-dump.img` from the single attached AX3 device, this could take up to half an hour.

8. Run the *cwa-recover* script: `python cwa-recover.py`

9. The script will attempt to reconstruct a `cwa-recover.cwa` file from the disk dump image.

10. Inspect the `cwa-recover.cwa` file (e.g. with OmGui) to gague how successful the process was.

<!-- 

There is a possibility that in some circumstances that being able to read the underlying physical NAND block memory could increase the data recovered, as this would include NAND blocks not used by the logical drive -- however, this is not an interface provided by the current device firmware, and would complicate the recovery, e.g. for overwritten blocks 

Read sectors from a device -- for any header or data sectors found: for each session id, create a map of sequence id to dump file offset (should sort by timestamp as the sequence id can be reset).  If more than one session id is found, the user must choose which to restore.  If no header was found for a specific session id, then a dummy one can be created (but the device id should be specified).  It may be necessary to re-base the sequence id so that it starts at 0).

-->
