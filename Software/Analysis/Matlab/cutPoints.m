function cp = cutPoints(filename, varargin)
% 
%   cp = cutPoints(filename, varargin)
%
%   Obtain cut-points from the specified CWA file.
%
%   Input: 
%       filename               .CWA Filename
%
%   Optional arguments ('name',value):
%       cuts           vector  Mean-g cut-points
%       filter         0/1     Band-pass filter 0.25-20Hz (0=no)
%       calibrate      0/1     Auto-calibrate the signal (0=no)
%       resampleMethod         Resample method ('nearest', 'linear', 'pchip')
%       resampleRate   double  Resample rate (Hz)
%
%   Output:
%       cp   [Nx4]             Cut point values for each epoch time.
%
%   Example:
%
%       cuts = [ 77/80/60 220/80/60 2057/80/60 ];   % Esliger(40-63)-waist
%       cutPoints('CWA-DATA.CWA', 'cuts',cuts);
%       sum(ans)
%
%   Dan Jackson, '19
%

% 
% Copyright (c) 2014-19, Newcastle University, UK.
% All rights reserved.
% 
% Redistribution and use in source and binary forms, with or without 
% modification, are permitted provided that the following conditions are met: 
% 1. Redistributions of source code must retain the above copyright notice, 
%    this list of conditions and the following disclaimer.
% 2. Redistributions in binary form must reproduce the above copyright notice, 
%    this list of conditions and the following disclaimer in the documentation 
%    and/or other materials provided with the distribution.
% 
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

p = inputParser;
    
% define optional arguments with default values
addOptional(p,'cuts', [ 77/80/60 220/80/60 2057/80/60 ]); % Esliger(40-63)-waist
addOptional(p,'filter',         true); % Band-pass filter the data (0.5-20Hz)
addOptional(p,'calibrate',      true); % Auto-calibrate the data
addOptional(p,'resampleMethod', 'pchip');          % 'nearest', 'linear', 'pchip'
addOptional(p,'resampleRate',   100);              % Resample rate
addOptional(p,'calibrationHours', 7*24);           % Number of hours of data to consider for calibration (slow in Matlab)
addOptional(p,'window', 60);                       % Epoch window size (should be 60 seconds)

% parse varargin
parse(p,varargin{:});
p = p.Results;

% parse again
cuts = p.cuts;
filter = p.filter;
calibrate = p.calibrate;
resampleMethod = p.resampleMethod;
Fs = p.resampleRate;
calibrationHours = p.calibrationHours;
window = p.window;

% Read raw CWA data
fprintf('\tReading file data...\n');
rawData = CWA_readFile(filename);

if calibrate && calibrationHours > 0
    % Get samples from stationary periods (NOTE: only use first N hours of file)
    fprintf('\tFinding stationary periods (NOTE: only first %d hours)...\n', calibrationHours);
    S = getStationaryPeriods(rawData, 'stopTime', rawData.ACC(1,1)+calibrationHours/24, 'progress', 1, 'resampleRate',Fs, 'resampleMethod',resampleMethod);

    % Estimate the calibration parameters (scale, offset, temp-offset)
    fprintf('\tEstimating calibration...\n');
    e = estimateCalibration(S, 'verbose', 1);
    
    % Re-scale data
    fprintf('\tRescale data...\n');
    scaledData = rescaleData(rawData, e);
else
    scaledData = rawData;
end

% Interpolate to fixed interval
fprintf('\tInterpolating data...\n');
data = resampleACC(scaledData, Fs, resampleMethod);

% SVM-1
if filter
    fprintf('\tCalculating band-pass filtered (0.5-20Hz) SVM(data)...\n');
    svm = SVM(data, Fs, 0.5, 20);	% 0.5-20 Hz Bandpass
else
    fprintf('\tCalculating (non-filtered) SVM(data)...\n');
    svm = SVM(data, Fs, 0, 0);      % No band-pass filter
end

% Convert to 60 second epochs (sum of absolute SVM-1 values)
fprintf('\tEpochs of abs(SVM-1) over 60 seconds... (Any remaining partial window will be ignored)\n');
epochSVM = epochs(abs(svm), window * Fs);

% Convert to mean SVM value over epoch
fprintf('\tMean SVM-1...\n');
meanSVM = epochSVM ./ (window * Fs);

% Cut-point classification
fprintf('\tCut points...\n');
summary = zeros(size(meanSVM,1), size(cuts,2) + 1);
for i = 1:size(meanSVM, 1)
    % Decide on the classicfication for this epoch
    k = 1;
    for j = 1:size(cuts,2)
        if meanSVM(i) >= cuts(j)
            k = j + 1;
        end
    end
    summary(i, k) = summary(i, k) + 1;
end
cp = summary;
