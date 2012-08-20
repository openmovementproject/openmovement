@ECHO OFF
rem Daniel Jackson, 2012

SETLOCAL
CD /D %~dp0

IF NOT EXIST "%1" GOTO FILE_ERROR

IF EXIST "%VS100COMNTOOLS%\vsvars32.bat" SET COMNTOOLS=%VS100COMNTOOLS%
IF "%COMNTOOLS%"=="" GOTO COMPILER_ERROR

ECHO Setting environment variables for C compiler...
call "%COMNTOOLS%\..\..\VC\vcvarsall.bat" x86

ECHO Compiling...
cl -c /D WIN32 /EHsc /Tc"%1"
IF ERRORLEVEL 1 GOTO ERROR

ECHO Linking... %PLATFORM%
link /defaultlib:user32.lib %~dpn1.obj /out:%~dpn1.exe
IF ERRORLEVEL 1 GOTO ERROR
GOTO END

:FILE_ERROR
ECHO ERROR: Source file not specific or not found:  %1
GOTO ERROR

:COMPILER_ERROR
ECHO ERROR: No known 'VS##COMNTOOLS' environment variable set - couldn't find vsvars32.bat for C compiler.
GOTO ERROR

:ERROR
ECHO ERROR: An error occured.
pause
GOTO END

:END
ENDLOCAL
