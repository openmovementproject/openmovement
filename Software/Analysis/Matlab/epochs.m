% Calculate epochs of the specified bucket size.
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
function ret = epochs(data, bucketSize)
	
    % number of full buckets we can supply, do not include a partial bucket
    buckets = floor(length(data) / bucketSize);
    
    % last sample index for the end of the last bucket
    lastSample = buckets * bucketSize;  
    
    ret = zeros(buckets,1);
    for i = 1:buckets
        % Source index
        start = floor((i - 1) * lastSample / buckets) + 1;
        finish = floor(i * lastSample / buckets);
        if finish > length(data)
            finish = length(data);
        end
        ret(i) = sum(data(start:finish));    % / interpRate / 60
    end
    
end

