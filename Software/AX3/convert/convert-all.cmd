@echo off
if "%1"=="" goto error_none

:loop
if "%1"=="" goto end
if /i not "%~x1"==".OMX" goto error_omx
:convert
"%~dp0convert.exe" "%1" -t:serial -stream a -out "%~dpn1-accel.csv"
shift
goto loop

:error_none
echo Drag and drop one or more .OMX files on to the script.
pause
goto end

:error_omx
echo WARNING: File is not an .OMX file:
echo.  %1
echo Press any key to continue anyway, or Ctrl+C,Y to cancel.
pause >NUL
echo.
goto convert

:end
