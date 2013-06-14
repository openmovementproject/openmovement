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
% Cassim Ladha Nov 2012
%
% Script to find Energy expendiature, PA levels and steps from a
% accelerometer data gathered on the left wrist
%       Input arguments:
%           cwaFilePath            Path to AX3 sensor file
%
%       OPTIONS:
%
%           'site'              1 = wrist, 2 = waist
%
%           'side'              For wrist mounted data
%                               1 = left, 2 = right
%
%           'mass'              participant mass in Kg
%
%           'bodyFat'           participant body Fat in decimal (1=100%)
%
%           'epoch'             length of time to summarize into epoch
%
%           'csv'               string containing path of output file
%
%       Output arguments:
%
%           PAdata -> m X 5 matrix 
%                               Format = [time, Energy ,Intensity, numSteps, distance]
%
%           'time'              Matlab Format Time Stamp. 
%                               (/86400 to get into seconds)
%
%           'Energy'            Unit = Joules. Ammount of Energy used between 
%                               the last time stamp and this one.
%
%           'Intensity'         Level of PA intensity for that timestamp.
%                               Updates every 60 Seconds
%                
%                               1 = light
%                               2 = moderate
%                               3 = vigorous     
%
%           'steps'             Number of steps in between last and present
%                               time stamp
%
%           'dist'              resulting distrance travelled in (m) from
%                               any steps taken
%
% Example Use:
%               findPA('input.CWA','site',1,'side',1,'mass',80,'bodyFat',0.2,'epoch',60,'csv','output.CSV');
%

function PAdata = findPA(filename,varargin)
    % parse arguments
    p = inputParser;
    
    % define optional arguments with default values
    addOptional(p,'site',1,@isnumeric);         
    addOptional(p,'side',1,@isnumeric);     
    addOptional(p,'mass',80,@isnumeric);     
    addOptional(p,'bodyFat',0.22,@isnumeric);     
    addOptional(p,'epoch',60,@isnumeric);       
    addOptional(p,'csv','PAdata.csv',@ischar);       
    
    % parse varargin
    parse(p,varargin{:});
    
    try
    fileinfo = AX3_readFile(filename, 'info', 1, 'useC', 1);
    catch ex
        fprintf('Error: Check file name and path\r\n');
        disp(ex.message)
        PAdata = -1;
        return;
    end
    
    % We do progress reporting by hours. 
    % First calculate number of hours
    numHours = (fileinfo.stop.mtime - fileinfo.start.mtime)*24;
    
    %if less than one hour call in one go otherwise call hour by hour and
    %then report progress in between. 
    % It takes about 16seconds to run the script for 1 hour.
    PAdata = [];
    if(numHours < 1)
        startTime = fileinfo.start.mtime;
        stopTime = fileinfo.stop.mtime;
        PAdata = calcPA(filename, p.Results,fileinfo,startTime,stopTime);
    else
        startTime = fileinfo.start.mtime;
        stopTime = fileinfo.start.mtime + (1/24);
        for i=2:floor(numHours)-1
            PAdata = [PAdata; calcPA(filename, p.Results,fileinfo,startTime,stopTime)];
            fprintf('progress: %d percent \n', round(100*(i/numHours))); 
            startTime = stopTime + 1/86400; %start the next second after where we stopped
            stopTime = fileinfo.start.mtime + (i/24);
        end
        PAdata = [PAdata; calcPA(filename, p.Results,fileinfo,stopTime,fileinfo.stop.mtime)];  %do the rest of the file - last stop time till end of file.
    end
    
    % Reporting if needed
    try
        if(p.Results.csv)
            fId = fopen(p.Results.csv,'w');
            for i =1:length(PAdata)
                fprintf(fId, '%s,%d,%d,%d,%.2f\n', datestr(PAdata(i,1)),PAdata(i,2),PAdata(i,3),PAdata(i,4),PAdata(i,5));
            end
        end
        fclose(fId);
    catch ex
        fprintf('Error: Problems calculating PA parameters\r\n');
        PAdata = -1;
        return
    end
end


function PAdata  = calcPA(cwaFilePath,options,fileinfo,startTime,stopTime)
PAdata = [];
%fileinfo = AX3_readFile(cwaFilePath, 'info', 1, 'useC', 1);
data = AX3_readFile(cwaFilePath, 'validPackets', fileinfo.validPackets, 'startTime', startTime, 'stopTime', stopTime);

% Step 1 - Run a wear time validation and plot non-complient periods agains the svm
%wtv = findWTV(data.ACC,0.005,60); %SVM < 5mg for 60 secs results in non-complient
try
wtv = findWTV_sd(data.ACC,0.03,1800); %std < 0.3 for 30mins (1800 secs) results in non-complient
catch ex
    disp(ex.message)
    PAdata = -1;
    return
end

% blank out the sections of Data that the device was not worn

for i = 1:length(wtv)
    if(wtv(i ==0))
        data.ACC(i,2:4)  = 0; % delete the row
    end
end

% Step 2 - Calculate EE into epoch periods
try
paEE = findPAEE_energy(options.mass,options.bodyFat,data.ACC,options.epoch);
catch ex
    disp(ex.message)
    PAdata = [0 -1];
    return
end

% Step 3 - Calculate the time spent in light, moderate and vigorous activity
try
pait = findPAIT(data.ACC,options.epoch,options.site,options.side);
catch ex
    disp(ex.message)
    PAdata = [0 -1];
    return
end

% Step 4 - Find time of any detected steps and distance travelled
try
    steps = findSteps(data.ACC);
    if (isempty(steps))
        steps = [paEE(1,1) 0 0];
    end
catch ex
    disp(ex.message)
    PAdata = [0 -1 -1];
    return
end

% Step 5 - Reporting
% <time>,<Energy in Js^-1>,<PA intensity>,<steps>,<Walked Distance m/s>
try
for i =1:length(paEE)
    % time is in seconds epochs. We Already have this nicely and continuously
    % to hand from the paEE. NOTE. Time is in MATLAB format
    tick = paEE(i,1);
    %Energy Expendiature is in colomn 2 of paEE and calculated per second basis
    Energy = paEE(i,2);

    %intensity is calculated on a 60second basis. If we are into a just stretch
    %and repeat the results into a second by second result for this report
    Intensity = pait(floor(i/60)+1,2);

    %if there is a step in this second
    relSteps = steps(steps(:,1) >= tick & steps(:,1) <= tick + 1/86400,:);
    numSteps = size(relSteps,1);
    distance = sum(relSteps(:,2));
    
    PAdata = [PAdata; tick, round(Energy),Intensity,numSteps,distance];
end
catch ex
    disp(ex.message)
    PAdata = [-1 -1 -1 -1 -1];
    return
end

end %function end