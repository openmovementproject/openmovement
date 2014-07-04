% Load CWA file with optional re-sampling.
% Returns tri-axial data with the format: [timestamp x y z] in units of 'g'
%
% Example:
%
% % Load CWA file re-sampled at 100Hz
% Fs = 100;
% data = resampleCWA('CWA-DATA.CWA', Fs);
% 
function D = resampleCWA(filename, interpRate)
	
	fprintf('\treading file information\n');
	info = AX3_readFile(filename, 'verbose',1, 'info',1);
	
	startTime = info.validPackets(1,2);
	stopTime  = info.validPackets(end,2);
	
	fprintf('\treading file data\n');
	data = AX3_readFile(filename, 'validPackets', info.validPackets, 'startTime', startTime, 'stopTime', stopTime);
	
    if interpRate > 0
		% Remove any duplicate timestamps
		data.ACC = data.ACC(find(diff(data.ACC(:,1))>0),:);
		data.ACC = data.ACC(find(diff(data.ACC(:,1))>0),:);
		
		fprintf('\tinterpolating data\n')
		
		% gather and interpolate
		t = linspace(startTime, stopTime, (stopTime - startTime) * 24 * 60 * 60 * interpRate);
		D = zeros(length(t), 8);
		
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

