@ECHO OFF
SETLOCAL
CD /D %~dp0

IF EXIST "%VS71COMNTOOLS%\vsvars32.bat"  SET COMNTOOLS=%VS71COMNTOOLS%
IF EXIST "%VS80COMNTOOLS%\vsvars32.bat"  SET COMNTOOLS=%VS80COMNTOOLS%
IF EXIST "%VS90COMNTOOLS%\vsvars32.bat"  SET COMNTOOLS=%VS90COMNTOOLS%
IF EXIST "%VS100COMNTOOLS%\vsvars32.bat" SET COMNTOOLS=%VS100COMNTOOLS%
IF EXIST "%VS110COMNTOOLS%\vsvars32.bat" SET COMNTOOLS=%VS110COMNTOOLS%
IF EXIST "%VS120COMNTOOLS%\vsvars32.bat" SET COMNTOOLS=%VS120COMNTOOLS%
IF EXIST "%VS130COMNTOOLS%\vsvars32.bat" SET COMNTOOLS=%VS130COMNTOOLS%
IF EXIST "%VS140COMNTOOLS%\vsvars32.bat" SET COMNTOOLS=%VS140COMNTOOLS%

IF NOT "%COMNTOOLS%"=="" GOTO C_COMPILER
ECHO ERROR: No known 'VS###COMNTOOLS' environment variable set - couldn't find vsvars32.bat for C compiler.
GOTO END

:C_COMPILER
ECHO Setting environment variables for C compiler...
call "%COMNTOOLS%\..\..\VC\vcvarsall.bat"

ECHO Compiling...
cl -c /D WIN32 /EHsc /I "..\..\include" /Tc"locktest.c" /Tp"..\..\src\DeviceFinder.cpp" /Tp"..\..\src\omapi-devicefinder.cpp" /Tc"..\..\src\omapi-download.c" /Tc"..\..\src\omapi-internal.c" /Tc"..\..\src\omapi-main.c" /Tc"..\..\src\omapi-reader.c" /Tc"..\..\src\omapi-settings.c" /Tc"..\..\src\omapi-status.c"
IF ERRORLEVEL 1 GOTO ERROR

ECHO Linking...
link locktest /defaultlib:user32.lib DeviceFinder omapi-devicefinder omapi-download omapi-internal omapi-main omapi-reader omapi-settings omapi-status /out:locktest.exe
IF ERRORLEVEL 1 GOTO ERROR

GOTO END

:ERROR
ECHO ERROR: An error occured.
pause
GOTO END

:END
ENDLOCAL
