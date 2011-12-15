function data = CWA_read_binary_file(filename, fromDateStr, toDateStr, timestampType)
%
% function data = CWA_read_binary_file(filename, fromDateStr, toDateStr, relativeTime)
%
%   Reads a CWA binary file (.cwa), as downloaded from the sensors. Takes a
%   specific range as input argument to limit the amount of data to be
%   read. Uses binary search to find start and end points of relevant
%   segments so should show significant improvement in speed especially for
%   larger files. THIS SHOULD WORK FOR CONTINUOUSLY RECORDED DATA. It does
%   however not check for sanity of the files.
%
%   Accepts as arguments:
%
%       filename        complete path to the .cwa file (String)
%
%       fromDateStr     Start date+time of the portion that you want to get.
%                       To use first packet as start put empty string ('').
%
%       toDateStr       Same for the end. 
%                       To use last packet as end put empty string ('').
%             
%       timestampType   2 for matlab time (fraction of days since 1.1.00)
%                       1 for relative timestamps (start at 0.0),
%                       0 for unixtime timestamps (seconds since 1.1.1970)
%
%   Returns
%
%       data            matrix containing with one sample per row and
%                       columns for timestamp, X, Y, Z.
%
%
%       Example:
%
%           data = CWA_read_binary_range('/thefile.cwa', '14-Jan-2011 12:15:00', '14-Jan-2011 13:20:23', 1);
%
%   A lot of different formats are accepted. See 'help datenum' for
%   details. If the script fails probably the "assumptions" about the file
%   structure are wrong (corrupted header etc.)
%
%   TODO: Fix assumption of location for first accelerometer packet
%   (currently 256 * SECTOR_SIZE).
%
%   Nils Y. Hammerla, 10/2011
%   n.y.hammerla@ncl.ac.uk
%

    if strcmp(fromDateStr, '')
        MATstartTime = -1;
    else
        MATstartTime = datenum(fromDateStr);
    end
    
    if strcmp(toDateStr, ''),
        MATstopTime = -1;
    else
        MATstopTime = datenum(toDateStr);
    end
    
    fid = fopen(filename);

    SECTOR_SIZE = 512;

    fseek(fid, 0, 1); % = eof
    lengthBytes = ftell(fid);
    fseek(fid, 0, -1); %= back to start
    lengthSectors = lengthBytes / SECTOR_SIZE;

    % initialize positions
    startPacket = -1;
    stopPacket  = -1;
    
    % read first packet
    % META AND USAGE INFO IS IGNORED HERE
    % TODO: find proper start, don't just assume it is after fixed header
    sectorPos = 256;
    fseek(fid, sectorPos*SECTOR_SIZE, -1);
    firstPacket = readPacket(fid);
    
    % read last packet
    sectorPos = lengthSectors;
    fseek(fid, sectorPos*SECTOR_SIZE, -1);
    lastPacket = readPacket(fid);
    
    % check boundaries
    if MATstartTime < 0 | MATstartTime <= readTimestamp(firstPacket.timestamp), 
        startPacket = 256; 
    end;
    if MATstopTime < 0 | MATstopTime <= readTimestamp(lastPacket.timestamp), 
        stopPacket = lengthSectors; 
    end;
    
    % use binary search to find (undefined) boundaries
    if startPacket == -1,
        startPacket = binsearch(fid, MATstartTime, 256, lengthSectors);
    end
    
    if stopPacket == -1,
        stopPacket = binsearch(fid, MATstopTime, 256, lengthSectors);
    end
    
    fprintf('Start at packet: %d, with datestr %s\n', startPacket, datestr(getTime(fid,startPacket)));
    fprintf('Stop at packet: %d, with datestr %s\n', stopPacket, datestr(getTime(fid,stopPacket)));
    
    % allocate memory for data to speed up the whole thing
    data = zeros((stopPacket-startPacket+1) * 80,3);
    
    % read the relevant portion of the binary file
    % CHANGE HERE TO INCLUDE OTHER ASPECTS LIKE TEMPERATURE ETC
    cnt = 0;
    h = waitbar(0, 'reading relevant packets');
    for i=startPacket:stopPacket,
        fseek(fid,i*SECTOR_SIZE,-1);
        dataPacket = readPacket(fid);
        % dataPacket contains all info from packet (see cwa.h)
        
        % we just want samples though
        data(cnt*80+1:(cnt+1)*80,:) = dataPacket.sampleData';
        cnt = cnt+1;
        
        % update the waitbar
        waitbar(cnt / (stopPacket-startPacket),h);
    end
    close(h);
    
    % convert data to g
    data = data / 256;
    
    % put in the timestamp
    %
    % first: convert from weird matlab timestamp to unix time
    %       Matlab time is days from 1.1.2000
    %       Unix time is seconds from 1.1.1970
    fprintf('Calculating timestamps for data.\n');
    
    matlab_start = 946684800;   % seconds till 1.1.2000
    day_sec = 86400;
    
    startTime = floor(MATstartTime * day_sec + matlab_start);
    stopTime = ceil(MATstopTime * day_sec + matlab_start);
    
    if timestampType == 1,
        % relative time (Starts at 0)
        stopTime = stopTime - startTime;
        startTime = 0;
    elseif timestampType == 2,
        % matlab time
        startTime = MATstartTime;
        stopTime = MATstopTime;
    end
    
    % add interpolated timestamp to data
    % REMINDER: Assumes constant sampling rate across the whole session.
    data = [linspace(startTime,stopTime,size(data,1))'  data];
    
    fclose('all');
    
end

function pos = binsearch(fid, value, low, high)
    % good old binary search on dates (seeks though file)
    % should be quite fast...
    if high < low, 
        pos = -1;
    else
        mid = round(low + (high - low) / 2);
        if getTime(fid,mid) > value,
            pos = binsearch(fid, value, low, mid-1);
        elseif getTime(fid,mid) < value,
            pos = binsearch(fid, value, mid+1, high);
        else
            pos = mid;
        end
    end
end

function time = getTime(fid,pos),
    % short helper to read timestamp for specific sector (used in search)
    fseek(fid,pos*512+14,-1);
    time = readTimestamp(fread(fid, 1, 'uint32',0,'ieee-le'));
end

    
function dataPacket = readPacket(fid);
    % read in datapacket (on foot as no nice typecast of buffer
    % possible). Based on definitions in cwa.h
    
    % reads 512 bytes of packet
    dataPacket.packetHeader = fread(fid, [1,2], 'uint8=>char',0,'ieee-le');
    dataPacket.packetLength = fread(fid, 1, 'uint16',0,'ieee-le');
    dataPacket.deviceId     = fread(fid, 1, 'uint16',0,'ieee-le');
    dataPacket.sessionId    = fread(fid, 1, 'uint32',0,'ieee-le');
    dataPacket.sequenceId   = fread(fid, 1, 'uint32',0,'ieee-le');
    dataPacket.timestamp    = fread(fid, 1, 'uint32',0,'ieee-le');
    dataPacket.light        = fread(fid, 1, 'uint16',0,'ieee-le');
    dataPacket.temperature  = fread(fid, 1, 'int16',0,'ieee-le');
    dataPacket.events       = fread(fid, 1, 'uint8',0,'ieee-le');
    dataPacket.battery      = fread(fid, 1, 'uint8',0,'ieee-le');
    dataPacket.reserved     = fread(fid, 1, 'uint8',0,'ieee-le');
    dataPacket.numAxesBPS   = fread(fid, 1, 'uint8',0,'ieee-le');
    dataPacket.sampleRate   = fread(fid, 1, 'uint16',0,'ieee-le');
    dataPacket.sampleCount  = fread(fid, 1, 'uint16',0,'ieee-le');
    dataPacket.sampleData   = fread(fid, [3 80], 'int16',0,'ieee-le');
    dataPacket.checksum     = fread(fid, 1, 'uint16',0,'ieee-le');
    
    % dataPacket.sampleData holds the measurements. Still needs to be
    % transposed and normalised (divide by 256 to get G).
end

function dateSec = readTimestamp(t);
    % Read the CWA-timestamp. Also from cwa.h
    d = [
        bitand(bitshift(t, -26),hex2dec('003f'))+2000   % year
        bitand(bitshift(t, -22),hex2dec('000f'))        % month
        bitand(bitshift(t, -17),hex2dec('001f'))        % day
        bitand(bitshift(t, -12),hex2dec('003f'))        % hour
        bitand(bitshift(t, -6),hex2dec('003f'))         % min
        bitand(t,hex2dec('003f'))                       % sec
    ];
    dateSec = datenum(d');
end