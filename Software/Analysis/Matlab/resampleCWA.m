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
	
	fprintf('\tinterpolating data\n');
    D = resampleACC(data, interpRate);
	
end

