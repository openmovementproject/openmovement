%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Wave file test script
%
%   This script demonstrates writing accelerometer data to a .WAV file.
%

inputFilename = 'TEST.CWA';
    
% read in data
if ~exist('rawData', 'var')
    fprintf('\tloading file\n');
    rawData = AX3_readFile(inputFilename);
    clear('stationaryPeriods');
end

% get samples from stationary periods (first couple of hours of file)
if ~exist('stationaryPeriods', 'var')
    fprintf('\tgetting stationary periods\n');
    stationaryPeriods = getStationaryPeriods(rawData, 'stopTime', rawData.ACC(1,1)+2/24, 'progress', 1);
    clear('estimatedCalibration');
end

% estimate the calibration parameters (scale, offset, temp-offset)
if ~exist('estimatedCalibration', 'var')
    fprintf('\testimating calibration\n');
    estimatedCalibration = estimateCalibration(stationaryPeriods, 'verbose', 1);
    clear('rescaledData');
end

% re-scale data
if ~exist('rescaledData', 'var')
    fprintf('\trescaling data\n');
    rescaledData = rescaleData(rawData, estimatedCalibration);
    clear('resampledData');
end

% re-sample data
Fs = 100;
if ~exist('resampledData', 'var')
    fprintf('\tresampling data\n');
    resampledData = resampleACC(rescaledData, Fs);
end


% Prepare to save to WAV (the metadata should come from the file)
%   to convert back from formatted date:   timeValue = datenum('2001-12-31 23:59:59', dateFormat)
filename = 'test.wav';
scale = 8;

% TODO: Get these from the desired recording configuration (and not the actual samples)
startTime = rawData.ACC(1,1);
stopTime  = rawData.ACC(end,1);

deviceInformation = struct('deviceType','AX3', 'deviceVersion',17, 'deviceId',12345, 'firmwareVersion',42);
subjectInformation = struct('sessionId',0, 'startTime',startTime, 'stopTime',stopTime, 'accelRate',100, 'accelSensitivity',8, 'metadata','""');

% Save to WAV
wavWriteAccel(filename, resampledData, rawData.LIGHT, rawData.TEMP, Fs, scale, deviceInformation, subjectInformation);


% Read back file for testing
info2 = audioinfo(filename);
[data2, Fs2] = audioread(filename);
xyz2 = data2(:, 1:3);
aux = int16(data2(:, 4) .* 32768);

