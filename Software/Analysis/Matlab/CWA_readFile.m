function data = CWA_readFile(filename, varargin)
    %
    %   NOTES:
    %   * .AXES returns [time Ax Ay Az] or [time Ax Ay Az Gx Gy Gz].
    %   * The accelerometer values are scaled to units of 'g' (9.81 m/s/s).
    %   * The gyro values are scaled to units of degrees/sec.
    %   * Light and temperature are not (yet) supported.
    %   * This Matlab importer does not (yet) check the per-sector checksum.
	%
    %   DATA = CWA_readFile(FILENAME, [OPTIONS])
    %
    %   Reads in an CWA binary file. 
	%   Returns a struct filled with data.
    %
    %       Input arguments:
    %           FILENAME            Path to CWA file
    %
    %       OPTIONS:
    %
    %           'info'              If set to 1, just reads information
    %                               from file. Such as start time of
    %                               logging, sensor id, etc. (see example
    %                               below)
    %
    %           'packetInfo'        A Nx3 matrix containing pre-read 
    %                               packet locations, timestamps and
    %                               timestamp offsets (produced by
    %                               info-mode). 
    %
    %           'startTime'         Use this combined with 'stopTime' for
    %                               sliced reading. i.e. read all samples
    %                               from startTime to stopTime. Has to be
    %                               given in Matlab-time format (see
    %                               example below)
    %
    %           'stopTime'          See above.
    %
    %           'modality'          A three element vector [1, 1, 1] that
    %                               indicates which sensor modalities to
    %                               extract. Order is:
    %                               [ AXES (time+accel+gyro), LIGHT, TEMP ]
    %                               e.g. [1, 0, 0]
    %                               (LIGHT/TEMP NOT CURRENTLY SUPPORTED)
    %
    %           'verbose'           Print out debug messages about
    %                               progress.
    %
    %           'useC'              Attempt to speed up parsing of samples and
    %                               timestamps by relying on external
    %                               c-code (parseDate.c). Requires compilation
    %                               using mex-interface or pre-compiled
    %                               binaries (.mexXXX files).
    %
    %           'step'              (NOT CURRENTLY SUPPORTED)
    %                               Skip samples (1 = no skipping),
    %                               inefficient for small numbers that are not 1.
    %
    %
    %       EXAMPLES:
    %           
    %       Reading file information:
    %           >> fileinfo = CWA_readFile('foobar.cwa', 'info', 1, 'useC', 1)
    %               fileinfo = 
    %                   packetInfo: [991997x3 double]
    %                        start: [1x1 struct]
    %                         stop: [1x1 struct]
    %                     deviceId: 33
    %                    sessionId: 0
    %           >> fileinfo.start
    %               ans = 
    %                   mtime: 7.3492e+05
    %                     str: '22-Oct-2019 12:56:25'
    %
    %       subsequent read of slice using pre-read packet info:
    %           >> data = CWA_readFile('foobar.cwa', ... 
    %               'packetInfo', fileinfo.packetInfo, ...
    %               'startTime', datenum('22-Oct-2019 00:00:00'), ... 
    %               'stopTime', datenum('23-Oct-2019 00:00:00'));
    %
    %           >> data = 
    %                 packetInfo: [73059x5 double]
    %                       AXES: [8766736x4 double]
    %
    %   data.AXES will be [time Ax Ay Az] for accelerometer-only recordings
    %                  or [time Ax Ay Az Gx Gy Gz] for accel + gyro.
    %
    %   v0.1
    %       Dan Jackson, 2019
    %       derived from AX3 loader by Nils Hammerla, 2012 <nils.hammerla@ncl.ac.uk>
    %
    
    % 
    % Copyright (c) 2012-2020, Newcastle University, UK.
    % All rights reserved.
    % 
    % Redistribution and use in source and binary forms, with or without 
    % modification, are permitted provided that the following conditions are met: 
    % 1. Redistributions of source code must retain the above copyright notice, 
    %    this list of conditions and the following disclaimer.
    % 2. Redistributions in binary form must reproduce the above copyright notice, 
    %    this list of conditions and the following disclaimer in the documentation 
    %    and/or other materials provided with the distribution.
    % 
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
    
    % parse arguments
    p = inputParser;
    
    % define optional arguments with default values
    addOptional(p,'info',0,@isnumeric);             % just get file-info?
    addOptional(p,'packetInfo',0,@isnumeric);       % pre-read packet info
    addOptional(p,'startTime',-1,@isnumeric);       % start time (matlab)
    addOptional(p,'stopTime',-1,@isnumeric);        % stop time  (matlab)
    addOptional(p,'verbose',0,@isnumeric);          % print out progress
    addOptional(p,'useC',0,@isnumeric);             % use external c-code for date conversion
    addOptional(p,'modality',[1 0 0], @isnumeric);  % specify modality [AXES LIGHT TEMP]
    addOptional(p,'step',1, @isnumeric);            % specify step size (default 1), this is inefficient for small numbers other than 1
    
    % parse varargin
    parse(p,varargin{:});
    
    % dispatch
    if p.Results.info > 0,
        data = readFileInfo(filename, p.Results);
    else
        data = readFile(filename, p.Results);
    end
    
    
