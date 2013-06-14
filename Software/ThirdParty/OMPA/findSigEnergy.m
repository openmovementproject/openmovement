% Notes:    Finds Signal Energy using a sliding window approach.
%
% Input     Takes in triaxial accelerometer data in row-wise fashion.
%           Assumes Col(1) is the time base and calculates fs from this at a constant
%           rate
%           E.g each row has a t(col1), xaxis(col2), yaxis(col3), zaxis(col4)% Output:   Standard vector mag (not removeing gravity component)
%
% Output    Energy as sliding window implementation

function energy = findSigEnergy(data)

    %first of all find the sample rate based on the first 100 rows
    for i=2:100
        fs(i-1) = 1/((data(i,1) - data(i-1,1))*86400);
    end
    fs = round(mean(fs)); % in this function we need fs as an integer as we use it for indexing

    
    if (length(data) < fs)
        fprintf('Not enough data. SVM must be larger than sample Freq');
    end
    
    % 50percent overlapping window
    wl = fs/2;
    
    for i = 1:length(data)-wl
        energy(i) = sum(sqrt(sum(data(i:i+wl-1,:).^2)),2);
    end
end