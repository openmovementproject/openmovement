%
% Basic sum(SVM-1) computation with the AX3.
%
% You will need the following four files:
%
%   - AX3_readFile.m
%   - resampleCWA.m
%   - SVM.m
%   - epochs.m
%

% Load CWA file re-sampled at 100Hz
fprintf('Loading and resampling data...\n');
Fs = 100;
data = resampleCWA('CWA-DATA.CWA', Fs);

% HP-Filtered SVM-1
fprintf('Calculating HP-filtered SVM(data)...\n');
svm = SVM(data, Fs, 0.5);

% Convert to 60 second epochs (sum of absolute SVM-1 values)
epochSVM = epochs(abs(svm), 60 * Fs);

