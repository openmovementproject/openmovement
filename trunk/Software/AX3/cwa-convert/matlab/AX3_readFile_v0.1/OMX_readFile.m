function data = OMX_readFile(filename, varargin)
    %
    %   NOTES:
    %   * This version defaults to reading just the accelerometer values.
    %   * The gyroscope/magnetometer data can be read when given the options:  'modality', [1, 1, 1]
    %   * Stepped data can be read given the options:  'step', 100
    %   * The accelerometer values are currently always scaled 1/4096 (the default range conversion to 'g')
    %   * The gyroscope/magnetometer values are currently not scaled at all
    %   * Metadata, light, pressure, temperature and battery levels are not yet supported
    %   * For speed, the Matlab importer doesn't check the sector checksum.
    %
	%
    %   DATA = OMX_readFile(FILENAME, [OPTIONS])
    %
    %   Reads in an OMX binary file. 
	%   Returns a struct filled with data.
    %
    %       Input arguments:
    %           FILENAME            Path to OMX file
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
    %                               extract. Order is ACC, GYR, MAG.
    %                               e.g. [1, 0, 0]
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
    %           'step'              Skip samples (1 = no skipping),
    %                               inefficient for small numbers that are not 1.
    %
    %
    %       EXAMPLES:
    %           
    %       Reading file information:
    %           >> fileinfo = OMX_readFile('foobar.omx', 'info', 1, 'useC', 1)
    %               fileinfo = 
    %                   packetInfo: [991997x3 double]
    %                        start: [1x1 struct]
    %                         stop: [1x1 struct]
    %                     deviceId: 33
    %                    sessionId: 0
    %           >> fileinfo.start
    %               ans = 
    %                   mtime: 7.3492e+05
    %                     str: '17-Feb-2012 12:56:25'
    %
    %       subsequent read of slice using pre-read packet info:
    %           >> data = OMX_readFile('foobar.omx', ... 
    %               'packetInfo', fileinfo.packetInfo, ...
    %               'startTime', datenum('19-Feb-2012 00:00:00'), ... 
    %               'stopTime', datenum('20-Feb-2012 00:00:00'));
    %
    %           >> data = 
    %                 packetInfo: [73059x5 double]
    %                        ACC: [8766736x4 double]
    %                        GYR: [8766736x4 double]
    %                        MAG: [8766736x4 double]
    %
    %
    %   v0.1
    %       Dan Jackson, 2014
    %       derived from CWA importer by Nils Hammerla, 2012 <nils.hammerla@ncl.ac.uk>
    %
    
    % 
    % Copyright (c) 2012-2014, Newcastle University, UK.
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
    addOptional(p,'modality',[1 0 0], @isnumeric);  % specify modality [ACC GYR MAG]
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


function data = readTriaxial(fid, packetInfo, seekType, step, scale)

    packetHeaders = packetInfo(:,5);
    
    % get the timestamps for each sample for interpolation later
    % Get all packet ids
    %packetIds = (1:length(packetInfo))';
    % Get requested packet ids
    packetIds = find(packetHeaders==seekType);
    packetInfo = packetInfo(packetIds,:);
    
    % ... with positive timestamp offset (packet where clock flicked over to current time)
    %packetIds = find(packetInfo(:,3) > 0);
    
    packetSampleCount = packetInfo(:,4);
    packetSampleOffset = [0; cumsum(packetSampleCount)];
    packetSampleOffset = packetSampleOffset(1:end-1);
    
    % now, use timestamp offset to get (sample) position on which timestamp occurred (80 samples per packet). 
    % TIME will be used for interpolation of other timestamps (offset, timestamp)
    TIME = [packetSampleOffset+double(packetInfo(:,3)) packetInfo(:,2)];
    % TIME_full just contains integer packet numbers for LIGHT and TEMP.
    %TIME_full = [packetIds packetInfo(packetIds,2)];
    
    % read data
    validIds = packetInfo(:,1);
    
