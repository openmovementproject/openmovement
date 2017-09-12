@echo off
setlocal

:check_req
set Program32=%ProgramFiles%
if not "%ProgramFiles(x86)%"=="" set Program32=%ProgramFiles(x86)%
set OMCONVERT=%Program32%\Open Movement\OM GUI\Plugins\OmConvertPlugin\omconvert.exe
if exist "%OMCONVERT%" goto check_args
echo ERROR: omconvert.exe not found -- is 'OMGUI' installed in the default location?
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

::: Remove existing temporary files
if exist "%~dpn1.svm.csv.part" del "%~dpn1.svm.csv.part" >nul
if exist "%~dpn1.wtv.csv.part" del "%~dpn1.wtv.csv.part" >nul
if exist "%~dpn1.paee.csv.part" del "%~dpn1.paee.csv.part" >nul

@echo on
"%OMCONVERT%" "%~1" -svm-file "%~dpn1.svm.csv.part" -wtv-file "%~dpn1.wtv.csv.part" -paee-file "%~dpn1.paee.csv.part"
@echo off

echo ERRORLEVEL=%ERRORLEVEL%

::: Remove existing output files
: if exist "%~dpn1.svm.csv" del "%~dpn1.svm.csv" >nul
: if exist "%~dpn1.wtv.csv" del "%~dpn1.wtv.csv" >nul
: if exist "%~dpn1.paee.csv" del "%~dpn1.paee.csv" >nul

::: Move temporary files to output
move /y "%~dpn1.svm.csv.part" "%~dpn1.svm.csv" >nul
move /y "%~dpn1.wtv.csv.part" "%~dpn1.wtv.csv" >nul
move /y "%~dpn1.paee.csv.part" "%~dpn1.paee.csv" >nul
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
