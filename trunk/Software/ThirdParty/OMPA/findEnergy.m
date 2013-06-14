% input xyz - x y and z data in g
% input sampleFreq in Hz
% output Energy as sliding window implementation
function energy = findEnergy(xyz,sampleFreq)
    if (size(xyz) < sampleFreq)
        fprintf('Not enough data. SVM must be larger than sample Freq\r\n');
    end
    
    wl = sampleFreq/2;
    
    for i = 1:length(xyz)-wl
        energy(i) = sum(sqrt(sum(xyz(i:i+wl-1,:).^2)),2);
    end
end