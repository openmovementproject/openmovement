function wavWriteAccel(filename, dataAccel, dataLight, dataTemp, Fs, scale, deviceInformation, subjectInformation)

%
% Writes the supplied, evenly sampled, accelerometer data to a WAV file.
%
% The function stores:
%   * the triaxial X/Y/Z values as separate channels.
%   * encodes the additional data (axis validity, temperature, light) as an additional channel
%   * uses the 'artist' field to record information about the device.
%   * uses the 'title' field to record information on the session (id, start/end time, configuration)
%   * uses the 'comment' field to record file information
%


% Check parameter validity
if size(dataAccel, 2) ~= 4
    error('Parameter data must have four channels (time/x/y/z).');
end


% TODO: (In AX3 reader) add disconnection range reader
% TODO: (In AX3 reader) add sector validity check
% TODO: Add padding at start and end of file to match configured interval (aux channel to read invalid)

% Create data for aux channel
timestamps = dataAccel(:,1);  % generate timestamps at 1 second intervals
timestamps = timestamps(1:Fs:end)';
lightVals = interp1(dataLight(:,1),dataLight(:,2),timestamps,'cubic',0);
tempVals = interp1(dataTemp(:,1),dataTemp(:,2),timestamps,'cubic',0);

% Create aux channel
% NOTE: This is not finalized
aux = uint16(zeros(size(dataAccel,1),1));
for i = 1:length(lightVals)
    ofs = i * Fs;
    if ofs < size(dataAccel,1)
        aux(ofs + 0) = int16(32768 + lightVals(i));
        aux(ofs + 1) = int16(32768 + (tempVals(i) * 10)+1000);
    end
end

% Scale to 16-bit data and add auxilliary channel (typecast from unsigned 16-bit to signed 16-bit)
data16 = [int16(dataAccel(:,2:4) .* (32768 / scale)) typecast(aux,'int16') ];

% Audio information structure
ai = {};
ai.SampleRate = Fs;


% Device information (stored to artist)
% deviceInformation = struct('deviceType','AX3','deviceVersion',17,'deviceId',12345,'firmwareVersion',42);
str = '';
fields = fieldnames(deviceInformation);
for fn = fields'
    if ~isempty(str), str = strcat(str, ','); end
    name = fn{1};
    value = deviceInformation.(fn{1});
    if isnumeric(value), value = num2str(value); end
    str = strcat(str, name, '=', value);
end
ai.Artist = str;


% Subject information (stored to title)
% subjectInformation = struct('sessionId',sessionId,'startTime',startTime,'stopTime',stopTime,'accelRate',accelRate,'accelSensitivity',accelSensitivity,'metadata','"' + metadata + '"');
str = '';
fields = fieldnames(subjectInformation);
for fn = fields'
    if ~isempty(str), str = strcat(str, ','); end
    name = fn{1};
    value = subjectInformation.(fn{1});
    if isnumeric(value)
        if regexp(name, 'Time$')
            value = datestr(value, 'yyyy-mm-dd HH:MM:SS');
        else
            value = num2str(value);
        end
    end
    str = strcat(str, name, '=', value);
end
ai.Title = str;


% File information (stored to comment)
str = '';
str = strcat(str, 'ch0_type=accel,ch0_axis=x,ch0_unit=g,ch0_scale=', scale, ',');
str = strcat(str, 'ch1_type=accel,ch1_axis=y,ch1_unit=g,ch1_scale=', scale, ',');
str = strcat(str, 'ch2_type=accel,ch2_axis=z,ch2_unit=g,ch2_scale=', scale, ',');
str = strcat(str, 'ch3_type=aux');
ai.Comment = str;


% Write file
audiowrite(filename, data16, ai.SampleRate, 'BitsPerSample',16, 'Artist',ai.Artist, 'Title',ai.Title, 'Comment',ai.Comment);

