% WET - Work Energy Theorem
% Notes:    Uses the equation:
%           kcals = counts * 0.0000191 * mass
%           where   kcals = Total calories for a single epoch
%           counts = count level for single epoch
%
% Input:    mass  = body mass in Kg
%           data  = triaxial accelerometer data in row-wise fashion. 
%                   E.g each row has an time(col1), xaxis(col2), yaxis(col3), zaxis(col4)
%           epoch = period over which to make epoch summaries
%
% Output:   WET_Energy per epoch period in kcal

function WET_energy  = findWET_energy(mass,data,epochSecs)

    %Initialise WET_energy
    WTV(1:length(data))=0;
    
    %SVM = findSVM(data(:,2:4)) -1; % remove gravity from this SVM
    
    %estimate sample frequency over 1st 100 samples
    for i=2:101
        fs(i-1) = 1/((data(i,1) - data(i-1,1))*86400);   
    end
    fs = round(mean(fs));    
    
    % Sanity check data first    
    if (length(data) < fs)
        fprintf('Not enough data for this method\n');
        return;
    end
    
    % get the g/s for each axis
    gps = findgps(data);
    
    % get the magnitude of g/s over all axis
    gps_mag = abs(sum(gps(:,2:4)));
    
    % work out the number of counts
    counts = gps2count(gps_mag);
    
    % calculate WET energy per second
    WET_secs = counts * 0.0000191 * mass;
    
    % sum the WET energy into epoch's
    numEpochs = length(WET_secs)/epochSecs;
    
    for i=1:numEpochs
        WET_energy(i) = sum(WET_secs((i-1)*epochSecs + 1:i*epochSecs));
    end
    
end
