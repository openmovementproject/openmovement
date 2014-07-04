% Calculate SVM-1, with optional HP-filter
% Assumes tri-axial data with the format: [timestamp x y z] in units of 'g'
%
% Example:
%
% % Load CWA file re-sampled at 100Hz
% Fs = 100;
% data = resampleCWA('CWA-DATA.CWA', Fs);
% 
% % HP-Filtered SVM-1
% svm = SVM(data, Fs, 1);
%
% % Convert to 60 second epochs (sum of absolute SVM-1 values)
% epochSVM = epochs(abs(svm), 60 * Fs);
%
function ret = SVM(data, Fs, Fc1, Fc2, FN)

    if nargin < 3
        Fc1 = 0.5;  % First Cutoff Frequency (0.2 Hz or 0.5 Hz)
    end
    if nargin < 4
        Fc2 = 15;   % Second Cutoff Frequency (15 Hz)
    end
    if nargin < 5
        FN  = 4;    % Order (4)
    end

    % Calculate SVM-1
    svm = sqrt(data(:,2) .^ 2 + data(:,3) .^ 2 + data(:,4) .^ 2) - 1;

    % No filtering...
    if Fc1 <= 0 | Fc2 <= 0 | Fc2 <= Fc1 | FN <= 0
        filteredSVM = svm;
        
    else
        
        % Create Butterworth filter parameters
        %[B,A] = butter(FN, [Fc1, Fc2] ./ (Fs / 2)); 
        [B,A] = filterCoefficients(FN, Fc1, Fc2, Fs); 

        % Calculate filtered SVM
        %filteredSVM = filter(B, A, svm);
        filteredSVM = manualFilter(B, A, svm);
        
    end
	
    % Return filteredSVM
    ret = filteredSVM;

end