end


function metadata = cwaMetadata(metadataBytes)
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
                ofs = ofs + 1;
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
        
        if ~isempty(str) && str(1) == double('?')   % Skip initial '?'
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

    % Trim end (0x00, 0xFF, or ' ')
    while ~isempty(metadataBytes) && (double(metadataBytes(end)) == 32 || double(metadataBytes(end)) == 0 || double(metadataBytes(end)) == 255)
        metadataBytes = metadataBytes(1:end-1);
    end
    
    params = decodeParams(metadataBytes);
    
    keySet = {}; % zeros(1, length(params));
    valueSet = {}; % zeros(1, length(params));
    for i = 1:length(params)
        keySet{i} = params{i}.key;
        valueSet{i} = params{i}.value;
    end
    
    metadata = containers.Map(keySet,valueSet,'UniformValues',false); % num2cell(valueSet)
end


function data = readData(fid, packetInfo, options)
    
    if options.verbose,
        fprintf('preparing to read data\n');
    end
       
    % Get all 'AX' packet ids
    seekType = hex2dec('5841');
    %packetIds = (1:length(packetInfo))';
    packetHeaders = packetInfo(:,5);
    packetIds = find(packetHeaders==seekType);
    if isempty(packetIds)
        fprintf('no data packets found.\n');
        data = [];
        return;
    end
    packetInfo = packetInfo(packetIds,:);
    
    % get the timestamps for each sample for interpolation later
    % ... with positive timestamp offset (packet where clock flicked over to current time)
    %packetIds = find(packetInfo(:,3) > 0);
    packetSampleCount = packetInfo(:,4);
    packetSampleOffset = [0; cumsum(packetSampleCount)];
    packetSampleOffset = packetSampleOffset(1:end-1);
    
    % now, use timestamp offset to get (sample) position on which timestamp occurred (40/80/120 samples per packet). 
    % TIME will be used for interpolation of other timestamps (offset, timestamp)
    TIME = [packetSampleOffset+double(packetInfo(:,3)) packetInfo(:,2)];
    % TIME_full just contains integer packet numbers for LIGHT and TEMP.
    %TIME_full = [packetIds packetInfo(packetIds,2)];
    
    % Number of samples
    numActualSamples = packetSampleOffset(end) + packetSampleCount(end);
    numSamples = floor(numActualSamples / options.step);
    
    % Scale
    packetNumAxesBps = packetInfo(:,6);
    packetLightScale = bitand(packetInfo(:,7), 64512);  % 0b1111110000000000
    numAxisBps = packetNumAxesBps(1);
    scaleCode = packetLightScale(1);
    
    % 0x32 (top nibble: number of axes, 3=Axyz, 6=Gxyz/Axyz, 9=Gxyz/Axyz/Mxyz; bottom nibble: packing format - 2 = 3x 16-bit signed, 0 = 3x 10-bit signed + 2-bit exponent)
    axes = bitshift(numAxisBps, -4);
    packing = bitand(numAxisBps, 15);
    if packing ~= 2 && (packing ~= 0 || (packing == 0 && axes ~= 3)),
        fprintf('Error: Unknown packing format %f.\n', numAxisBps);
        data = [];
        return;
    end
    if axes ~= 3 && axes ~= 6,
        fprintf('Warning: Not fully supported number of axes %f.\n', axes);
    end
    
    % Which IDs to read
    validIds = packetInfo(:,1);
    validIds = validIds(find(packetNumAxesBps==numAxisBps));
    validIds = validIds(find(packetLightScale==scaleCode));
    if length(validIds) < length(packetInfo(:,1)),
        fprintf('Warning: Only reading %f of %f sector(s) as the other %f was/were of another format.\n', length(validIds), length(packetInfo(:,1)), length(packetInfo(:,1)) - length(validIds));
    end
    
    % AAAGGGxxxxxxxxxx top three bits are unpacked accel scale (1/2^(8+n) g); next three bits are gyro scale (8000/2^n dps)
    accScale = 1 / pow2(8 + bitshift(scaleCode, -13));
    gyrScale = (8000 / pow2(bitand(bitshift(scaleCode, -10), 7))) / 32768;

    % see what modalities to extract...
    data = zeros(numSamples, axes + 1);
    if packing == 0,  % Packed format
        fprintf('Warning: Packed format not yet tested (use AX3_readFile).\n');
        dataRaw = zeros(numSamples, 1, 'uint32');
    else
        dataRaw = zeros(numSamples, axes, 'int16');
    end
    
    if options.verbose,
        fprintf('reading data\n');
    end
    
    if options.step == 1,
        % for each packet in valid packets, read samples
        for i=1:length(validIds),
            try
                % read values
                fseek(fid,(validIds(i)-1)*512+30,-1);

                dataOffset = packetSampleOffset(i) + 1;  % dataOffset = (i-1)*dataCount+1;
                dataCount = packetSampleCount(i);        % dataCount = 40|80|120;

                if packing == 0, % Packed format
                    dataRaw(dataOffset:(dataOffset+dataCount-1),1) = fread(fid, dataCount, 'uint32',0,'ieee-le');
                else
                    % reads shorts (16 bit). 
                    dataRaw(dataOffset:(dataOffset+dataCount-1),1:axes) = fread(fid, [axes,dataCount], 'int16=>int16',0,'ieee-le')';
                end
                
            catch err
                fprintf('Warning: problem during data read: %s - %s\n', err.identifier, err.message);
                %rethrow err;
            end
        end
        
    else
        disp 'Error: Stepped reading not currently supported.';
        data = [];
        return;
    end

    if options.verbose > 0,
        fprintf('scaling\n');
    end

    if axes == 3 && packing == 0, % Packed format
        % calls external c-code to unpack values (for speed pass on full block instead of loop)
        if options.useC
        	data(:,2:4) = parseValueBlock(dataRaw)' .* accScale;
        else
            % Stored accel packed into DWORD
            data(:,2:4) = double(parseValueBlockML(dataRaw)) .* accScale;
        end
    else
        if axes == 6,
            % Stored Gx,Gy,Gz,Ax,Ay,Az
            data(:,5:7) = double(dataRaw(:,1:3)) * gyrScale;
            data(:,2:4) = double(dataRaw(:,4:6)) * accScale;
        elseif axes == 3,
            % Stored Ax,Ay,Az
            data(:,2:4) = double(dataRaw(:,1:3)) * accScale;
        end
    end
    
    clear dataRaw; % clear some memory
    
    if options.verbose,
        fprintf('Calculating timestamps.\n');
    end
    
    % interpolate using TIME, linearly between known sample-based timestamp locations.
    % clean TIME so that they are in strict monotonic order
    TIME = TIME([diff(TIME(:,1))~=0 ; true],:);
    data(:,1) = interp1(TIME(:,1),TIME(:,2),1:size(data,1),'linear','extrap');

    % clean up cases outside of known timestamps
    %data = data(data(:,1)>0,:);