%    numActualSamples = size(validIds,1)*80;
    numActualSamples = packetSampleOffset(end) + packetSampleCount(end);
    
    % Skip by 'step'
    numSamples = floor(numActualSamples / step);
    
    % see what modalities to extract...
    data = zeros(numSamples,4);
    tmp = zeros(numSamples,3,'int16');
    
    %if options.verbose,
    %    fprintf('reading data samples\n');
    %end

    %cnt = 1;
    
    if step == 1,
    
        % for each packet in valid packets, read samples
        for i=1:length(validIds),
            try % just to make sure

                % read values
                fseek(fid,(validIds(i)-1)*512+24,-1);

                dataOffset = packetSampleOffset(i) + 1;  % dataOffset = (i-1)*80+1;
                dataCount = packetSampleCount(i);       % dataCount = 80;

                % reads 80*3 signed shorts (16 bit). 
                tmp(dataOffset:(dataOffset+dataCount-1),1:3) = fread(fid, [3,dataCount], 'int16=>int16',0,'ieee-le')';

                %cnt = cnt + 1;
            catch %err
                disp 'Warning: problem during data read ';
                %rethrow err;
            end
        end
        
    else
        
        % Sub-sampled indexes
        sampleIndexes = 1:numSamples;
        
        % Calculate physical sample number
        physicalSampleIndex = floor((sampleIndexes - 1) * numActualSamples / numSamples) + 1;
        
        % Calculate the offset within the file of the sample
        offsets = zeros(numSamples,1);
        % Enumerate over the packets
        sample = 1;
        for packet=1:size(packetInfo,1),
            
            % Calculate base & limit sample number for this packet
            packetSampleBase = packetSampleOffset(packet) + 1;
            packetSampleLimit = packetSampleBase - 1 + packetSampleCount(packet);
            
            % For the samples within this packet...
            while sample <= numSamples && physicalSampleIndex(sample) <= packetSampleLimit

                offsets(sample) = ((validIds(packet) - 1) * 512) + 24 + ((physicalSampleIndex(sample) - packetSampleBase) * 6);
                sample = sample + 1;
                
            end            
        end
        
        % for each sample
        for i=1:numSamples,
            try % just to make sure
                % read axes
                fseek(fid,offsets(i),-1);
                tmp(i,1:3) = fread(fid, [3,1], 'int16=>int16',0,'ieee-le')';
            catch %err
                disp 'Warning: problem during data read ';
                %rethrow err;
            end
        end
        
    end

    % decode values for accelerometer

    %if options.verbose > 0,
    %    fprintf('parsing samples\n');
    %end

    data(:,2:4) = double(tmp) * scale;

    clear tmp; % clear some memory
    
    %if options.verbose,
    %    fprintf('Calculating timestamps.\n');
    %end
    
    % Interpolate the timestamps
    %data = data(1:length(validIds)*80,:);

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
        tmplist = find(data.packetInfo(:,2) >= options.startTime & data.packetInfo(:,2) <= options.stopTime);

        if isempty(tmplist),
            fprintf('No packets found in desired time-range! Aborting.\n');
        end

        first = tmplist(1);
        last = tmplist(end);
        
        clear tmplist;
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
        % little endian: 0x64 'd', 0x61 = 'a'.
        data.ACC = readTriaxial(fid, data.packetInfo, hex2dec('6164'), options.step, 1/4096);
    end
    if options.modality(2),
        % little endian: 0x64 'd', 0x67 = 'g'.
        data.GYR = readTriaxial(fid, data.packetInfo, hex2dec('6764'), options.step, 1.0);   % 0.07
    end
    if options.modality(3),
        % little endian: 0x64 'd', 0x6d = 'm'.
        data.MAG = readTriaxial(fid, data.packetInfo, hex2dec('6D64'), options.step, 1.0);   % 0.1
    end
   
    %if options.modality(2) == 1,
    %    % interpolate time for light
    %    data.LIGHT(:,1) = interp1(TIME_full(:,1),TIME_full(:,2),1:length(data.LIGHT),'linear',0);
    %    data.LIGHT = data.LIGHT(data.LIGHT(:,1)>0,:);
    %end
    
    %if options.modality(3) == 1,
    %    % interpolate time for temperature
    %    data.TEMP(:,1) = interp1(TIME_full(:,1),TIME_full(:,2),1:length(data.TEMP),'linear',0);
    %    data.TEMP = data.TEMP(data.TEMP(:,1)>0,:);
    %end
        
    % close the file, done.
    fclose(fid);
end

