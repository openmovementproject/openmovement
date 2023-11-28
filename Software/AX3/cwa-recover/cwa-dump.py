#!/usr/bin/env python
# coding=UTF-8
# CWA Drive Dump

from subprocess import check_output
import time
from datetime import timedelta
import ctypes
import sys
import os
import atexit
#import io

def driveDump(path, outputFile, mode):
  blockSize = 128 * 1024
  
  print("DriveDump:", path)
  try:
    # AX3 drive size: fileSize = 992161*512
    # AX6 drive size: fileSize = 1975995*512
    print("Detecting device physical drive size...")
    fileSize = findPhysicalDriveSize(path)
    if fileSize <= 0:
      print("ERROR: Problem determining drive size.")
      return

    startTime = time.time()
    with open(path, 'rb') as fi:
      with open(outputFile, mode) as fo:
        writtenSize = 0
        # Resume
        offset = fo.tell()
        fi.seek(offset)
        while True:
          data = fi.read(blockSize)
          size = len(data)
          if size <= 0:
            break
          written = fo.write(data)
          fo.flush()
          if written != size:
            print("ERROR: Problem writing all of the data, wrote " + str(written) + " of " + str(size) + "")
            break
            
          offset += size
          writtenSize += written
          perc = round(100 * offset / fileSize, 1)
          elapsed = time.time() - startTime
          remaining = 0
          if elapsed > 0:
            rate = writtenSize / elapsed
            if rate > 0:
              remaining = (fileSize - offset) / rate
          print("Dumping " + str(written) + " =" + str(writtenSize) + " @" + str(offset) + " /" + str(fileSize) + " (" + str(perc) + "%) in " + str(timedelta(seconds=int(elapsed))) + ", " + str(round(rate / 1024, 3)) + " kB/s, ETA " + str(timedelta(seconds=int(remaining))) + ".")
          
          if offset >= fileSize:
            break

  except FileExistsError:
    print("ERROR: Output file already exists.  Remove, rename, or use options --overwrite or --resume: ", outputFile)

  except PermissionError:
    print("ERROR: Permission error -- you must run this in an Ctrl+Shift+Esc, Alt+F, N, cmd, 'Create this task with administrative privileges.'")
    return
    

def findPhysicalDriveSize(physicalDrive):
  out = check_output(["wmic", "diskdrive", "list", "brief"])
  lines = out.split(b"\r\n")
  for line in lines:
    parts = line.split()
    for part in parts:
      if part.decode() == physicalDrive:
        return int(parts[-1].decode())
  return 0


def findPhysicalDrives(prefixDevice, prefixDrive):
  out = check_output(["wmic", "diskdrive", "list", "brief"])
  paths = []
  lines = out.split(b"\r\n")
  for line in lines:
    match = False
    for prefix in prefixDevice:
      if line.startswith(prefix):
        match = True
    if match:
      parts = line.split()
      for part in parts:
        if part.startswith(prefixDrive):
          paths.append(part.decode("utf-8"))
  return paths

 
def findSingleDrive(prefixDevice, prefixDrive):
  paths = findPhysicalDrives(prefixDevice, prefixDrive)
  if len(paths) <= 0:
    print("WARNING: Found no matching drive (expecting one):")
    return None
  elif len(paths) > 1:
    print("WARNING: Found too many matching drives (expecting at most one):")
    for path in paths:
      print("", path);
    return None
  else:
    path = paths[0]
    print("NOTE: Found path:", path)
    return path


def is_admin():
  try:
    return ctypes.windll.shell32.IsUserAnAdmin()
  except:
    return False


def pause():
  print("Press Enter to continue...")
  input()

def main():
  print("Checking admin permissions...")
  if not is_admin():
    # Re-run the program with admin rights
    params = " ".join(['"%s"' % (x,) for x in sys.argv[0:]])
    print("Spawning version with admin rights...", sys.executable, params)
    ctypes.windll.shell32.ShellExecuteW(None, "runas", sys.executable, params, None, 1)
    return

  print("Running...")
  atexit.register(pause);

  outputFile = None
  mode = "xb"
  arg = 1
  while arg < len(sys.argv):
    if sys.argv[arg].startswith("-"):
      if sys.argv[arg] == "--no-overwrite":
        mode = "xb"
      elif sys.argv[arg] == "--overwrite":
        mode = "wb"
      elif sys.argv[arg] == "--resume":
        mode = "ab"
      else:
        print("ERROR: Unrecognized option: " + sys.argv[arg])
        return
    elif outputFile == None:
      outputFile = sys.argv[arg]
    else:
      print("ERROR: Unrecognized positional argument: " + sys.argv[arg])
      return
    arg += 1

  if outputFile is None:
    outputFile = "cwa-dump.img"
  print("NOTE: Using output file in mode=" + mode + ":", outputFile)
  
  print("Detecting device physical drive...")
  prefixDevice = [b"AX3 AX3 Mass Storage USB Device", b"AX6 AX6 Mass Storage USB Device"]
  prefixDrive = b"\\\\.\\PHYSICALDRIVE"
  drivePath = findSingleDrive(prefixDevice, prefixDrive)
  if drivePath is None:
    print("ERROR: Cannot continue without a device drive.")
    return

  # Run code needing admin rights
  driveDump(drivePath, outputFile, mode)

if __name__ == "__main__":
  main()
