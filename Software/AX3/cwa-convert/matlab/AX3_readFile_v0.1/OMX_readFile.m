function data = OMX_readFile(filename, varargin)
    %
    %   IMPORTANT:  This version only reads accelerometer data and makes assumptions about the format and range, and doesn't check the sector checksum.
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
    %           'validPackets'      A Nx3 matrix containing pre-read valid
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
    %                               extract. Order is ACC, (reserved), (reserved). 
    %                               e.g. [1, 0, 0]
    %
    %           'verbose'           Print out debug messages about
    %                               progress.
    %
    %           'useC'              Heavily speed up parsing of samples and
    %                               timestamps by relying on external
    %                               c-code (parseDate.c). Requires compilation
    %                               using mex-interface or pre-compiled
    %                               binaries (.mexXXX files).
    %
    %       EXAMPLES:
    %           
    %       Reading file information:
    %           >> fileinfo = OMX_readFile('foobar.omx', 'info', 1, 'useC', 1)
    %               fileinfo = 
    %                 validPackets: [991997x3 double]
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
    %               'validPackets', fileinfo.validPackets, ...
    %               'startTime', datenum('19-Feb-2012 00:00:00'), ... 
    %               'stopTime', datenum('20-Feb-2012 00:00:00'));
    %
    %           >> data = 
    %               validPackets: [73059x3 double]
    %                        ACC: [8766736x4 double]
    %                  reserved1: ...
    %                  reserved2: ...       
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
    addOptional(p,'validPackets',0,@isnumeric);     % pre-read packet info
    addOptional(p,'startTime',-1,@isnumeric);       % start time (matlab)
    addOptional(p,'stopTime',-1,@isnumeric);        % stop time  (matlab)
    addOptional(p,'verbose',0,@isnumeric);          % print out progress
    addOptional(p,'useC',0,@isnumeric);             % use external c-code 
                                                    % for speed.
    addOptional(p,'modality',[1 0 0], @isnumeric);  % specify modality
                                                    % [ACC reserved1 reserved2]
    
    % parse varargin
    parse(p,varargin{:});
    
    % dispatch
    if p.Results.info > 0,
        data = readFileInfo(filename, p.Results);
    else
        data = readFile(filename, p.Results);
    end
    
    
end

function data = readFile(filename, options)
    % read data from file according to specified options
    
    % open file
    fid = fopen(filename);
    
    if options.verbose,
        fprintf('reading file: %s\n', filename);
    end


    % check if we have pre-read timestamps...
    if length(options.validPackets) == 1,
        % we don't, so read information
        data.validPackets = getValidPackets(fid, options);
    else
        % get the valid packet position from the given matrix
        data.validPackets = options.validPackets;
    end
    
    
    % get relevant parts of data if sliced reading enabled
    %
    
    % pre-set boundaries
    first = 1;
    last  = size(data.validPackets,1);
    
    % if startTime & stopTime given, get slice of data
    if options.startTime > 0 && options.stopTime > 0,
        if options.verbose > 0,
            fprintf('Finding relevant packets from %s to %s.\n',datestr(options.startTime),datestr(options.stopTime));
        end

        % find relevant parts of valid packets (second column = matlab-timestamp)
        tmplist = find(data.validPackets(:,2) >= options.startTime & data.validPackets(:,2) <= options.stopTime);

        if isempty(tmplist),
            fprintf('No packets found in desired time-range! Aborting.\n');
        end

        first = tmplist(1);
        last = tmplist(end);
        
        clear tmplist;
    end
    
    % filter positions for period of interest
    data.validPackets = data.validPackets(first:last,:);
    
    % check if any packets are left...
    if isempty(data.validPackets),
        fprintf('no data after filtering for start and stop dates given.\n');
        return
    end
    
    % get the timestamps for each sample for interpolation later
    % First: get packet ids
    packetIds = (1:length(data.validPackets))';
    % ... with positive timestamp offset (packet where clock flicked over to current time)
    %packetIds = find(data.validPackets(:,3) > 0);
    
    % now, use timestamp offset to get (sample) position on which timestamp occurred (80 samples per packet). 
    % TIME will be used for interpolation of other timestamps (offset, timestamp)
    TIME = [(packetIds*80)+double(data.validPackets(packetIds,3)) data.validPackets(packetIds,2)];
    % TIME_full just contains integer packet numbers for LIGHT and TEMP.
    %TIME_full = [packetIds data.validPackets(packetIds,2)];
    
    % read data
    validIds = data.validPackets(:,1);
    
