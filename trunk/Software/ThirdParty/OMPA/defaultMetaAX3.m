% Copyright (c) 2009-2012, Newcastle University, UK.
% All rights reserved.
% 
% Redistribution and use in source and binary forms, with or without 
% modification, are permitted provided that the following conditions are met: 
% 1. Redistributions of source code must retain the above copyright notice, 
%    this list of conditions and the following disclaimer.
% 2. Redistributions in binary form must reproduce the above copyright notice, 
%    this list of conditions and the following disclaimer in the documentation 
%    and/or other materials provided with the distribution.
% THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
% AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
% IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
% ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
% LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
% CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
% SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
% INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
% CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
% ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
% POSSIBILITY OF SUCH DAMAGE. 
%
% Cassim Ladha Feb 2013
%
% Script to populate default meta data into an outputect.

function output = defaultMetaAX3()
    
    output.Device.Type = 'AX3';           
    output.Device.Model = '17';
    output.Device.ID = '12345';
    output.Device.Firmware = 'R36';
    output.Device.Calibration = '0,0,0';

    output.Recording.StartTime = '2000-14-10 12:34:56:789'; %Obligitory
    output.Recording.StopTime = '2000-15-10 12:34:56:789'; %Obligitory
    output.Recording.LocationSite = 'wrist'; %waist,ankle,thigh,hip,upper-arm,chest,sacrum,neck,head
    output.Recording.LocationSide = 'left'; %right
    output.Recording.TimeZone = 'GMT+1'; %Obligitory %GMT-9 to GMT+12

    output.Subject.Code = 'Participant1'; %free-text
    output.Subject.DOB = '1981-14-10'; %yyyy-dd-mm
    output.Subject.Sex = 'male'; %female
    output.Subject.Heightcm = '183';
    output.Subject.Weightkg = '78';
    output.Subject.Handedness = 'right'; %left
    output.Subject.Notes = 'Neque porro quisquam est qui dolorem'; %free text

    output.Study.Centre = 'Newcastle'; %free text
    output.Study.Code = 'Study #1'; %free text
    output.Study.Investigator = 'A Apple'; %free text
    output.Study.ExerciseType ='Daily Living'; %free text
    output.Study.ConfigOperator = 'B Bannana'; %free text
    output.Study.ConfigTime = '2000-14-10 00:00:00:000'; %Obligitory %yyyy-dd-mm hh:mm:ss.000 got from PC
    output.Study.ConfigNotes = 'Ipsum quia dolor sit amet, consectetur'; %free text

    output.Extract.Operator = 'C Cherry'; %free text
    output.Extract.Time = '2000-16-10 12:34:56:789'; %Obligitory %yyyy-dd-mm hh:mm:ss.000 got from PC
    output.Extract.Notes = 'Lorem ipsum dolor sit amet, consectetur adipisicing elit'; 

    output.Sensor.Acc.SampleFreq = '100'; %Obligitory
    output.Sensor.Acc.Range = '8';  %Obligitory           
    output.Sensor.Acc.Res = '30'; %in mg 
    output.Sensor.Acc.Units = 'g';                   
    output.Sensor.Acc.Axis = 'xyz';
    output.Sensor.Acc.Model = 'ADXL345';

    output.Sensor.Temp.SampleFreq = '0.0166';             
    output.Sensor.Lux.Range = '10bit';             
    output.Sensor.Temp.Res = '1'; %in degC
    output.Sensor.Temp.Units = 'C';                   
    output.Sensor.Temp.Model = 'MCP9700';

    output.Sensor.Lux.SampleFreq = '1';
    output.Sensor.Lux.Range = '10bit';             
    output.Sensor.Lux.Res = '8bit';
    output.Sensor.Lux.Units = 'LogLux';                   
    output.Sensor.Lux.Model = 'ADPS9007';
