function info = wavInfo(file)
% Load movement WAV file information
% - Dan Jackson, 2019.
%
%  info = wavInfo('CWA-DATA.wav');
%  data = audioread('CWA-DATA.wav');
%
% First sample is at the timestamp:
%
%   info.Storage.Time
%
% Sample rate:
%
%   info.SampleRate
%
% Duration (seconds):
%
%   info.Duration
%
% First channel data:
%
%   info.Storage.Channel_1  % = 'Accel-X'
%
% First channel scale:
%
%   info.Storage.Scale_1  % = 8
%
% Fourth channel data:
%
%   info.Storage.Channel_4  % = 'Gyro-X'
%
% Fourth channel scale:
%
%   info.Storage.Scale_4  % = 2000
%

function params = keyValuePairs(str)
    params = struct();
    str = str.';        
    breakIndexes = find(str(:)==double(10)).';
    breakIndexes = [1-1 breakIndexes length(str)+1];
    for idx = 2:length(breakIndexes)
        pair = str(breakIndexes(idx-1)+1:breakIndexes(idx)-1);

        equals = find(pair(:) == double(':'));
        if isempty(equals)
            equals = length(pair) + 1;
        else
            equals = equals(1);
        end

        key = pair(1:(equals-1));
        if equals + 1 <= length(pair)
            value = pair(equals + 1:end);
        else
            continue;
        end

        key = key.';
        value = value.';

        key = strrep(key,'-','_');
        value = strip(value);
        
        if startsWith(key, 'Scale_') || startsWith(key, 'Session') 
            value = str2num(value);
        end
        if endsWith(key, 'Time') || endsWith(key, 'Start') || endsWith(key, 'Stop')  
            value = datetime(value, 'InputFormat', 'yyyy-MM-dd HH:mm:ss.SSS');
        end

        %params{end+1} = struct('key', key, 'value', value); %#ok<AGROW>
        params.(key) = value;
    end

end

info = audioinfo(file);

% info.Artist - Data about the device that made the recording
info.Device = keyValuePairs(info.Artist);

% info.Title - Data about the recording itself
info.Recording = keyValuePairs(info.Title);

% info.Comment - Data about this file representation
info.Storage = keyValuePairs(info.Comment);


end
