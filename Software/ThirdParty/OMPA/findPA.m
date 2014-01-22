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
%
%   NOTES: Function complies with OM plugin standards and uses stdout to
%   communicate progress and errors.
%
%       Input arguments:
%           cwaFilePath            Path to AX3 sensor file
%
%       OPTIONS:
%
%           'starttime'         start time in ISO8061 format
%
%           'stoptime'          stop time in ISO8061 format
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
%           'epoch'             epoch summary period to use. Valid units are:
%                               'secs'
%                               'mins'
%                               'hours'
%                               'days'
%
%           'csv'               string containing path of output file
%
%           'js'                string containing path of javascript used to load a JSON object of data.Epoch
%
%           'html'              string containing path interactive HTML Report
%
%
%
%       Output arguments:
%
%           PAdata ->
%                       .RAW    m X 5 matrix
%                              Format - [time, Energy ,Intensity, numSteps, distance]
%
%           'time'              Time Stamp ISO8061.
%
%           'Energy'            Unit = Cals. Ammount of Energy used between
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
%                       .Epoch  summarised version of .Raw into Epoch size summaries
%                               Format is in a struct as follows:
%                                .'t' - time stamp in ISO8061 format
%                                .'ee'- energy expendiature in epoch period
%                                .'intensity_l' - seconds spent in light exercise
%                                .'intensity_m' - seconds spent in mod exercise
%                                .'intensity_v' - seconds spent in vig exercise
%                                .'steps' - steps taken in epoch period
%                                .'dist' - dist traveled in epoch period
%   Example Use:
%               myPA = findPA('input.CWA','site',1,'side',1,'mass',80,'bodyFat',0.2,'epoch',mins,'csv','outputPA.CSV','js','outputPA.json');
%

function PAdata = findPA(filename,varargin)

% print out just to get the progress bar to load properly
fprintf('progress: 0 percent \n');

% parse arguments
p = inputParser;
p.KeepUnmatched = true;
ts=now;
% define optional arguments with default values
addOptional(p,'site',1,@isnumeric);
addOptional(p,'side',1,@isnumeric);
addOptional(p,'mass',80,@isnumeric);
addOptional(p,'bodyFat',0.22,@isnumeric);
addOptional(p,'epoch','mins',@ischar);
addOptional(p,'csv',strcat(datestr(ts,31),'data.csv'),@ischar);
addOptional(p,'js',strcat(datestr(ts,31),'data.js'),@ischar);
addOptional(p,'html',strcat(datestr(ts,31),'data.html'),@ischar);
addOptional(p,'startTime','fileStart',@ischar);
addOptional(p,'stopTime','fileEnd',@ischar);
% parse varargin
parse(p,varargin{:});

currentFolder = pwd; % THIS IS THE PATH OF THE (..\PLUGINS\PLUGIN\)
fprintf('current path: %s\n', currentFolder);

try
    fileinfo = AX3_readFile(filename, 'info', 1, 'useC', 0); % USED 'C' TURNED OFF
catch ex
    fprintf('Error: Check file name and path\r\n');
    disp(ex.message)
    PAdata = -1;
    return;
end

%depending if we specified a startTime then start at it
if(strcmp(p.Results.startTime,'fileStart'))
    startTime = fileinfo.start.mtime;
else
    try
        startTime = datenum(p.Results.startTime,'dd/mm/yyyy/_HH:MM:SS');
        %fprintf('status: start time set to %s %s\n', datestr(startTime),startTime);
    catch ex
        fprintf('Error: Invalid start time specified');
        disp(ex.message)
        PAdata = -1;
        return;
    end
end
%depending if we specified a stopTime then end at it
if(strcmp(p.Results.stopTime,'fileEnd'))
    stopTime = fileinfo.stop.mtime;
else
    try
        stopTime = datenum(p.Results.stopTime,'dd/mm/yyyy/_HH:MM:SS');
        %fprintf('status: stop time set to %s %s\n', datestr(stopTime), stopTime);
    catch ex
        printf('Error: Invalid start time specified');
        disp(ex.message)
        PAdata = -1;
        return;
    end
end

% We do progress reporting by hours.
% First calculate number of hours
numHours = (stopTime - startTime)*24;

fprintf('Status: Calculating the PA data.\r\n');

