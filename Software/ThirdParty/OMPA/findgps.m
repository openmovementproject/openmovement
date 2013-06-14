% Notes:    Finds the ammount of g's per second based. 
% Input:    data =  Takes in triaxial accelerometer data in row-wise fashion.
%                   Assumes Col(1) is the time base and calculates fs from this at a constant
%                   rate
%                   E.g each row has a t(col1), xaxis(col2), yaxis(col3), zaxis(col4)
%
% Output:   gps =   rows for each second of data that continain g/s for each axis
%                   E.g col(1) Seconds, col2(x g/s), col3(y g/s), col4(z g/s)

function gps=findgps(data)
    %first of all find the sample rate based on the first 100 rows
    for i=2:100
        fs(i-1) = 1/((data(i,1) - data(i-1,1))*86400);
    end
    fs = round(mean(fs)); % in this function we need fs as an integer as we use it for indexing
    
    % now sum into 1 second chunks
    % NOTE: vector direction is maintained, therefore a mean zero result is
    % possible under device oscillation!!
    numWholeSecs = floor(length(data)/fs);
    for i=1:numWholeSecs
        gps(i,1) = i;
        gps(i,2) = sum(data(((i-1)*fs)+1:((i-1)*fs)+fs,2));
        gps(i,3) = sum(data(((i-1)*fs)+1:((i-1)*fs)+fs,3));
        gps(i,4) = sum(data(((i-1)*fs)+1:((i-1)*fs)+fs,4));
    end
end