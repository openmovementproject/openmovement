% PAIT - Physical Activity Intensity Time Estimation
% Notes:    Uses the techniques described in 
% http://www.geneactiv.co.uk/media/1677/esliger_et_al_2011.pdf
%
% Input:    data  =     triaxial accelerometer data in row-wise fashion. 
%                       E.g each row has an time(col1), xaxis(col2), yaxis(col3), zaxis(col4)
%           epoch =     period in seconds, over which to make epoch summaries
%           location =  site where sensor is mounted: ('wrist' or 'waist')
%           side =      'left' or 'right' side if appropriate. 0 if
%                       inappropriate
%
% Output:   Time series matrix in chunks of epoch period with appropriate
%           intensity: 1 = sendentry, 2 = light, 3 = moderate, 4 = vigorous
%
% Example use (with side):      findPAIT(data,60,'wrist','left')
% Example use (without side):   findPAIT(data,60,'waist',0)

function PAIT  = findPAIT(data,location,side)

    % Step 1 - preprocessing and sanity checks
    %Initialise PAIT
    PAIT=0;
    
    %estimate sample frequency over 1st 100 samples
    for i=2:101
        fs(i-1) = 1/((data(i,1) - data(i-1,1))*86400); 
    end
    
    fs = round(mean(fs));    
    
    % Sanity check data first - we need at least 60 seconds of data
    if (size(data) < (fs*60))
        fprintf('Not enough data for this method');
        return;
    end

    % Sanity  check sensor location 
    %                               wrist = 1,Waist = 2 
    %                               Left = 1, right = 2                       
    
    if(~((location==1) || (location==2)))
        fprintf('Invalid mounting site specified. Only "waist" or "wrist" currenty supported');
        return;
    end
    
    if(~((side==1)||(side==2)))
        fprintf('Invalid side specified. Only "left" or "right" currenty supported');
        return;
    end
   
    % Step 2 is to bandpass filter the signal.
    % use a 4th order butterworth w0 0.2-15 Hz
    Fpass = 0.2;         % Passband Frequency
    Fstop = 15;          % Stopband Frequency
    Apass = 1;           % Passband Ripple (dB)
    Astop = 80;          % Stopband Attenuation (dB)
    match = 'stopband';  % Band to match exactly

    % Construct an FDESIGN object and call its BUTTER method.
    h  = fdesign.lowpass(Fpass, Fstop, Apass, Astop, fs);
    Hd = design(h, 'butter', 'MatchExactly', match);

    filteredData = filter(Hd,data(:,2:4));

    %[B A] = butter(4, [0.2/fs 15/fs],'stop');
    %filteredData = filter(B,A,data(:,2:4));

    % Step 3 is to calculate the SVM and remove gravity
    svm = abs(findSVM(filteredData) - 1);

    % Step 4 is to summarise into mins and remove gravity
    numWholeMins = floor(length(svm)/(fs*60));
    for i=1:numWholeMins
        gpmin(i,1) = data(i*fs*60);
        gpmin(i,2) = sum(svm(((i-1)*fs*60)+1:((i-1)*fs*60)+fs*60,1));   %note - abs = we do not discriminate orientation
                                                                    
    end

    % Step 5 is apply the cutpoints found in the paper
    for i=1:numWholeMins
        PAIT(i,1)=gpmin(i,1);
    end
    
    if((location == 1) && (side ==1))
        %left wrist
        for i=1:numWholeMins
            if(gpmin(i,2) < 217)
                PAIT(((i-1)*fs*60)+1:((i-1)*fs*60)+(fs*60),2) = 1;
            elseif(gpmin(i,2) < 644)
                PAIT(((i-1)*fs*60)+1:((i-1)*fs*60)+(fs*60),2) = 2;
            elseif(gpmin(i,2) < 1810)
                PAIT(((i-1)*fs*60)+1:((i-1)*fs*60)+(fs*60),2) = 3;
            else
                PAIT(((i-1)*fs*60)+1:((i-1)*fs*60)+(fs*60),2) = 4;
            end
        end
                
    elseif((location == 1) && (side ==2))
        %right wrist
        for i = 1:numWholeMins
            if(gpmin(i,2) < 386)
                PAIT(((i-1)*fs*60)+1:((i-1)*fs*60)+(fs*60),2) = 1;
            elseif(gpmin(i,2) < 439)
                PAIT(((i-1)*fs*60)+1:((i-1)*fs*60)+(fs*60),2) = 2;
            elseif(gpmin(i,2) < 2098)
                PAIT(((i-1)*fs*60)+1:((i-1)*fs*60)+(fs*60),2) = 3;
            els
                PAIT(((i-1)*fs*60)+1:((i-1)*fs*60)+(fs*60),2) = 4;
            end
        end
    elseif(location == 2)
        %waist. left and right the same
        for i = 1:numWholeMins
            if(gpmin(i,2) < 77)
                PAIT(((i-1)*fs*60)+1:((i-1)*fs*60)+(fs*60),2) = 1;
            elseif(gpmin(i,2) < 219)
                PAIT(((i-1)*fs*60)+1:((i-1)*fs*60)+(fs*60),2) = 2;
            elseif(gpmin(i,2) < 2056)
                PAIT(((i-1)*fs*60)+1:((i-1)*fs*60)+(fs*60),2) = 3;
            else
                PAIT(((i-1)*fs*60)+1:((i-1)*fs*60)+(fs*60),2) = 4;
            end
        end
    end

end