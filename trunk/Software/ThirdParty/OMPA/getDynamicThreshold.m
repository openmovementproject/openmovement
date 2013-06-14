function thresholds = getDynamicThreshold(signal, wlen)
% calculates thresholds = [max, min, (max+min)/2]
% with a sliding window of length wlen
% wlen should be odd!

thresholds=zeros(length(signal),3);

b = ceil(wlen / 2);

for i=1:length(signal)-wlen+1,
    mi = min(signal(i:i+wlen-1));
    ma = max(signal(i:i+wlen-1));
    thresholds(i+b-1,:) = [mi ma (mi+ma)/2];
end

thresholds(1:b,:) =repmat(thresholds(b,:),b,1);
thresholds(end-b+1:end,:) =repmat(thresholds(end-b,:),b,1);
end