function packetInfo = getPacketInfo(fid, options)
    % read all the valid ids (accelerometer packets in file)
    
    % get length of file in bytes
    fseek(fid,0,1);                         % goto eof
    lengthBytes = ftell(fid);               % get position
    fseek(fid,0,-1);                        % rewind
    numPackets = floor(lengthBytes / 512);  % number of packets
    
    % Now reading headers and timestamps, etc. Refer to omapi.h for details
    % (OM_READER_DATA_PACKET)
    
    % get headers
    if options.verbose
        fprintf('reading headers\n');
    end
    packetHeaders = fread(fid, numPackets, 'uint16=>uint16',510,'ieee-le');
    
    % get timestamps
    if options.verbose
        fprintf('reading timestamps/fractional/offsets\n');
    end
	fseek(fid,8,-1);
	packetTimestamps = fread(fid, numPackets, 'uint32=>uint32',508,'ieee-le');
	fseek(fid,12,-1);
	packetTimestampFractional = fread(fid, numPackets, 'uint16=>uint16',510,'ieee-le');	
    fseek(fid,14,-1);
    packetOffsets = fread(fid,numPackets,'int16=>int16',510,'ieee-le');
    fseek(fid,22,-1);
    packetSampleCount = fread(fid,numPackets,'int16=>int16',510,'ieee-le');
	
    %uint8_t  packetType;            // @0  [1] Packet type (ASCII 'd' = single-timestamped data stream, 's' = string, others = reserved)
    %uint8_t  streamId;              // @1  [1] Stream identifier (by convention: a-ccel, g-yro, m-agnetometer, p-ressure, other-ASCII, non-alphanumeric=reserved, *=all streams)
    %uint16_t payloadLength;         // @2  [2] <0x01FC> Payload length (payload is 508 bytes long, + 4 header/length = 512 bytes total)
    %uint32_t sequenceId;            // @4  [4] (32-bit sequence counter, each packet in a stream has a sequential number, reset when a stream is restarted)
    %uint32_t timestamp;             // @8  [4] Timestamp stored little-endian (top-bit 0 = packed as 0YYYYYMM MMDDDDDh hhhhmmmm mmssssss with year-offset, default 2000; top-bit 1 = 31-bit serial time value of seconds since epoch, default 1/1/2000)
    %uint16_t fractionalTime;        // @12 [2] Fractional part of the time (1/65536 second)
    %int16_t  timestampOffset;       // @14 [2] The sample index, relative to the start of the buffer, when the timestamp is valid (0 if at the start of the packet, can be negative or positive)
    %uint16_t sampleRate;            // @16 [2] Sample rate (Hz)
    %int8_t   sampleRateModifier;    // @18 [1] Sample rate modifier (1 = none; >1 = divisor: sample rate / value; <-1 = multiplier: -value * sample rate; 0 = period in seconds; -1 = period in minutes)
    %uint8_t  dataType;              // @19 [1] Data type [NOT FINALIZED!] (top-bit set indicates "non-standard" conversion; bottom 7-bits: 0x00 = reserved,  0x10-0x13 = accelerometer (g, at +-2,4,8,16g sensitivity), 0x20 = gyroscope (dps), 0x30 = magnetometer (uT/raw?), 0x40 = light (CWA-raw), 0x50 = temperature (CWA-raw), 0x60 = battery (CWA-raw), 0x70 = pressure (raw?))
    %int8_t   dataConversion;        // @20 [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
    %uint8_t  channelPacking;        // @21 [1] Packing type (0x32 = 3-channel 2-bytes-per-sample (16-bit); 0x12 = single-channel 16-bit; 0x30 = DWORD packing of 3-axis 10-bit 0-3 binary shifted)
    %uint16_t sampleCount;           // @22 [2] Number samples in the packet (any remaining space is unused, or used for type-specific auxilliary values)
    %uint8_t  data[480];             // @24 [480] = e.g. 240 1-channel 16-bit samples, 80 3-channel 16-bit samples, 120 DWORD-packed 3-channel 10-bit samples
    %uint16_t aux[3];                // @504 [6] (Optional) auxiliary data specific to the data-type (e.g. for device configuration; or for battery/events/light/temperature in a CWA stream)
    %uint16_t checksum;              // @510 [2] 16-bit word-wise checksum of packet
    
    % find valid packets
    %seekType = hex2dec('6164'); % little endian: 0x64 'd', 0x61 = 'a'.
    %validIds = find(packetHeaders==seekType);
    validIds = 1:size(packetHeaders,1);
	
    % return ids and timestamps
    if options.verbose
        fprintf('parsing timestamps\n');
    end
 	packetInfo = zeros(length(validIds),5);
	packetInfo(:,1) = validIds;
    
    packetInfo(:,5) = packetHeaders;
    
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
    
end

function data = readFileInfo(filename, options)
    % read file general info

    % open file
    fid = fopen(filename);
    
    % read valid packet positions 
    data.packetInfo = getPacketInfo(fid, options);
    
    % get time (first and last valid packets)
    data.start.mtime = data.packetInfo(1,2);
    data.start.str   = datestr(data.packetInfo(1,2));
    data.stop.mtime  = data.packetInfo(end,2);
    data.stop.str    = datestr(data.packetInfo(end,2));
    
    % dummy deviceId
    data.deviceId = 0;
    
    % dummy sessionId
    data.sessionId = 0;
    
    % close the file
    fclose(fid);
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