%if less than one hour call in one go otherwise call hour by hour and
%then report progress in between.
% It takes about 16seconds to run the script for 1 hour.
PAdata.Raw = [];
if(numHours <= 3)
    PAdata.Raw = calcPA(filename, p.Results,fileinfo,startTime,stopTime);
else
    
    % pre-calculate for parallel tbx compatability
    %startTime(1) = startTime; % startTime already saved to trot on
    stopTime(ceil(numHours)) = stopTime; %save the actual last value here for later
    stopTime(1) = startTime + (1/24); %1 hour in from the start
    
    for i=2:ceil(numHours)-1
        startTime(i) = stopTime(i-1) + 1/86400; %make sure start the second after last stop time
        stopTime(i) = startTime(1) + (i/24);
    end
    
    for i=1:ceil(numHours)-1
        PAdata.Raw = [PAdata.Raw; calcPA(filename, p.Results,fileinfo,startTime(i),stopTime(i))];
        fprintf('progress: %d percent \n', round(100*(i/numHours)));
    end
    
    PAdata.Raw = [PAdata.Raw; calcPA(filename, p.Results,fileinfo,startTime(end)+(1/86400),stopTime(end))];  %do the rest of the file - the last loop just did hours but might be a bit left over
    
end

% summarize into epoch periods here
PAdata.Epoch = summarizeData(p.Results.epoch,PAdata.Raw);