end


function data = readFile(filename, options)
    % read data from file according to specified options
    
    % open file
    fid = fopen(filename);
    
    if options.verbose,
        fprintf('reading file: %s\n', filename);
    end


    % check if we have pre-read packet information...
    if length(options.packetInfo) == 1,
        % we don't, so read information
        data.packetInfo = getPacketInfo(fid, options);
    else
        % get the valid packet position from the given matrix
        data.packetInfo = options.packetInfo;
    end
    
    
    % get relevant parts of data if sliced reading enabled
    %
    
    % pre-set boundaries
    first = 1;
    last  = size(data.packetInfo,1);
    
    % if startTime & stopTime given, get slice of data
    if options.startTime > 0 && options.stopTime > 0,
        if options.verbose > 0,
            fprintf('Finding relevant packets from %s to %s.\n',datestr(options.startTime),datestr(options.stopTime));
        end

        % find relevant parts of valid packets (second column = matlab-timestamp)
        timeList = find(data.packetInfo(:,2) >= options.startTime & data.packetInfo(:,2) <= options.stopTime);

        if isempty(timeList),
            fprintf('No packets found in desired time-range! Aborting.\n');
        end

        first = timeList(1);
        last = timeList(end);
        
        clear timeList;
    end
    
    % filter positions for period of interest
    data.packetInfo = data.packetInfo(first:last,:);
    
    % check if any packets are left...
    if isempty(data.packetInfo),
        fprintf('no data after filtering for start and stop dates given.\n');
        fclose(fid);
        return
    end
    
    % see what modalities to extract...
    if options.modality(1),
        data.AXES = readData(fid, data.packetInfo, options);
    end
    if options.modality(2),
        disp 'Warning: Light reading not currently supported.';
        data.LIGHT = [];
    end
    if options.modality(3),
        disp 'Warning: Temperature reading not currently supported.';
        data.TEMP = [];
    end
   
    % close the file, done.
    fclose(fid);
