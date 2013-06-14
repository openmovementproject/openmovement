% Copyright (c) 2009-2012, Newcastle University, UK.
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
% Cassim Ladha March 2013
%
% Wrapper to call the findPA function from Windows command line. This
% function takes in argumens in a fixed order and then calls the matlab
% the function findPA in "matlab style". The order for the input arguments
% is fixed and there is no error checking done here.
%
%   NOTES: To use with OM_PA compile this function as the main function and
%   findPA as a dependancy
%
%   Description of arguments:
%   filename - the cwa file to analyse
%   site - the location on the body the data was captured from
%   side - left or right side of the body
%   mass - of the person in Kg
%   bodyFat - as a percentage (ie number less than 1)
%   epoch - lenght of summary period
%   startTime - Time to begin in the file
%   stopTime - Time to stop in the file
%   js - name of the json object ('_' for none)
%   csv - name of the json object ('_' for none)
%   html - name of the json object ('_' for none)

%   Example Use With Start Times:
%   myPA = findPA_win32('sample.cwa',1,1,80,0.2,'mins','20-May-2011 10:00:00','20-May-2011 11:00:00','data.js','_','_');
%
%   Example Use Without Times:
%   myPA = findPA_win32('sample.cwa',1,1,80,0.2,'hours','fileStart','fileEnd','_','data.csv','_');
%
%   Example Use Just Start Time:
%   myPA = findPA_win32('sample.cwa', 1,1,80,0.2,'secs','21-May-2011 02:39:47','fileEnd','_','_','data.html');

function PAdata = findPA_win32(filename,site,side,mass,bodyFat,epoch,startTime,stopTime,js,csv,html)
try
    if(~strcmp(js,'_'))
        PAdata = findPA(filename,'site',str2num(site),'side',str2num(side),'mass',str2num(mass),'bodyFat',str2num(bodyFat),'epoch',epoch,'startTime',startTime,'stopTime',stopTime,'js',js);
    end
    if(~strcmp(csv,'_'))
        PAdata = findPA(filename,'site',str2num(site),'side',str2num(side),'mass',str2num(mass),'bodyFat',str2num(bodyFat),'epoch',epoch,'stopTime',stopTime,'csv',csv);
    end
    if(~strcmp(html,'_'))
        PAdata = findPA(filename,'site',str2num(site),'side',str2num(side),'mass',str2num(mass),'bodyFat',str2num(bodyFat),'epoch',epoch,'startTime',startTime,'stopTime',stopTime,'html',html);
    end
    
catch ex
    disp(ex.message)
    PAdata = -1;
    return;
end    