% Reporting if needed
try
    
    if~(strcmp(strcat(datestr(ts,31),'data.csv'),p.Results.csv))
        fprintf('Status: Writing the csv file.\r\n');
        meta = defaultMetaAX3();
        
        fId = fopen(p.Results.csv,'w');
        %PrintMeta(fId,meta);
        fprintf(fId,'Time(yyyyddmmThhmmss), Physical Activity (Cals),Intensity (1=light,2,mod,3=vig),Steps,Dist(m)\r\n');
        for i =1:length(PAdata.Raw)
            fprintf(fId, '%s,%d,%d,%d,%.2f\n', datestr(PAdata.Raw(i,1),31),PAdata.Raw(i,2),PAdata.Raw(i,3),PAdata.Raw(i,4),PAdata.Raw(i,5));
        end
        fclose(fId);    %close the file
    end
    
    if~(strcmp(strcat(datestr(ts,31),'data.js'),p.Results.js))
        
        fprintf('Status: Writing the JS file.\r\n');
        
        % AX3 readfile does not support meta data yet so include this temporary
        % HACKEROO:
        PAjson = defaultMetaAX3();
        
        %file info should have all meta data in but called parameters take
        %prescedence. Set from fileInfo and then try resetting from called
        %parameters:
        %SetMeta(fileinfo,PAjson);
        
        PAjson.Data = PAdata.Epoch;
        jsonstring = savejson('PAjson',PAjson);
        
        
        %% write output - into a js function for now...!!
        
        fId = fopen(p.Results.js,'w');
        % function readData()
        % {
        %   var data = [
        %            JSON OBJECT
        %            ...
        %          ];
        %   return data;
        % }
        fprintf(fId,'function readData() {\r\n');
        fprintf(fId,'    var data = \r\n');
        fprintf(fId,'%s\r\n',jsonstring);
        fprintf(fId,';\r\n');
        fprintf(fId,'return data;\r\n');
        fprintf(fId,'}\r\n');
        fclose(fId);
        
        
    end
    
    if~(strcmp(strcat(datestr(ts,31),'data.html'),p.Results.html))
        fprintf('Status: p.results is %s\n', p.Results.html);
        outPath = sprintf('%s', p.Results.html);
        fprintf('Status: outPath is %s\n', outPath);
        
        %fprintf('Status: outPath(1,1:end-5) is %s\n', outPath(1,1:end-5));
        
        outPath = outPath(1,1:end-5);
        
        fprintf('Status: the outPath is is now%s\n', outPath);
        
        %p.Results.html = p.Results.html(1:end-5);
        
        % step 1 - make the directory structure and copy the static files accross
        if(ispc)
            fprintf('Status: using a PC\n');
            %             slashes = strfind(p.Results.html,'\');
            %             fPath = p.Results.html(1:slashes(end));
            %             fId = p.Results.html(slashes(end)+1:end);
            %             fprintf('Status: fPath %s\r\n', fPath);
            %             fprintf('Status: fId %s\r\n', fId);
            if(strcmp(outPath(1,2:3),':\'))
                %a full path has been given so use it
                fprintf('Status: a full path was given\n');
                fPath = outPath;
            else
                %only a file name was given so make the full path
                fprintf('Status: only a file name was given');
                fPath = [pwd '\' outPath];
            end
            fprintf('Status: fPath is %s\n', fPath);
            
            fprintf('Status: making directory...\n');
            mkdir(fPath);
            fprintf('Status: copying files...\n');
            copyfile([currentFolder '\OMPA_viewer\*.*'],[fPath '\'],'f')
            
        else
            %             slashes = strfind(p.Results.html,'/');
            %             fPath = p.Results.html(1:slashes(end));
            %             fId = p.Results.html(slashes(end)+1:end);
            fPath = [pwd '/' outPath];
            mkdir(fPath);
            
            mkdir(fPath);
            copyfile([currentFolder '/OMPA_viewer/*.*'],[fPath '/'],'f')
        end
        
        % step 2 - make the data.js
        fprintf('Status: Writing the HTML file.\r\n');
        
        % AX3 readfile does not support meta data yet so include this temporary
        % HACKEROO:
        PAjson = defaultMetaAX3();
        
        PAjson.Data = PAdata.Epoch;
        jsonstring = savejson('PAjson',PAjson);
        
        %% write output - into a js function for now...!!
        if(ispc)
            fId = fopen([fPath '\data\data.js'],'w');
        else
            fId = fopen([fPath '/data/data.js'],'w');
        end
        % function readData()
        % {
        %   var data = [
        %            JSON OBJECT
        %            ...
        %          ];
        %   return data;
        % }
        fprintf(fId,'function readData() {\r\n');
        fprintf(fId,'    var data = \r\n');
        fprintf(fId,'%s\r\n',jsonstring);
        fprintf(fId,';\r\n');
        fprintf(fId,'return data;\r\n');
        fprintf(fId,'}\r\n');
        fclose(fId);
        
    end
    
catch ex
    fprintf('Error: Problems calculating PA parameters\r\n');
    PAdata = -1;
    return
end
end
%
% function retval = calcFeatures(data)
%     retval.cals = sum(data(:,2));
%     retval.intensity.light = length(find(data(:,3)==1));
%     retval.intensity.moderate = length(find(data(:,3)==2));
%     retval.intensity.vigorous = length(find(data(:,3)==3));
%     retval.steps = sum(data(:,4));
%     retval.dist = sum(data(:,5));
% end

function PAdata  = calcPA(cwaFilePath,options,fileinfo,startTime,stopTime)
PAdata = [];
%fileinfo = AX3_readFile(cwaFilePath, 'info', 1, 'useC', 1);
tmpData = AX3_readFile(cwaFilePath, 'validPackets', fileinfo.validPackets, 'startTime', startTime, 'stopTime', stopTime);

% Step 1 - Run a wear time validation and plot non-complient periods agains the svm
%wtv = findWTV(data.ACC,0.005,60); %SVM < 5mg for 60 secs results in non-complient
try
    wtv = findWTV_sd(tmpData.ACC,0.03,900); %std < 0.3 for 15mins (900 secs) results in non-complient
catch ex
    disp(ex.message)
    PAdata = -1;
    return
end

% blank out the sections of Data that the device was not worn

for i = 1:length(wtv)
    if(wtv(i ==0))
        tmpData.ACC(i,2:4)  = 0; % delete the row
    end
end

% Step 2 - Calculate EE into epoch periods
try
    paEE = findPAEE_energy(options.mass,options.bodyFat,tmpData.ACC);
catch ex
    disp(ex.message)
    PAdata = [0 -1];
    return
end

% Step 3 - Calculate the time spent in light, moderate and vigorous activity
try
    pait = findPAIT(tmpData.ACC,options.site,options.side);
catch ex
    disp(ex.message)
    PAdata = [0 -1];
    return
end

% Step 4 - Find time of any detected steps and distance travelled
try
    %cas-  make this FASTER
    
    steps = findSteps(tmpData.ACC);
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
    
    %init variables
    PAdata=[];
    for i =1:length(paEE)
        
        % time is in 60 seconds epochs. We Already have this nicely and continuously
        % to hand from the pait. NOTE. Time is in MATLAB format
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