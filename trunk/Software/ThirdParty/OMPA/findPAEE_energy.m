% PAEE - Physical Activity Energy Expendiature
% Notes:    Uses the techniques described in 
% http://www.ncbi.nlm.nih.gov/pmc/articles/PMC3146494/
%
% Input:    mass  = body mass in Kg
%           data  = triaxial accelerometer data in row-wise fashion. 
%                   E.g each row has an time(col1), xaxis(col2), yaxis(col3), zaxis(col4)
%           epoch = period over which to make epoch summaries
%
% Output:   PAEE_energy per epoch period in kcal

function PAEE_energy  = findPAEE_energy(mass,bodyFat,data)

%estimate sample frequency over 1st 100 samples
    for i=2:101
        Fs(i-1) = 1/((data(i,1) - data(i-1,1))*86400);   
    end
    Fs = round(mean(Fs)); 
    
    
% Step 1 is to bandpass filter the signal.
% use a 4th order butterworth w0 0.2-15 Hz
Fpass = 0.2;         % Passband Frequency
Fstop = 15;          % Stopband Frequency
Apass = 1;           % Passband Ripple (dB)
Astop = 80;          % Stopband Attenuation (dB)
match = 'stopband';  % Band to match exactly

% Construct an FDESIGN object and call its BUTTER method.
h  = fdesign.lowpass(Fpass, Fstop, Apass, Astop, Fs);
Hd = design(h, 'butter', 'MatchExactly', match);

filteredData = filter(Hd,data(:,2:4));

% Step 2 is to calculate the SVM
svm = findSVM(filteredData);

% Step 3 is to summarise 
%numWholeSecs = floor((data(end,1)-data(1,1))*86400);
numWholeSecs = floor(length(svm)/Fs) -1 ;
for i=2:numWholeSecs
    gps(i,1) = data(i*Fs,1);
    gps(i,2) = mean(svm(((i-1)*Fs)+1:((i-1)*Fs)+Fs,1));    
end

% Step 4 make the jump from g/s to MJs. We also dont discriminate Sex in
% this method as http://www.geneactiv.co.uk/media/1677/esliger_et_al_2011.pdf states men move similarly to women
% Using Van Hees Mapping in Table 3 in model 2 that incorporates weight (Kg)
% Van Heess States that TEE(total energy expendiatire) is calculated:
% MJ/day = (mass * 0.019)(22.553 * g_perDay -0.873)
% MJ/sec = {(mass * 0.019)(22.553 * g_perDay -0.873)}/(24h * 60min * 60s)

for(i = 1:numWholeSecs)
    MJs(i) = ((mass*0.019)*((22.553*gps(i,2)) - 0.873))/(86400);

%    MJs(i) = (mass*(22.553*gps(i,2))-0.873)/(24*60*60); %divide at end to get MJ/24h -> MJ/s
end

% Step 5 - estimate the Physical Activity component of each measurement.
% equations from http://ajcn.nutrition.org/content/35/3/566.full.pdf+html
% for energy expendiature
%RMR - resting metabolic rate (kJ/24h) , also known as REE
%FMF - fat free mass (Kg)
%24EE - Energy expenditure in 24h (same as TEE) (kJ)
%RMR = 1971 + 87.1*FFM
%24EE = 3782 + 99.3*FFM
%24EE = 2920+0.94*RMR
%and another method with combined methods
%24EE = 2754 + 0.52*RMR + 53.8*FFM
REE = (1971 + 87.1* mass * (1-bodyFat)) / 86400; 

% From http://www.ncbi.nlm.nih.gov/pmc/articles/PMC3146494/
%TEE - Total Energy Expenditure
%REE - Resting Energy Expenditure
%PAEE - Physical Activity Energy Expenditure
%PAEE = 0.9*TEE-REE;

%Reported in Joules                   kj->Mj    MJ->J
PAEE_energy = [gps(:,1) (0.9*MJs' - REE/1000)*1000000];
PAEE_energy(1:2,2) = PAEE_energy(3,2);

%PAEE_energy = PAEE_energy(3:end,:);


end