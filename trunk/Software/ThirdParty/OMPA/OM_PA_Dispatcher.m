function OM_PA_Dispatcher(str)
parts =  rsplit(' ',str);

if (length(parts) == 0)
    fprintf('s Optional arguments are: -site -side, -mass -bodyFat -epoch -out \r\n. Eg OM_PA.exe "InputFile.CWA" -site 1 -side 1 -mass 80 -bodyFat 0.2 -epoch 60 -out "c:\output.CSV" \r\n'); 
    return
end

if (iseven(length(parts)))
    fprintf('error: Number of input argument does not equal number of actual arguments\r\n')
    return
end

if ~(strcmpi(parts{1}(end-2:end),'CWA'))
    fprintf('error: input file is not a cwa file\r\n')
    return
end
site = 1;
side = 1;
mass = 80;
bodyFat = 0.22;
epoch = 60;
csv = 'PAdata.csv';

for i=2:length(parts)
        %OM_PA.exe "InputFile.CWA" -site 1 -side 1 -mass 80 -bodyFat 0.2 -epoch 60 -out "c:\output.CSV"
        if(strcmpi(parts{i},'-site'))
            site = str2num(parts{i+1});
        elseif(strcmpi(parts{i},'-side'))
            side = str2num(parts{i+1});
        elseif(strcmpi(parts{i},'-mass'))
            mass = str2num(parts{i+1});
        elseif(strcmpi(parts(i),'-bodyFat'))
            bodyFat = str2num(parts{i+1});
        elseif(strcmpi(parts{i},'-epoch'))
            epoch = str2num(parts{i+1});
        elseif(strcmpi(parts{i},'-out'))
            csv = parts{i+1}
        end
    end
    %call removing dodgy quotes etc from strings
    findPA(parts{1}, 'side',side,'mass',mass,'bodyFat',bodyFat,'epoch',epoch,'csv',csv(2:end-1));
end