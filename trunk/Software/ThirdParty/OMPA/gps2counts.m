% Notes:    Converts g/s to counts.
% Input:    Takes in g/sec values row-wise fashion. 
%           E.g each row has an time(col1), g/sec(col2)
% Output:   Actigraph counts in row-wise fashion

% HISTORY - (http://support.theactigraph.com/faq/counts)
% ActiGraph's original activity monitor, the 7164 model, 
% utilized a mechanical lever capable of measuring the 
% change in acceleration with respect to time (g/sec, where 
% g is gravity or 9.806 m/s2). To suppress unwanted motion 
% and enhance human activity, the acceleration signal was 
% passed through an analog band-pass filter, the output 
% of which yields a dynamic range of 4.26g/sec (+/-2.13g/sec) 
% at 0.75Hz (center frequency of the filter). Using a sample 
% rate of 10 samples-per-second, this filtered signal was then 
% digitized into 256 distinct levels by an 8-bit solid-state 
% analog-to-digital converter, producing 4.26g/sec per 256 
% levels or 0.01664 g/sec/count (each level is considered 1 count). 
% When each filtered sample is multiplied by the sample 
% window of 0.1sec, a resolution of 0.001664g/count is achieved.

function counts = gps2counts(gps)
%acc(m/s^2) = A(m) . freq(rad/s)

% Step 1 is to bandpass filter the signal.
% use a 4th order butterworth w0 0.2-15 Hz
[B A] = butter(4, [0.2/fs 15/fs],'stop');
filteredData = filter(B,A,data(:,2:4));

% rate limit the signal to ±2.13g/s

% now divivde into 256 levels normalised to range (±8g)

end

    


% Step 2 is to calculate the SVM
svm = findSVM(filteredData);

% Step 3 is to summarise 
numWholeSecs = floor(length(svm)/fs);
for i=1:numWholeSecs
    gps(i,1) = i;
    gps(i,2) = sum(svm(((i-1)*fs)+1:((i-1)*fs)+fs,1));    
end


% %% Bandpass
% fc1 = 6.31;
% %w1 = 2*pi*(fc1/fs);
% w1 = 2*pi*(fc1);
% fc2 = 1258.9;
% %w2 = 2*pi*(fc2/fs);
% w2 = 2*pi*(fc2);
% Q1 = 0.71;
% 
% num=[w2^2 0 0];
% den = [1 ((w2+w1)/Q1) (w2^2 + (w2*w1)/Q1^2 + w1^2) ((w1*w2^2+w1^2*w2)/Q1) (w1^2 * w2^2)];
% 
% 
% [BPB,BPA]=bilinear(num,den,fs);

