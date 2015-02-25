% Calculate SVM-1, with optional HP-filter
% Assumes tri-axial data with the format: [x y z] or [timestamp x y z] in units of 'g'
%
% Example:
%
% % Load CWA file re-sampled at 100Hz
% Fs = 100;
% data = resampleCWA('CWA-DATA.CWA', Fs);
% 
% % BP-Filtered SVM-1
% svm = SVM(data, Fs, 1);
%
% % Convert to 60 second epochs (sum of absolute SVM-1 values)
% epochSVM = epochs(abs(svm), 60 * Fs);
%
function svm = SVM(data, Fs, Fc1, Fc2, FN)

    % Defaults for unspecified arguments
    if nargin < 2; Fs = 100; end    % 100 Hz
    if nargin < 3; Fc1 = 0.5; end   % First Cut-off Frequency (0.2 Hz or 0.5 Hz)
    if nargin < 4; Fc2 = 15; end    % Second Cut-off Frequency (15 Hz)
    if nargin < 5; FN  = 4; end     % Order (4)

    % Calculate SVM-1 on last three columns: sqrt(x^2 + y^2 + z^2) - 1;
    svm = sqrt(sum(data(:, end-2:end) .^ 2, 2)) - 1;

    % Only filter if valid args
    if Fs > 0 && Fc1 > 0 && Fc2 > 0 && Fc2 > Fc1 && FN > 0
        % Create Butterworth filter parameters
        [B,A] = butter(FN, [Fc1, Fc2] ./ (Fs / 2)); 

        % Calculate filtered SVM
        svm = filter(B, A, svm);
    end
	
end
