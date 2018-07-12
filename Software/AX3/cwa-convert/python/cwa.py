#!/usr/bin/env python
# coding=UTF-8
#
#  CWA Data Conversion Tool
#     
#  Copyright (c) 2011 Technische Universität München, 
#  Distributed Multimodal Information Processing Group
#  http://vmi.lmt.ei.tum.de
#  All rights reserved.
#
#  Stefan Diewald <stefan.diewald [at] tum.de>
#     
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following
#    disclaimer in the documentation and/or other materials provided
#    with the distribution.
#  * Neither the name of the University or Institute nor the names
#    of its contributors may be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
#  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
#  COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
#  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
#  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
#  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
#  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
#  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.
#

from struct import *
from math import *
import time
from datetime import datetime
import sqlite3
import sys
import io

def byte(value):
    return (value + 2 ** 7) % 2 ** 8 - 2 ** 7 

def ushort(value):
    return value % 2 ** 16

def short(value):
    return (value + 2 ** 15) % 2 ** 16 - 2 ** 15

# Local "URL-decode as UTF-8 string" function
def urldecode(input):
    output = bytearray()
    nibbles = 0
    value = 0
    # Each input character
    for char in input:
        if char == '%':
            # Begin a percent-encoded hex pair
            nibbles = 2
            value = 0
        elif nibbles > 0:
            # Parse the percent-encoded hex digits
            value *= 16
            if char >= 'a' and char <= 'f':
                value += ord(char) + 10 - ord('a')
            elif char >= 'A' and char <= 'F':
                value += ord(char) + 10 - ord('A')
            elif char >= '0' and char <= '9':
                value += ord(char) - ord('0')
            nibbles -= 1
            if nibbles == 0:
                output.append(value)
        elif char == '+':
            # Treat plus as space (application/x-www-form-urlencoded)
            output.append(ord(' '))
        else:
            # Preserve character
            output.append(ord(char))
    return output.decode('utf-8')

class CWA_Sample:
    pass

