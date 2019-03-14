function metadata = cwaMetadata(file)
% Load CWA Metadata
% - Dan Jackson, 2019.
%
% To load the metadata for a .CWA file:
%
%  metadata = cwaMetadata(cwaFilename);
%
% To list the metadata keys (fields) and their values:
%
%  for k = metadata.keys()
%    disp(struct('key', k, 'value', metadata(k{1})))
%  end
%
% The possible keys for the study are:
%
% _c  Study Centre
% _s  Study Code
% _i  Study Investigator
% _x  Exercise Type
% _so Operator
% _n  Notes
%
% The possible keys for the subject are:
%
% _sc Subject Code
% _se Sex (- / male / female)
% _h  Height
% _w  Weight
% _ha Handedness (- / left / right)
% _p  Site (- / left wrist / right wrist / waist / left ankle / right ankle / left thigh / right thigh / left hip / right hip / left upper-arm / right upper-arm / chest / sacrum / neck / head
% _co Comments
%
% You can check if a given key is present, e.g. for 'Height':
%
%  metadata.isKey('_h')
%
% And you can look up a given value, e.g. for 'Height':
%
%  metadata('_h')
%
% As the metadata is always a string, you may sometimes want the numerical value instead, e.g.:
%
%  str2num(metadata('_h'))
%
%
    function ret = urldecode(str)
        function hex = isHex(char)
            %hexChars = ['0':'9' 'A':'F' 'a':'f']
            char = double(char);
            hex = (char >= double('0') && char <= double('9')) || (char >= double('A') && char <= double('F')) || (char >= double('a') && char <= double('f'));
        end
        ret = [];
        ofs = 1;
        while ofs <= length(str)
            if double(str(ofs)) == double('%') && length(str) > ofs+2 && isHex(str(ofs+1)) && isHex(str(ofs+2))
                %sprintf('%s%c', ret, char(hex2dec(str((ofs+1):(ofs+2)))));
                ret = [ret hex2dec(str((ofs+1):(ofs+2)))]; %#ok<AGROW>
                ofs = ofs + 3;
            elseif double(str(ofs)) == double('+')
                ret = [ret 32]; %#ok<AGROW>
                ofs = ofs + 1
            else % not matched as hex
                %ret = sprintf('%s%c', ret, str(ofs));
                ret = [ret str(ofs)]; %#ok<AGROW>
                ofs = ofs + 1;
            end
        end
        ret = native2unicode(ret, 'UTF-8');
    end

    function params = decodeParams(str)
        params = {};
        
        str = str.';
        
        %disp(urldecode(str));
        
        if str(1) == double('?')   % Skip initial '?'
            str = str(2:end);
        end
        
        breakIndexes = find(str(:)==double('&')).';
        breakIndexes = [1-1 breakIndexes length(str)+1];
        for idx = 2:length(breakIndexes)
            pair = str(breakIndexes(idx-1)+1:breakIndexes(idx)-1);
            
            equals = find(pair(:) == double('='));
            if isempty(equals)
                equals = length(pair) + 1;
            else
                equals = equals(1);
            end
            
            key = pair(1:(equals-1));
            if equals + 1 <= length(pair)
                value = pair(equals + 1:end);
            else
                value = [];
            end
            
            key = urldecode(key);
            value = urldecode(value);
            
            params{end+1} = struct('key', key, 'value', value); %#ok<AGROW>
        end
        
    end


    disp(['METADATA: CWA... ', file]);
    fid = fopen(file);
    cleanfid = onCleanup(@()fclose(fid));
    
    % @64  +448 Scratch buffer / meta-data (448 ASCII characters, ignore trailing 0x20/0x00/0xff bytes, url-encoded UTF-8 name-value pairs)
    [bytes, count] = fread(fid, 512);
    if count ~= 512
        throw(MException('Metadata load error'));
    end
    if bytes(1) ~= double('M') || bytes(2) ~= double('D')
        throw(MException('Metadata not valid'));
    end
    bytes = bytes(65:512);    
    % Trim end (0x00, 0xFF, or ' ')
    while double(bytes(end)) == 32 || double(bytes(end)) == 0 || double(bytes(end)) == 255
        bytes = bytes(1:end-1);
    end
    
    params = decodeParams(bytes);
    
    keySet = {}; % zeros(1, length(params));
    valueSet = {}; % zeros(1, length(params));
    for i = 1:length(params)
        keySet{i} = params{i}.key;
        valueSet{i} = params{i}.value;
    end
    
    metadata = containers.Map(keySet,valueSet,'UniformValues',false); % num2cell(valueSet)
    
    %fprintf('READ: %s.\n', datestr(start), datestr(finish));
end
