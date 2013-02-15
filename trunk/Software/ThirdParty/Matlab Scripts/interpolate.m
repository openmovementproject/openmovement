% Copyright (c) 2009-2013, Newcastle University, UK.
% All rights reserved.
% 
% Redistribution and use in source and binary forms, with or without 
% modification, are permitted provided that the following conditions are met: 
% 1. Redistributions of source code must retain the above copyright notice, 
%    this list of conditions and the following disclaimer.
% 2. Redistributions in binary form must reproduce the above copyright notice, 
%    this list of conditions and the following disclaimer in the documentation 
%    and/or other materials provided with the distribution.
% THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
% AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
% IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
% ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
% LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
% CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
% SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
% INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
% CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
% ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
% POSSIBILITY OF SUCH DAMAGE. 
%
% Re-interpolates Data to be suitable for Elan Analysis
% Cas Feb 2012
%
% Args in: 
%   fin - string e.g 'sampleData.csv'; Col 1 = Time, Col 2-4 = data
%   fout - string e.g 'newData.csv';
%   StartSamplesToTrim is the number of inclusive samples to chop out of begggining of the data
%   EndSamplesToTrim is the number of samples to chop from the end of the data
%   sampleFreq - is the sample frequency in Hz.
%
%   Example use: interpolate('inputFileName.csv','outputFileName.csv',0,100);
%%

function interpolate(fin,fout,samplesToTrim,sampleFreq)
% Load in CSV File
data = load(fin);

% Trim file
if samplesToTrim >1
%data(1:samplesToTrim,:) = [];
    data = data(samplesToTrim:end,:);
    data(:,1)=data(:,1)-data(1,1);
end

newTime = [0:length(data)-1]/sampleFreq;

% Interpolate between time stamps
%time = 0:sampleInc:length(data*sampleInc); % the timestamps, zero to last timestamp recorded

newdata = zeros(length(newTime),4);
newdata(:,1) = newTime;

for axis=2:4,
    newdata(:,axis) = interp1(data(:,1), data(:,axis), newTime, 'cubic', 0); % interpolate each axis
end

% Save file
dlmwrite(fout,newdata,'Delimiter',',','Precision', '%.6f') % put in any c-type precision

end