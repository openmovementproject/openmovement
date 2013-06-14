% findSteps - Finds steps in triaxial accelerometer signal based on a
% simple falling flank crossing a threshold
% Notes:    Uses the techniques described in 
%
% Input:    data  =     triaxial accelerometer data in row-wise fashion. 
%                       E.g each row has an time(col1), xaxis(col2), yaxis(col3), zaxis(col4)
%           location =  site where sensor is mounted: ('wrist' or 'waist')
%           side =      'left' or 'right' side if appropriate. 0 if
%                       inappropriate
%
% Output:   steps = col1 - (time step was detected)
%                   col2 - (distance of the step in meters)
%
% Example use (with side):      findSteps(data)

function [F2steps] = findSteps(data)

% config:
smoothWindow    = 20;   % Taken from PanTompkins Paper //window used for smoothing
F2steps = []; %preload in case none found

verbose         = 0;

stepThreshold = 0.1; % this is an amplitude threshold for valid steps. usefull in dogs

%estimate sample frequency over 1st 100 samples
if data(1,1) > 730486
    % we are using matlab time. (730486 is jan 1st 2000)
    matlabTime = 1;
    for i=2:101
        fs(i-1) = 1/((data(i,1) - data(i-1,1))*86400); 
    end
 
else
    matlabTime = 0;
    for i=2:101
        fs(i-1) = 1/(data(i,1) - data(i-1,1)); 
    end
end
fs = round(mean(fs)); 
thresWindow     = round(fs/2)+1;   % window used for threshold estimation (max/min)
%thresWindow     = round(fs*2);   % window used for threshold estimation (max/min)
energyWindow    = fs+1;  % window used to calculate energy

% this must be a single stream eg SVM of triaxial
s = findSVM(data(:,2:4));

% smooth signal
s = smooth(s, smoothWindow);

% get thresholds (max/min/middle)
thres = getDynamicThreshold(s, thresWindow);

% remember middle line as that one is used to detect steps.
d = thres(:,3);

% initialize step-counter. Will be filtered later on
steps = [];

a = s(1:size(thres,1)) - thres(:,3);
c = (a(1:end-1) > 0) & (a(2:end)<=0) & (a(1:end-1)>a(2:end));

steps = find(c);
steps = [steps thres(steps,3)];

%for i=1:length(s)-1,
%    % get crossing points (falling flanks) of signal versus
%    % middle between max & min
%    if s(i) > d(i) & s(i) > s(i+1) & s(i+1) < d(i+1),
%        % add to stepcounter
%        % position (time) and (rough) point where middle is crossed
%        steps = [steps ; [i (s(i)+s(i+1))/2]];
%   end 
%end;



% get out of here if not many steps are found (filtering useless then).
if nnz(steps(:,1)) < 2 
    fprintf(1,'Error: barely any steps found ... \n');
    return; 
else
    if verbose == 1,
        fprintf(1,'Number of falling flank step candidates found: %d\n',length(steps));
    end
end

% First filter is a very simple threshold on the signal energy.
% Get steps with energy > stepThreshold

% For some reason I made every window zero mean before energy is computed ... maybe not necessary
halfWindow = ceil(energyWindow/2);
sss = [zeros(halfWindow,1) ;  s ; zeros(halfWindow,1)]; % just a temporary helper
e = zeros(size(ss));
for i=1:length(sss)-energyWindow,
    e(i+((round(fs/2))+1)) = sum((abs((sss(i:i+energyWindow-1) - mean(sss(i:i+energyWindow-1))))) .^ 2);
    %e(i+((round(fs/2))+1)) = sum((abs((sss(i:i+energyWindow-1)))) .^ 2);
end;
    
stepEnergies = e(steps(:,1));
validSteps = find(stepEnergies > stepThreshold);
steps = steps(validSteps,:);

% get out of here if not many steps are found (filtering useless then).
if nnz(steps(:,1)) < 2 
    if verbose == 1,
       fprintf(1,'no steps over the threshold ... \n');
    end
    return; 
else
    if verbose == 1,
        fprintf(1,'Steps over energy threshold: %d\n',length(steps));
    end
end


% convert to matlab time
steps(:,1) = data(steps(:,1),1);

% Here we do the filtering of the found steps accoring to Analog Devices AN602
% Filter 1 is a "Time Window" : We can only take a certain ammount of steps
% per in a certain time window. 
% AN602 details this should be hard coded to 0.2sec < tStep < 2sec
%
% In terms of our sampling rate this means step seperation distance is:
% 20samples < tStep < 200samples