end

function packetInfo = getPacketInfo(fid, options)
    % read all the valid ids (packets in file)
    
    % get length of file in bytes
    fseek(fid,0,1);                         % goto eof
    lengthBytes = ftell(fid);               % get position
    fseek(fid,0,-1);                        % rewind
    numPackets = floor(lengthBytes / 512);  % number of packets
    
    % Now reading headers and timestamps, etc.
    
    % get headers
    if options.verbose
        fprintf('reading headers\n');
    end
	fseek(fid,0,-1);
    packetHeaders = fread(fid, numPackets, 'uint16=>uint16',510,'ieee-le');
    
    % get timestamps
    if options.verbose
        fprintf('reading fractional\n');
    end
	fseek(fid,4,-1);
	packetTimestampFractional = fread(fid, numPackets, 'uint16=>uint16',510,'ieee-le');	
    packetTimestampFractional = bitand(packetTimestampFractional, 32767);
    if options.verbose
        fprintf('reading timestamps\n');
    end
	fseek(fid,14,-1);
	packetTimestamps = fread(fid, numPackets, 'uint32=>uint32',508,'ieee-le');
    if options.verbose
        fprintf('reading light/scale\n');
    end
	fseek(fid,18,-1);
	packetLightScale = fread(fid,numPackets,'uint16=>uint16',510,'ieee-le');
    if options.verbose
        fprintf('reading temperature\n');
    end
	fseek(fid,20,-1);
	packetTemperature = fread(fid,numPackets,'uint16=>uint16',510,'ieee-le');
    if options.verbose
        fprintf('reading packing\n');
    end
	fseek(fid,25,-1);
	packetNumAxesBps = fread(fid, numPackets, 'uint8=>uint8',511);	
    if options.verbose
        fprintf('reading offsets\n');
    end
    fseek(fid,26,-1);
    packetOffsets = fread(fid,numPackets,'int16=>int16',510,'ieee-le');
    if options.verbose
        fprintf('reading sample counts\n');
    end
    fseek(fid,28,-1);
    packetSampleCount = fread(fid,numPackets,'uint16=>uint16',510,'ieee-le');
	    
    % find valid packets
    if options.verbose
        fprintf('finding valid packets\n');
    end
    validIds = 1:size(packetHeaders,1);
    %seekType = hex2dec('5841'); % little endian: 0x41 'A', 0x58 = 'X'.
    %validIds = find(packetHeaders==seekType);
	
    % return ids and timestamps
    if options.verbose
        fprintf('parsing timestamps\n');
    end
 	packetInfo = zeros(length(validIds),8);
	packetInfo(:,1) = validIds;
    
    tsf = double(packetTimestampFractional(validIds)) * (1 / (65536 * 24 * 60 * 60));

    for i=1:length(validIds),
		ts = packetTimestamps(validIds(i));
        if options.useC
            date = datenum(parseDate(ts));
        else
            date = parseDateML(ts);
        end
        packetInfo(i,2) = date + tsf(i);
    end
    
    packetInfo(:,3) = packetOffsets(validIds);
    packetInfo(:,4) = packetSampleCount(validIds);
    packetInfo(:,5) = packetHeaders(validIds);
    packetInfo(:,6) = packetNumAxesBps(validIds);
    packetInfo(:,7) = packetLightScale(validIds);
    packetInfo(:,8) = packetTemperature(validIds);

end

