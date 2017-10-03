@echo off
setlocal

::: Set epoch size, e.g. to 1, 10, or 60 (seconds)
set EPOCH=10

::: Set to 1 to output raw files at 30Hz (in a format that can be converted from raw to epoch with ActiLife software)
set USE_RAW=

:check_req
set OMCONVERT=%~dp0omconvert.exe
if exist "%OMCONVERT%" goto check_args
set Program32=%ProgramFiles%
if not "%ProgramFiles(x86)%"=="" set Program32=%ProgramFiles(x86)%
set OMCONVERT=%Program32%\Open Movement\OM GUI\Plugins\OmConvertPlugin\omconvert.exe
if exist "%OMCONVERT%" goto check_args
echo ERROR: omconvert.exe not found -- missing local copy or check 'OMGUI' installed in the default location?
echo Looked at: %OMCONVERT%
pause
goto eof

:check_args
if not "%~1"=="" goto start
echo ERROR: No files specified.  Drag-and-drop .CWA files on to this batch file.
pause
goto eof

:start

:next
if "%~1"=="" goto end
echo.
echo ----------------------------------------------------------------------
echo  Processing file: "%~1"
echo ----------------------------------------------------------------------

if /i "%~x1"==".CWA" goto file_type_ok
echo ERROR: Unknown file type ("%~x1"): %~1
pause
shift
goto next

:file_type_ok
if exist "%~1" goto file_exist
echo ERROR: File does not exist: %~1
pause
shift
goto next

:file_exist
set RAW_OPTION=
if %USE_RAW%!==1! set RAW_OPTION=-csv-file "%~dpn1.raw30ag.csv"
@echo on
"%OMCONVERT%" "%~1" -resample 30 -csv-format:ag %RAW_OPTION% -counts-epoch %EPOCH% -counts-file "%~dpn1.counts%EPOCH%.csv"
@echo off

echo ERRORLEVEL=%ERRORLEVEL%
echo ======================================================================

shift
goto next
:end

echo.
echo ----------------------------------------------------------------------
echo  Done
echo ======================================================================
pause
:eof