% CAS - I have empiricaly proved these knees are ok in both the sedentry
% type walking and energetic running

F1steps = [];
for i=2:length(steps)
    %tSteps = steps(i,1) - steps(i-1);
    if(matlabTime)
        tSteps = ((steps(i,1) - steps(i-1,1))*86400); %time between steps in seconds
    else
        tSteps = steps(i,1) - steps(i-1,1); %time between steps in seconds    
    end
    if(tSteps > 0.20 && tSteps < 2.00)
    F1steps = [F1steps; steps(i,1)];
    end
end

if verbose == 1,
    fprintf(1,'Steps after TimeWindow filtering: %d\n',length(F1steps));
end

%Filter 2 is a "Periodicity Regulation"
% The filter is designed to take out "one off" bouts and only count timely steps
% that are part of a valid walk. A period window of ±15percent either way of the
% mean was suggested by AN602. After my own analysis, I found the following
% Filter Width    ±tollerance
%   32              27.3%
%   16              26.8%
%   8               30.3%
%   4               35.2%
%   2               42.7%

% I choose to use a 16 step historical filter

%preload
%figure;hold on;
%tFromLastStep = [70;70;70;70;70;70;70;70;70;70;70;70;70;70;70;70;70;70;70;70;70;70;70;70;70;70;70;70;70;70;70;70;70;70;70];%in samples!!
%n=2;
%Periodicity=[];
%for i=n+1:length(F1steps)
%    tFromLastStep(i) = F1steps(i,1) - F1steps(i-1);
%   Periodicity(i,1) = tFromLastStep(i)/((1/n)*sum(tFromLastStep(i-n:i)));
%end
%hist(Periodicity,100);
%h = findobj(gca,'Type','patch');
%set(h,'FaceColor','b','EdgeColor','b')

F2steps = [];
%preload in samples
tFromLastStep = [70;70;70;70;70;70;70;70;70;70;70;70;70;70;70;70;70];
for i = 9:length(F1steps)
    %tFromLastStep(i) = F1steps(i,1) - F1steps(i-1);
    if(matlabTime)
        tFromLastStep(i) = ((F1steps(i,1) - F1steps(i-1,1))*86400); %time between steps in seconds
    else
        tFromLastStep(i) = F1steps(i,1) - F1steps(i-1,1); %time between steps in seconds
    end
    meanPast8Steps = mean(tFromLastStep(i-8:i-1));
    stepPeriodDeviation = tFromLastStep(i) * 0.3; 
    if(((meanPast8Steps + stepPeriodDeviation) > tFromLastStep(i)) && ((meanPast8Steps - stepPeriodDeviation) < tFromLastStep(i)))
        F2steps = [F2steps; F1steps(i,1)];
    end
end

if verbose == 1,
    fprintf(1,'Steps after Periodicity Regulation: %d\n',length(F2steps));
end

% Next process is to calculate stride length. This is done according to
% AN602:
% distance = (Amax * Amin)^0.25 .* n * k
% where Amax/Amin is the max/min Acc measured in a single stride, n is the
% number of steps and k is a const to get into meters or feet
% As we already have the Dynamic Thresholds Calculated we can just use
% those
% formatted as -> thres(min,max,middle)
for i=1:length(F2steps)
    F2steps(i,2) = (thres(i,2)* thres(i,1))^0.25*1; %k=1 to get into meters
end

% plot the stuff so we see what is going on
 if verbose == 1,
    figure; plot(e); title('energy');
     figure; hold on;
     plot(data(:,1),s,'r-');
     plot(data(:,1),thres(:,1),'g--')
     plot(data(:,1),thres(:,2),'g--')
     plot(data(:,1),d,'g--')
     plot(F2steps(:,1),F2steps(:,2),'ko');
     title('Signal, smoothed signal, thresholds and steps.')
     
     fprintf(1,'Final Steps detected: %d\n',size(F2steps,1));
     fprintf(1,'Distance traveled: %d\n',sum(F2steps(:,2)));
 end

%if verbose == 1,
%   fprintf(1,'Final Steps detected: %d\n',size(F2steps,1));
%   if size(F2steps,1) > 1
%        fprintf(1,'Distance traveled: %d\n',sum(F2steps(:,3)));
%   end
%end




end
