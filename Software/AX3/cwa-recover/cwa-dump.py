#!/usr/bin/env python
# coding=UTF-8
# CWA Drive Dump

from subprocess import check_output
import time
from datetime import timedelta
import ctypes
import sys
import os
#import io

def driveDump(path, outputFile):
  blockSize = 128 * 1024
  
  print("DriveDump:", path)
  try:
    startTime = time.time()
    with open(path, 'rb') as fi:
      fileSize = 992161*512 # AX3 drive size
      # fi.seek(0, os.SEEK_END) # 2
      # fileSize = fi.tell() # 992161*512
      # fi.seek(0, os.SEEK_SET) # 0
      with open(outputFile, 'wb') as fo:
        writtenSize = 0
        while True:
          data = fi.read(blockSize)
          size = len(data)
          if size <= 0:
            break
          fo.write(data)
          writtenSize += size
          perc = int(100 * writtenSize / fileSize)
          elapsed = time.time() - startTime
          remaining = 0
          if elapsed > 0:
            rate = writtenSize / elapsed
            if rate > 0:
              remaining = (fileSize - writtenSize) / rate
          print("Dump " + str(perc) + "% in " + str(timedelta(seconds=elapsed)) + ", estimated remaining: " + str(timedelta(seconds=remaining)) + "...")
        
  except PermissionError:
    print("ERROR: Permission error -- you must run this in an Ctrl+Shift+Esc, Alt+F, N, cmd, 'Create this task with administrative privileges.'")
    return
    

def findPhysicalDrives(prefixDevice, prefixDrive):
  out = check_output(["wmic", "diskdrive", "list", "brief"])
  
  paths = []
  lines = out.split(b"\r\n")
  for line in lines:
    if line.startswith(prefixDevice):
      parts = line.split()
      for part in parts:
        if part.startswith(prefixDrive):
          paths.append(part.decode("utf-8"))
  
  return paths

 
def findSingleDrive(prefixDevice, prefixDrive):
  paths = findPhysicalDrives(prefixDevice, prefixDrive)
  
  if len(paths) <= 0:
    print("ERROR: Found no matching drive")
    return None
  elif len(paths) > 1:
    print("ERROR: Found too many matching drives (expecting just one):")
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


def main():
  print("Running...")
  outputFile = "cwa-dump.img"
  if len(sys.argv) > 1:
    outputFile = sys.argv[1]
    print("NOTE: Using output file:", outputFile)
    
  if len(sys.argv) > 2:
    print("ERROR: Too many parameters passed")
    return
  
  if os.path.exists(outputFile):
    print("ERROR: Output file already exists, must remove or use another output file:", outputFile)
    return
  
  print("Autodetect...")
  prefixDevice = b"AX3 AX3 Mass Storage USB Device"
  prefixDrive = b"\\\\.\\PHYSICALDRIVE"
  drivePath = findSingleDrive(prefixDevice, prefixDrive)
  if drivePath == None:
    print("ERROR: Cannot continue without drive.")
    return
  
  print("Checking admin permissions...")
  if is_admin():
    # Run code needing admin rights
    driveDump(drivePath, outputFile)
  else:
    # Re-run the program with admin rights
    params = " ".join(['"%s"' % (x,) for x in sys.argv[0:]])
    print("Spawning version with admin rights...", sys.executable, params)
    ctypes.windll.shell32.ShellExecuteW(None, "runas", sys.executable, params, None, 1)

if __name__ == "__main__":
  main()