class CWA:
    def read(self, bytes):
        data = self.fh.read(bytes)
        if len(data) == bytes:
            return data
        else:
            raise IOError
    
    def __init__(self, filename):
        self.fh = open(filename, 'rb')
        
        dbcon = sqlite3.connect(sys.argv[1] + '.sqlite')
        dbcurs = dbcon.cursor()
        
        dbcurs.execute('''DROP TABLE IF EXISTS acc''')
        dbcurs.execute('''CREATE TABLE acc (time REAL, x INTEGER, y INTEGER, z INTEGER)''')
        dbcurs.execute('''CREATE INDEX time_hash ON acc (time)''')
        
        lastSequenceId = None
        lastTimestampOffset = None
        lastTimestamp = None

        try:
            header = self.read(2)
            while len(header) == 2:
                if header == 'MD':
                    print('MD')
                    self.parse_header()
                elif header == 'UB':
                    print('UB')
                    blockSize = unpack('H', self.read(2))[0]
                elif header == 'SI':
                    print('SI')
                elif header == 'AX':
                    packetLength = unpack('H', self.read(2))[0]               
                    deviceId = unpack('H', self.read(2))[0]
                    sessionId = unpack('I', self.read(4))[0]
                    sequenceId = unpack('I', self.read(4))[0]
                    sampleTime = self.read_timestamp(self.read(4))
                    light = unpack('H', self.read(2))[0]
                    temperature = unpack('H', self.read(2))[0]
                    events = self.read(1)
                    battery = unpack('B', self.read(1))[0]
                    sampleRate = unpack('B', self.read(1))[0]
                    numAxesBPS = unpack('B', self.read(1))[0]
                    timestampOffset = unpack('h', self.read(2))[0]
                    sampleCount = unpack('H', self.read(2))[0]
                    
                    sampleData = io.BytesIO(self.read(480))
                    checksum = unpack('H', self.read(2))[0]
                    
                    if packetLength != 508:
                        continue
                    
                    if sampleTime == None:
                        continue
                    
                    if sampleRate == 0:
                        chksum = 0
                    else:
                        # rewind for checksum calculation
                        self.fh.seek(-packetLength - 4, 1)
                        # calculate checksum
                        chksum = 0
                        for x in range(packetLength / 2 + 2):
                            chksum += unpack('H', self.read(2))[0]
                        chksum %= 2 ** 16
                    
                    if chksum != 0:
                        continue
                    
                    if sessionId != self.sessionId:
                        print("x")
                        continue
                    
                    if ((numAxesBPS >> 4) & 15) != 3:
                        print('[ERROR: Axes!=3 not supported yet -- this will not work properly]')

                    if (light & 0xfc00) != 0:
                        print('[ERROR: Scale not supported yet -- this will not work properly]')
                        
                    if (numAxesBPS & 15) == 2:
                        bps = 6
                    elif (numAxesBPS & 15) == 0:
                        bps = 4
                    
                    freq = float(3200) / (1 << (15 - sampleRate & 15))
                    if freq <= 0:
                        freq = 1
                    
                    # range = 16 >> (rateCode >> 6)
                    
                    timeFractional = 0                    
                    # if top-bit set, we have a fractional date
                    if deviceId & 0x8000:
                        # Need to undo backwards-compatible shim by calculating how many whole samples the fractional part of timestamp accounts for.
                        timeFractional = (deviceId & 0x7fff) * 2     # use original deviceId field bottom 15-bits as 16-bit fractional time
                        timestampOffset += (timeFractional * int(freq)) // 65536 # undo the backwards-compatible shift (as we have a true fractional)
                        timeFractional = float(timeFractional) / 65536
                    
                    # Add fractional time to timestamp
                    timestamp = float(time.mktime(sampleTime)) + timeFractional
                    
                    # --- Time interpolation ---
                    # Reset interpolator if there's a sequence break or there was no previous timestamp
                    if lastSequenceId == None or (lastSequenceId + 1) & 0xffff != sequenceId or lastTimestampOffset == None or lastTimestamp == None:
                        # Bootstrapping condition is a sample one second ago (assuming the ideal frequency)
                        lastTimestampOffset = timestampOffset - freq
                        lastTimestamp = timestamp - 1
                        lastSequenceId = sequenceId - 1
                    
                    localFreq = float(timestampOffset - lastTimestampOffset) / (timestamp - lastTimestamp)
                    time0 = timestamp + -timestampOffset / localFreq
                    
                    # Update for next loop
                    lastSequenceId = sequenceId
                    lastTimestampOffset = timestampOffset - sampleCount
                    lastTimestamp = timestamp
                    
                    sys.stdout.write('*')
                    for x in range(sampleCount):
                        sample = CWA_Sample()
                        if bps == 6:
                            sample.x = unpack('h', sampleData.read(2))[0]
                            sample.y = unpack('h', sampleData.read(2))[0]
                            sample.z = unpack('h', sampleData.read(2))[0]
                        elif bps == 4:
                            temp = unpack('I', sampleData.read(4))[0]
                            temp2 = (6 - byte(temp >> 30))
                            sample.x = short(short((ushort(65472) & ushort(temp << 6))) >> temp2)
                            sample.y = short(short((ushort(65472) & ushort(temp >> 4))) >> temp2)
                            sample.z = short(short((ushort(65472) & ushort(temp >> 14))) >> temp2)
                        
                        sample.t = time0 + (x / localFreq)
                        tStr = "{:.5f}".format(sample.t)
                        dbcurs.execute("INSERT INTO acc VALUES (" + tStr + ", " + str(sample.x) + ", " + str(sample.y) + ", " + str(sample.z) + ")")
                        
                header = self.read(2)
        except IOError:
            pass
            
        dbcurs.execute('''DELETE FROM acc WHERE time < 0''')
        dbcurs.close()
        dbcon.commit()
        dbcon.close()
        
    def parse_header(self):
        blockSize = unpack('H', self.read(2))[0]
        performClear = unpack('B', self.read(1))[0]
        deviceId = unpack('H', self.read(2))[0]
        sessionId = unpack('I', self.read(4))[0]
        shippingMinLightLevel = unpack('H', self.read(2))[0]
        loggingStartTime = self.read(4)
        loggingEndTime = self.read(4)
        loggingCapacity = unpack('I', self.read(4))[0]
        allowStandby = unpack('B', self.read(1))[0]
        debuggingInfo = unpack('B', self.read(1))[0]
        batteryMinimumToLog = unpack('H', self.read(2))[0]
        batteryWarning = unpack('H', self.read(2))[0]
        enableSerial = unpack('B', self.read(1))[0]
        lastClearTime = self.read(4)
        samplingRate = unpack('B', self.read(1))[0]
        lastChangeTime = self.read(4)
        firmwareVersion = unpack('B', self.read(1))[0]

        reserved = self.read(22)

        annotationBlock = self.read(448 + 512)
        
        if len(annotationBlock) < 448 + 512:
            annotationBlock = ""

        annotation = ""
        for x in annotationBlock:
            if ord(x) != 255 and x != ' ':
                if x == '?':
                    x = '&'
                annotation += x
        annotation = annotation.strip()

        annotationElements = annotation.split('&')
        annotationNames = {
                # at device set-up time
                '_c': 'studyCentre',
                '_s': 'studyCode',
                '_i': 'investigator',
                '_x': 'exerciseCode',
                '_v': 'volunteerNum',
                '_p': 'bodyLocation',
                '_so': 'setupOperator',
                '_n': 'notes',
                # at retrieval time
                '_b': 'startTime', 
                '_e': 'endTime', 
                '_ro': 'recoveryOperator', 
                '_r': 'retrievalTime', 
                '_co': 'comments'}
        annotations = dict()
        for element in annotationElements:
            kv = element.split('=', 2)
            annotationName = urldecode(kv[0])
            if kv[0] in annotationNames:
                annotationName = annotationNames[kv[0]]
            annotations[annotationName] = urldecode(kv[1])

        for x in ('startTime', 'endTime', 'retrievalTime'):
            if x in annotations:
                if '/' in annotations[x]:
                    annotations[x] = time.strptime(annotations[x], '%d/%m/%Y')
                else:
                    annotations[x] = time.strptime(annotations[x], '%Y-%m-%d %H:%M:%S')

        self.annotations = annotations
        self.deviceId = deviceId
        self.sessionId = sessionId
        self.lastClearTime = self.read_timestamp(lastClearTime)
        self.lastChangeTime = self.read_timestamp(lastChangeTime)
        self.firmwareVersion = firmwareVersion if firmwareVersion != 255 else 0
        
    def read_timestamp(self, stamp):
        stamp = unpack('I', stamp)[0]
        # bit pattern:  YYYYYYMM MMDDDDDh hhhhmmmm mmssssss
        year  = ((stamp >> 26) & 0x3f) + 2000
        month = (stamp >> 22) & 0x0f
        day   = (stamp >> 17) & 0x1f
        hours = (stamp >> 12) & 0x1f
        mins  = (stamp >>  6) & 0x3f
        secs  = (stamp >>  0) & 0x3f
        try:
            t = time.strptime(str(datetime(year, month, day, hours, mins, secs)), '%Y-%m-%d %H:%M:%S')
        except ValueError:
            t = None
        return t
    
def main():
    if len(sys.argv) < 2:
        print("ERROR: File not specified.");
    else:
        cwa = CWA(sys.argv[1])

if __name__ == "__main__":
    main()
