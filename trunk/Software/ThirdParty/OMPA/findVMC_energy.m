% VMC_WET - Vector Magnitude, Work Energy Theorem Combined 
% Uses VMC where 60 sec epoch counts > 2453 and WET <=2453
% where   kcals = Total calories for a single epoch
%         counts = count level for single epoch
%         mass = body mass in Kg
%         scale = Epoch period (s) / 60
%
% Input:    mass = body mass in Kg
%           triaxial accelerometer data in row-wise fashion. 
%           E.g each row has an xaxis(col1), yaxis(col2), zaxis(col3)
%
% Output:   WET_Energy per epoch period in kcal
%            for non-valid counts, 0 is returned


function VMC_energy = findVMC_energy(mass,data)
    gps = findgps(data);
    counts = gps2count(gps);
    scale = 1/60; % we use 1 second epochs (ie. @100Hz, 100 samples are summed into 1 epoch)
    if(count > 2453)
        VMC_energy = scale * (0.00094*counts +(0.1346 * mass - 737418));
    else
        VMC_energy = counts * 0.0000191 * mass;     
    end
    
end