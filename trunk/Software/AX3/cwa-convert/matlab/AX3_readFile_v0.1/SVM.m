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
function ret = SVM(data, Fs, filterFreq)

    % Calculate SVM-1
    svm = sqrt(data(:,2) .^ 2 + data(:,3) .^ 2 + data(:,4) .^ 2) - 1;

    % No filtering...
    if filterFreq == 0
        filteredSVM = svm;
        
    elseif filterFreq > 0
        
        % Filter parameters
        [B,A] = butter(9, filterFreq/(Fs/2), 'high'); 

        % Calculate filtered SVM
        filteredSVM = filter(B, A, svm);
        
    elseif filterFreq < 0

        % Discrete-time filter object.
        Fpass = -filterFreq;    % Passband Frequency
        Fstop = 0.5 * Fpass;    % Stopband Frequency
        Astop = 60;   % Stopband Attenuation (dB)
        Apass = 1;    % Passband Ripple (dB)

        h = fdesign.highpass('fst,fp,ast,ap', Fstop, Fpass, Astop, Apass, Fs);
        Hd = design(h, 'equiripple', 'MinOrder', 'any', 'StopbandShape', 'flat');
        filteredSVM = filter(Hd, svm);

    end
	
    % Return filteredSVM
    ret = filteredSVM;

end
