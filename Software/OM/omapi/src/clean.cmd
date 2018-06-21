@ECHO OFF
SETLOCAL
CD /D %~dp0

ECHO Cleaning...

rem Objects / library compiled code
if exist *.obj del *.obj
if exist libomapi.exp del libomapi.exp
if exist libomapi.lib del libomapi.lib
if exist libomapi.def del libomapi.def

:END
ENDLOCAL
