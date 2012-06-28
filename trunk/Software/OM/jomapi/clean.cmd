@ECHO OFF
rem Daniel Jackson, 2006-2012
SETLOCAL
CD /D %~dp0

ECHO Cleaning...

rem Objects / library compiled code
if exist *.obj del *.obj
if exist bin\*.exp del bin\*.exp
if exist bin\*.lib del bin\*.lib

rem Java compiled code
if exist class rmdir /s /q class

rem Visual Studio folders
if exist *.user del *.user
if exist Release rmdir /s /q Release
if exist Debug rmdir /s /q Debug


:END
ENDLOCAL
