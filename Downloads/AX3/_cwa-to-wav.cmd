@echo off
setlocal

set Program32=%ProgramFiles%
if not "%ProgramFiles(x86)%"=="" set Program32=%ProgramFiles(x86)%

:next
if "%~1"=="" goto end
echo.
echo ----------------------------------------------------------------------
echo  Before
echo ----------------------------------------------------------------------
echo Processing file: "%~1"

if /i "%~x1"==".CWA" goto ok
echo ERROR: Unknown file type ("%~x1")
pause
shift
goto next
:ok

echo Output file: "%~dpn1.wav"
if exist "%~dpn1.wav" echo Removing existing output file...
if exist "%~dpn1.wav" del "%~dpn1.wav" 
echo ======================================================================

@echo on
"%Program32%\Open Movement\OM GUI\Plugins\OmConvertPlugin\omconvert.exe" "%~1" -out "%~dpn1.wav"
@echo off

echo ----------------------------------------------------------------------
echo  After
echo ----------------------------------------------------------------------
echo ERRORLEVEL=%ERRORLEVEL%
echo Output file: "%~dpn1.wav"
if exist "%~dpn1.wav" echo Exists: true
if not exist "%~dpn1.wav" echo Exists: false
echo ======================================================================

shift
goto next
:end

echo.
echo ----------------------------------------------------------------------
echo  Done
echo ======================================================================
pause
