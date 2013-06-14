% WTV - Wear Time Validation
% Finds the periods where the device is likely to be worn based on a 
% threshold of the SVM of data
% Input:    acc -       accelerometer data in the format
%                       col1(time),col2(x),col3(y),col4(z) 
%
%           thresh -    a threshold on the SVM (in g)
%                       needed to make a decision about wear time.
%                       NOTE: 'thresh' excludes gravity and therefore
%                       the value is typicaly < 1.
%
%           period -    the acceptable gap (in seconds) where the data
%                       is permitted to fall below the threshold
%                        
%
% Output:   Returna a n,1 matrix where:
%            1 - indicates the device was probably being worn
%            0 - the device was probably not worn
%
% Example Use: WTV = findWTV(data, 1.01, 10)
% 
% TODO - Make into a proper sliding window. This method currently just
% chops into segments and checks each segment

% http://www.ncbi.nlm.nih.gov/pmc/articles/PMC3146494/
%Accelerometer non-wear time was estimated on the basis of the standard
%deviation and the value range of each accelerometer axis, calculated for
%consecutive blocks of 30 minutes. A block was classified as non-wear time
%if the standard deviation was less than 3.0 mg (1 mg=0.00981 m·s?2) for at
%least two out of the three axes or if the value range, for at least two
%out of three axes, was less than 50 mg. Thresholds were based on lab
%experiments in which thirty GENEA accelerometers were left motionless on a
%flat, stable surface for 30 minutes, showing that the standard deviation
%of an acceleration signal (which has some inherent noise) is 2.6 mg during
%non-motion. Therefore, the 3.0 mg threshold will allow a maximal increase
%of 0.4 mg in the standard deviation, which when expressed in angular
%orientation of the acceleration sensor corresponds to a standard deviation
%of 1.6 degrees []. Phan et al. showed that the acceleration of the chest
%in a resting person resulting from the breathing movement alone has an
%amplitude of 10 mg, while the vibrations resulting from the heart beat
%have a peak-to-peak amplitude of 80 mg [16]. Therefore, even the tiniest
%wrist movements are likely to be picked up by the method as described
%above.

function WTV = findWTV(data,thresh,period)
    
    %Initialise WTV
    WTV(1:length(data))=0;
    
    SVM = findSVM(data(:,2:4)) -1; % remove gravity from this SVM
    
    %estimate sample frequency over 1st 100 samples
    for i=2:101
        fs(i-1) = 1/((data(i,1) - data(i-1,1))*86400);
        
    end
    fs = round(mean(fs));    
    
    % Sanity check data first    
    if (length(SVM) < fs)
        fprintf('Not enough data for this method\n');
        return;
    end
    
    %initialise variable
    period_counter = 0; % note the period counter counts in SAMPLES but we check threshold in seconds
    samplesInPeriod = period*fs;
    samplesInWindow = samplesInPeriod/2; %50 percent overlap
    
    for i=1:floor(length(SVM)/samplesInPeriod)

        if(abs(sum(SVM((i-1)*samplesInPeriod + 1:i*samplesInPeriod))) > thresh*samplesInPeriod)
            %if here we are ok - mark data as valid
            WTV((i-1)*samplesInPeriod + 1:i*samplesInPeriod) = 1;
        else
            %if here we have been below the threshold for over period
            % so mark data as invalid. Note - this method does not go
            % back and retrospectively mark up the previous samples before this
            % decision

            WTV((i-1)*samplesInPeriod + 1:i*samplesInPeriod) = 0;
        end
    end
    WTV = WTV';
end
