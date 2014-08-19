% Load CWA file with optional re-sampling.
% Returns tri-axial data with the format: [timestamp x y z] in units of 'g'
%
% Example:
%
% % Load CWA file re-sampled at 100Hz
% Fs = 100;
% data = AX3_readFile('CWA-DATA.CWA');
% data.ACC = resampleACC(data, Fs);
% 
function D = resampleACC(data, interpRate)
    
    if interpRate > 0
		% Remove any duplicate timestamps
		data.ACC = data.ACC(find(diff(data.ACC(:,1))>0),:);
		data.ACC = data.ACC(find(diff(data.ACC(:,1))>0),:);
	
        startTime = data.ACC(1,1);
        stopTime  = data.ACC(end,1);
    
		% gather and interpolate
		t = linspace(startTime, stopTime, (stopTime - startTime) * 24 * 60 * 60 * interpRate);
		D = zeros(length(t), 4);
		
		D(:,1) = t;
		for a=2:4,
			D(:,a) = interp1(data.ACC(:,1),(data.ACC(:,a)),t,'cubic',0);
		end
		
		% D is now the interpolated signal with time-stamps @interpRate
		
    else
		% Pass-through data with no interpolation
		D = data.ACC;
    end
	
end