function data = readFileInfo(filename, options)
    % read file general info

    % open file
    fid = fopen(filename);
    %cleanfid = onCleanup(@()fclose(fid));
    
    % @64  +448 Scratch buffer / meta-data (448 ASCII characters, ignore trailing 0x20/0x00/0xff bytes, url-encoded UTF-8 name-value pairs)
    [bytes, count] = fread(fid, 512);
    if count ~= 512
        disp('Warning: File header could not be read');
    elseif bytes(1) ~= double('M') || bytes(2) ~= double('D')
        disp('Warning: File header is not valid');
    else
        data.deviceId = (bytes(7) * 256) + bytes(6);
        if bytes(12) ~= 255 || bytes(13) ~= 255,
            data.deviceId = data.deviceId + (bytes(13) * 256 * 256 * 256) + (bytes(12) * 256 * 256);
        end
        data.sessionId = (bytes(11) * 256 * 256 * 256) + (bytes(10) * 256 * 256) + (bytes(9) * 256) + bytes(8);
        
        metadataBytes = bytes(65:512);
        data.metadata = cwaMetadata(metadataBytes);
    end
    
    % read valid packet positions 
    data.packetInfo = getPacketInfo(fid, options);
    
    % get time (first and last valid packets)
    data.start.mtime = data.packetInfo(1,2);
    data.start.str   = datestr(data.packetInfo(1,2));
    data.stop.mtime  = data.packetInfo(end,2);
    data.stop.str    = datestr(data.packetInfo(end,2));
    
    % close the file
    fclose(fid);
end

% Packed accelerometer values
% Matlab version by Dan Jackson, 2012
function res = parseValueBlockML(value)
    % 32-bit packed value (The bits in bytes [3][2][1][0]: eezzzzzz zzzzyyyy yyyyyyxx xxxxxxxx, e = binary exponent, lsb on right)
    e = int32(2 .^ bitshift(value, -30));
    x = int32(bitand(         value      , 1023)); x = e .* (x - (int32(1024) .* int32(x >= 512)));
    y = int32(bitand(bitshift(value, -10), 1023)); y = e .* (y - (int32(1024) .* int32(y >= 512)));
    z = int32(bitand(bitshift(value, -20), 1023)); z = e .* (z - (int32(1024) .* int32(z >= 512)));
    res = [x y z];
end

function dat = parseDateML(t)
    persistent lastvalue;
    persistent lastdatearr;
    persistent lastreturn;
    
    if isempty(lastdatearr) 
        lastdatearr = zeros(1,6);
    end

    if (not(isempty(lastvalue)) && t == lastvalue)
        % do nothing
    elseif (not(isempty(lastvalue)) && t == lastvalue + 1)
        lastdatearr(6) = lastdatearr(6) + 1;
        lastreturn = datenum(lastdatearr); % lastreturn = lastreturn + (1 / 60 / 60 / 24); % accumulates a small error
    else
        %%% 'idivide' version -- think this fixes potential problems with initial version
        lastdatearr(1) = idivide(t, 67108864, 'floor') + 2000;  % #define OM_DATETIME_YEAR(dateTime)    ((unsigned int)((unsigned char)(((dateTime) >> 26) & 0x3f)) + 2000) /**< Extract the year from a packed date/time value. @hideinitializer */
        lastdatearr(2) = mod(idivide(t, 4194304, 'floor'), 16); % #define OM_DATETIME_MONTH(dateTime)   ((unsigned char)(((dateTime) >> 22) & 0x0f))  /**< Extract the month (1-12) from a packed date/time value. @hideinitializer */
        lastdatearr(3) = mod(idivide(t, 131072, 'floor'), 32);  % #define OM_DATETIME_DAY(dateTime)     ((unsigned char)(((dateTime) >> 17) & 0x1f))  /**< Extract the day (1-31) from a packed date/time value. @hideinitializer */
        lastdatearr(4) = mod(idivide(t, 4096, 'floor'), 32);    % #define OM_DATETIME_HOURS(dateTime)   ((unsigned char)(((dateTime) >> 12) & 0x1f))  /**< Extract the hours (0-23) from a packed date/time value. @hideinitializer */
        lastdatearr(5) = mod(idivide(t, 64, 'floor'), 64);      % #define OM_DATETIME_MINUTES(dateTime) ((unsigned char)(((dateTime) >>  6) & 0x3f))  /**< Extract the minutes (0-59) from a packed date/time value. @hideinitializer */
        lastdatearr(6) = mod(t, 64);                            % #define OM_DATETIME_SECONDS(dateTime) ((unsigned char)(((dateTime)      ) & 0x3f))  /**< Extract the seconds (0-59) from a packed date/time value. @hideinitializer */
        lastreturn = datenum(lastdatearr);
    end

    lastvalue = t;
    
    dat = lastreturn;
end
