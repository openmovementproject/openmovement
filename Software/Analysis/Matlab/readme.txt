%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Basic sum(SVM-1) computation with the AX3.
%
% You will need the following four files:
%
%   - https://raw.githubusercontent.com/digitalinteraction/openmovement/master/Software/Analysis/Matlab/CWA_readFile.m
%   - https://raw.githubusercontent.com/digitalinteraction/openmovement/master/Software/Analysis/Matlab/resampleCWA.m
%   - https://raw.githubusercontent.com/digitalinteraction/openmovement/master/Software/Analysis/Matlab/SVM.m
%   - https://raw.githubusercontent.com/digitalinteraction/openmovement/master/Software/Analysis/Matlab/epochs.m
%

% Load CWA file re-sampled at 100Hz
fprintf('Loading and resampling data...\n');
Fs = 100;
data = resampleCWA('CWA-DATA.CWA', Fs);

% BP-Filtered SVM-1
fprintf('Calculating bandpass-filtered SVM(data)...\n');
svm = SVM(data, Fs, 0.5, 15);

% Convert to 60 second epochs (sum of absolute SVM-1 values)
epochSVM = epochs(abs(svm), 60 * Fs);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Calibration Example script
%
% You will need the following four files:
%
%   - https://raw.githubusercontent.com/digitalinteraction/openmovement/master/Software/Analysis/Matlab/CWA_readFile.m
%   - https://raw.githubusercontent.com/digitalinteraction/openmovement/master/Software/Analysis/Matlab/getStationaryPeriods.m
%   - https://raw.githubusercontent.com/digitalinteraction/openmovement/master/Software/Analysis/Matlab/estimateCalibration.m
%   - https://raw.githubusercontent.com/digitalinteraction/openmovement/master/Software/Analysis/Matlab/rescaleData.m
%
%   this script demonstrates the use of the calibration routines for
%   OpenMovement AX devices (even though it should work with other accelerometers as
%   well). Results in data rescaled to unit gravity.
%
%
% read in data
rawData = CWA_readFile('CWA-DATA.CWA');

% get samples from stationary periods (at most first 7 days of file)
S = getStationaryPeriods(rawData, 'stopTime', rawData.ACC(1,1)+168/24, 'progress', 1);

% estimate the calibration parameters (scale, offset, temp-offset)
e = estimateCalibration(S, 'verbose', 1);

% re-scale data
data = rescaleData(rawData, e);