%    numSamples = size(validIds,1)*80;
    numSamples = data.validPackets(end,5) + data.validPackets(end,4);
    
    % see what modalities to extract...
    if options.modality(1),
        % initialize variables
        data.ACC = zeros(numSamples,4);
        ACC_tmp = zeros(numSamples,3,'int16');
    end
    
    if options.modality(2),
        % initialize variables
        %data.LIGHT   = zeros(size(validIds,1),2);
    end
    
    if options.modality(3),
        % initialize variables
        %data.TEMP    = zeros(size(validIds,1),2);
    end
    
    if options.verbose,
        fprintf('reading data samples\n');
    end

    %cnt = 1;
    
    % for each packet in valid packets, read samples
    for i=1:length(validIds),
        try % just to make sure
            
            if options.modality(1) == 1,
                % read accelerometer values
                fseek(fid,(validIds(i)-1)*512+24,-1);
                
                dataOffset = data.validPackets(i,5) + 1;  % dataOffset = (i-1)*80+1;
                dataCount = data.validPackets(i,4);       % dataCount = 80;
                
                % reads 80*3 signed shorts (16 bit). 
                ACC_tmp(dataOffset:(dataOffset+dataCount-1),1:3) = fread(fid, [3,dataCount], 'int16=>int16',0,'ieee-le')';
                
            end
            
            %cnt = cnt + 1;
        catch %err
            disp 'Warning: problem during data read ';
            %rethrow err;
        end
    end

    if options.modality(1),
        % decode values for accelerometer

        if options.verbose > 0,
            fprintf('parsing samples\n');
        end
        
        data.ACC(:,2:4) = double(ACC_tmp) / 4096;
		
        clear ACC_tmp; % clear some memory
    end
    
    if options.verbose,
        fprintf('Calculating timestamps.\n');
    end
    
    if options.modality(1),
        % Interpolate the timestamps
        %data.ACC = data.ACC(1:length(validIds)*80,:);
        
        % interpolate using TIME, linearly between known sample-based timestamp locations.
        
        % clean TIME so that they are in strict monotonic order
        TIME = TIME([diff(TIME(:,1))~=0 ; true],:);
		
        data.ACC(:,1) = interp1(TIME(:,1),TIME(:,2),1:length(data.ACC),'linear','extrap');

        % clean up cases outside of known timestamps
        %data.ACC = data.ACC(data.ACC(:,1)>0,:);
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

function validPackets = getValidPackets(fid, options)
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
    seekType = hex2dec('6164'); % little endian: 0x64 'd', 0x61 = 'a'.
    validIds = find(packetHeaders==seekType);
	
    % return ids and timestamps
    if options.verbose
        fprintf('parsing timestamps\n');
    end
 	validPackets = zeros(length(validIds),5);
	validPackets(:,1) = validIds;
    
    tsf = double(packetTimestampFractional(validIds)) * (1 / (65536 * 24 * 60 * 60));
        
    for i=1:length(validIds),
		ts = packetTimestamps(validIds(i));
        if options.useC
            date = datenum(parseDate(ts));
        else
            date = parseDateML(ts);
        end
        validPackets(i,2) = date + tsf(i);
    end
    
    validPackets(:,3) = packetOffsets(validIds);
    validPackets(:,4) = packetSampleCount(validIds);
    
    packetSampleOffset = [0; cumsum(packetSampleCount(validIds))];
    validPackets(:,5) = packetSampleOffset(1:end-1);
    
end

function data = readFileInfo(filename, options)
    % read file general info

    % open file
    fid = fopen(filename);
    
    % read valid packet positions 
    data.validPackets = getValidPackets(fid, options);
    
    % get time (first and last valid packets)
    data.start.mtime = data.validPackets(1,2);
    data.start.str   = datestr(data.validPackets(1,2));
    data.stop.mtime  = data.validPackets(end,2);
    data.stop.str    = datestr(data.validPackets(end,2));
    
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
