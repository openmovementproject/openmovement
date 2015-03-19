@echo off
cd /d %~dp0

::: Check arguments
if "%~1"=="" goto ERROR_NO_SOURCE
rem if not "%~2"=="" goto ERROR_TOO_MANY_ARGS
if not exist "%~1" goto ERROR_SOURCE_NOT_FOUND
set INPUT=%~f1
set OUTPUT=%~dpn1

::: Choose a temporary output folder
set TEMPDIR=%TEMP%\CBR-%RANDOM%
mkdir "%TEMPDIR%"

::: Run the script
echo OMCONVERT: INPUT: %INPUT%
rem  echo INPUT: %INPUT% 1>&2
echo OMCONVERT: OUTPUT: %OUTPUT%
rem  echo OUTPUT: %OUTPUT% 1>&2
echo OMCONVERT: TEMPORARY: %TEMPDIR%
rem  echo TEMPORARY: %TEMPDIR% 1>&2
echo OMCONVERT: Running:    omconvert.exe "%INPUT%" "%TEMPDIR%"
if not exist omconvert.exe echo OMCONVERT: Executable not found...
if exist omconvert.exe omconvert.exe "%INPUT%" -out "%TEMPDIR%\file.wav" -svm-file "%TEMPDIR%\file.svm.csv" -wtv-file "%TEMPDIR%\file.wtv.csv" -paee-file "%TEMPDIR%\file.paee.csv"

::: Move files from the temporary folder
move "%TEMPDIR%\file.wav" "%OUTPUT%.wav" >nul
move "%TEMPDIR%\file.svm.csv" "%OUTPUT%.svm.csv" >nul
move "%TEMPDIR%\file.wtv.csv" "%OUTPUT%.wtv.csv" >nul
move "%TEMPDIR%\file.paee.csv" "%OUTPUT%.paee.csv" >nul
rmdir "%TEMPDIR%"

goto END


:ERROR_NO_SOURCE
echo ERROR: No input file specified. 1>&2
pause
goto END

:ERROR_TOO_MANY_ARGS
echo ERROR: Too many arguments specified. 1>&2
pause
goto END

:ERROR_SOURCE_NOT_FOUND
echo ERROR: Input file not found: %1 1>&2
pause
goto END

:END
