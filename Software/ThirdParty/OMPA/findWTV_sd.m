% WTV - Wear Time Validation using standard deviation Accelerometer
%
% Based on http://www.ncbi.nlm.nih.gov/pmc/articles/PMC3146494/
%
% "non-wear time was estimated on the basis of the standard deviation and
% the value range of each accelerometer axis, calculated for consecutive
% blocks of 30 minutes. A block was classified as non-wear time if the
% standard deviation was less than 3.0 mg (1 mg=0.00981 m·s?2) for at least
% two out of the three axes or if the value range, for at least two out of
% three axes, was less than 50 mg. Thresholds were based on lab experiments
% in which thirty GENEA accelerometers were left motionless on a flat,
% stable surface for 30 minutes, showing that the standard deviation of an
% acceleration signal (which has some inherent noise) is 2.6 mg during
% non-motion. Therefore, the 3.0 mg threshold will allow a maximal increase
% of 0.4 mg in the standard deviation, which when expressed in angular
% orientation of the acceleration sensor corresponds to a standard
% deviation of 1.6 degrees []. Phan et al. showed that the acceleration of
% the chest in a resting person resulting from the breathing movement alone
% has an amplitude of 10 mg, while the vibrations resulting from the heart
% beat have a peak-to-peak amplitude of 80 mg [16]. Therefore, even the
% tiniest wrist movements are likely to be picked up by the method as
% described above. 
%
%   Input:    acc -       accelerometer data in the format
%
%                       col1(time),col2(x),col3(y),col4(z) 
%
%           thresh -    a threshold on the std (in g)a
%                       needed to make a decision about wear time.
%                       NOTE: typical value is 0.3
%
%           period -    the chunk we investigate the threshold for (in
%                       seconds). typical is 30 mins or 1800 seconds
%                        
%
% Output:   Returna a n,1 matrix where:
%            1 - indicates the device was probably being worn
%            0 - the device was probably not worn
%            n - is the dimension of the original data
%
% Example Use: WTV = findWTV_sd(data, 1.01, 10)

function WTV = findWTV_sd(data,thresh,period)

% Step 1 - preprocessing and sanity checks
%Initialise WTV
    WTV(1:length(data))=0;
    
    %estimate sample frequency over 1st 100 samples
    for i=2:101
        fs(i-1) = 1/((data(i,1) - data(i-1,1))*86400); 
    end
    
    fs = round(mean(fs));    
    
    % Sanity check data first - we need at least 30 mins (1800 seconds)   
    if (length(data) < (fs*period))
        fprintf('Not enough data for this WTV_sd method\n');
        return;
    end
    
    
% Step 2 - find std for each 30 min chunk
    numChunks = floor(length(data)/(fs*period));
    for i=1:numChunks
        stdepoch(i,1) = i;
        stdepoch(i,2) = std(data(((i-1)*(fs*period))+1:((i-1)*(fs*period))+(fs*period),2));
        stdepoch(i,3) = std(data(((i-1)*(fs*period))+1:((i-1)*(fs*period))+(fs*period),3));
        stdepoch(i,4) = std(data(((i-1)*(fs*period))+1:((i-1)*(fs*period))+(fs*period),4));
    end

    % calculate the remaining minuites at the end
    i = i+1;
    stdepoch(i,1) = i;
    stdepoch(i,2) = std(data(((i-1)*(fs*period))+1:end,2));
    stdepoch(i,3) = std(data(((i-1)*(fs*period))+1:end,3));
    stdepoch(i,4) = std(data(((i-1)*(fs*period))+1:end,4));
    
% Step 3 - look at sd in each chunk and threshold. if below threshold mark
% the data as invalid. We must base the test on 2 out of 3 axis must be
% below to be invalid.
    for i=1:size(stdepoch)
        if(((stdepoch(i,2) > thresh) && (stdepoch(i,3) > thresh)) ||...
           ((stdepoch(i,2) > thresh) && (stdepoch(i,4) > thresh)) ||...
           ((stdepoch(i,3) > thresh) && (stdepoch(i,4) > thresh)))
           
             WTV(((i-1)*(fs*period))+1:((i-1)*(fs*period))+(fs*period)) = 1; % Valid wear
        else
             WTV(((i-1)*(fs*period))+1:((i-1)*(fs*period))+(fs*period)) = 0; % Invalid wear
        end
    end
    
% Step 4 - get into right dimensions and shape (a bit hacky at the moment)
    WTV = WTV'; %transpose to be in colomn vector
    WTV(size(data)+1:end) = []; %trim of the bits that result from not 
                                      % fitting exactly in the period window. We return a colomn vector the same length as 'data'
end