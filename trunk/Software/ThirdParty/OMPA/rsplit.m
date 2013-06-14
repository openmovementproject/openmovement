function l = rsplit(r,s)
%L=RSPLIT(R,S) 
%  Splits a string S using the regular expression R.  Meant to work like
%  the PERL split function.  Returns a cell array of strings.   Requires
%  REGEXP.
%
%  This function acts a bit like a dual to REGEXP: it returns all of the
%  strings that don't match the regular expression instead of those that
%  do match. STRREAD is similar to RSPLIT, but it uses a fixed delimiter 
%  set (whitepace) when use '%s'.
%
%Examples:
%    >> rsplit('[_/]+', 'this_is___a_/_string/_//')
%    ans = 
%        'this'    'is'    'a'    'string'
%
%    >> rsplit(',', '$GPGGA,012911.00,7111.04510,N,15841.80861,W')
%    ans =
%        '$GPGGA'  '012911.00'  '7111.04510'  'N'  '15841.80861'  'W'
%
%    >> rsplit(',', '')
%    ans = 
%         {}
%
%    >> rsplit(',', ',')
%    ans = 
%         {}
%
%    >> rsplit('\s+',' 47.590516667   N     122.341633333   W   1     4  ')
%    ans = 
%         ''  '47.590516667'  'N'  '122.341633333'  'W'  '1'  '4'
%
%Implementation inspired by 
% Val Schmidt, Center for Coastal and Ocean Mapping
% University of New Hampshire
%
%by Gerald Dalley (dalleyg@mit.edu), 2007

[delimStarts,delimEnds] = regexp(s,r, 'start','end');

if isempty(delimStarts) 
  if isempty(s)
    l = {};
  else
    l = {s};
  end
  return
elseif (length(delimStarts) == 1) && ...
    (delimStarts(1) == 1) && (delimEnds(end) == length(s)) 
  l={}; 
  return
end

lastChunk = s((delimEnds(end)+1):end);
if isempty(lastChunk)
  l = cell(1, length(delimStarts));
else
  l = cell(1, length(delimStarts)+1);
  l{end} = lastChunk;
end

firstChunk = s(1:delimStarts(1)-1);
if isempty(firstChunk)
  l{1} = ''; % prefer '' (0x0 char array) to a 1x0 char array
else
  l{1} = firstChunk;
end

for i=2:length(delimStarts)
  l{i} = s((delimEnds(i-1)+1):(delimStarts(i)-1));
end
