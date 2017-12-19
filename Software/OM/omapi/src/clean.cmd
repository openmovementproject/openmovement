@ECHO OFF
SETLOCAL
CD /D %~dp0

ECHO Cleaning...

rem Objects / library compiled code
if exist *.obj del *.obj
if exist *.exp del *.exp
if exist *.lib del *.lib

:END
ENDLOCAL
