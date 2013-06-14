
**************
Readme to OMPA
**************
% Copyright (c) 2009-2013, Newcastle University, UK.
% All rights reserved.
% 
% Redistribution and use in source and binary forms, with or without 
% modification, are permitted provided that the following conditions are met: 
% 1. Redistributions of source code must retain the above copyright notice, 
%    this list of conditions and the following disclaimer.
% 2. Redistributions in binary form must reproduce the above copyright notice, 
%    this list of conditions and the following disclaimer in the documentation 
%    and/or other materials provided with the distribution.
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
% Cassim Ladha Nov 2012
% cassimladha@gmail.com

Installation Intructions (Assuming Binary Dist Downloaded)

1. Unzip the Package using favorite app (winRar works fine)
2. Install matlab Runtime version 2012b (choose x86 OR x64 version)
3. Put plugin folder into your OMGUI installation folder:
eg. 
	copy "OM PA.dir" ->  "C:\Program Files (x86)\OM Gui beta10\Plugins"

4.(Optional to get tool bar shortcut). Put profile into your working directory
e.g
	copy "pluginsProfile.profile" -> "User\My Documents\Settings" (if settings folder doesnt exist create it)


Use Instructions:
You can launch the plugin from inside OM GUI with either a selection of data or the whole file.

The plugin will run (it can take a while for large captures) and progress is reported in the Queue

Upon completion (assuming HTML report selected) a directory will be created containing the HTML based report and all the gubbins needed for web viweing.

The "index.html" can be launched in a browser (Chrome tested) and the data viewed.
The actual data is JSON based and is within the ".\data\data.js" directory

NJoy!

**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**
****Geek info****
Build instructions (assuming:
Tested build using Matlab R2012b. May work on later builds but will need to distribut with that version of the runtime

FindPA.m does the majority of the work
findPA_win32.m is a simple wrapper to make passing variables from OMGUI nice and consistent

Each of the PA calculations has been abstracted as much as possible.

Dependancies:
AX3_readfile
- https://code.google.com/p/openmovement/source/browse/#svn%2Ftrunk%2FSoftware%2FAX3%2Fcwa-convert%2Fmatlab

jsonlab - http://iso2mesh.sourceforge.net/cgi-bin/index.cgi?jsonlab/Download
- savejson.m
- varargin2struct.m