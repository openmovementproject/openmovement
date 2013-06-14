
%Epoch length must be specified as: secs,mins,hours,days

function SummarizedData = summarizeData(EpochLength,data)

startTime = data(2,1); %start at second time stamp as problem sometimes with 1st one...
stopTime = data(end,1);

if(strcmpi(EpochLength,'secs'))
    %secs - each row is a second so no need to find index's
    
    timeStamps = datestr(data(2:end,1),31); %get timstamps in ISO8601 format for Morris Graph - http://www.oesmith.co.uk/morris.js/lines.html
    data(1,:)=[]; % clear 1st row as full of rubbish

    for i=1:length(timeStamps)
        SummarizedData.epoch(i) = struct('t',timeStamps(i,:),'ee',data(i,2),'intensity',data(i,3),'steps',data(i,4),'dist',data(i,5));
    end
    
elseif(strcmpi(EpochLength,'mins'))
    % mins - matlab time divide by 1/24*60
    firstMin= startTime - mod(startTime,1/(24*60));
    lastMin = stopTime - mod(stopTime,1/(24*60));
    mins = firstMin:(1/(24*60)):lastMin;

    for i=1:length(mins)-1
        tmpData = data(data(2:end,1) >= mins(i) & data(2:end,1) <= mins(i+1),:);
 %       for j=1:length(tmpData)
            SummarizedData.epoch(i) = struct('t',datestr(mins(i),31),...
                                            'ee',sum(tmpData(:,2)),...
                                            'intensity_l',length(find(tmpData(:,3)==1)),...
                                            'intensity_m',length(find(tmpData(:,3)==2)),...
                                            'intensity_v',length(find(tmpData(:,3)==3)),...
                                            'steps',sum(tmpData(:,4)),...
                                            'dist',sum(tmpData(:,5)));
 %       end        
    end
    
elseif(strcmpi(EpochLength,'hours'))
    % hours - matlab time divide by 24
    firstHour = startTime - mod(startTime,1/24);
    lastHour = stopTime - mod(stopTime,1/24)+1;
    hours = firstHour:1/24:lastHour;

    for i=1:length(hours)-1
        tmpData = data(data(2:end,1) >= hours(i) & data(2:end,1) <= hours(i+1),:);
%        for j=1:length(tmpData)
            SummarizedData.epoch(i) = struct('t',datestr(hours(i),31),...
                                            'ee',sum(tmpData(:,2)),...
                                            'intensity_l',length(find(tmpData(:,3)==1)),...
                                            'intensity_m',length(find(tmpData(:,3)==2)),...
                                            'intensity_v',length(find(tmpData(:,3)==3)),...
                                            'steps',sum(tmpData(:,4)),...
                                            'dist',sum(tmpData(:,5)));
 %       end        
    end
    
elseif(strcmpi(EpochLength,'days'))
    % days - matlab time is fraction of days since 0000 
    firstDay = floor(startTime); 
    lastDay = ceil(stopTime);
    days = firstDay:lastDay;

    for i=1:length(days)-1
        tmpData = data(data(2:end,1) >= days(i) & data(2:end,1) <= days(i+1),:);
 %       for j=1:length(tmpData)
            SummarizedData.epoch(i) = struct('t',datestr(days(i),31),...
                                            'ee',sum(tmpData(:,2)),...
                                            'intensity_l',length(find(tmpData(:,3)==1)),...
                                            'intensity_m',length(find(tmpData(:,3)==2)),...
                                            'intensity_v',length(find(tmpData(:,3)==3)),...
                                            'steps',sum(tmpData(:,4)),...
                                            'dist',sum(tmpData(:,5)));
 %       end        
    end
    
else
    fprintf('Error: Invalid Epoch length specified/r/n');
    SummarizedData = 0;
    return;
end









