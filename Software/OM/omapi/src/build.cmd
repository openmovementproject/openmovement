@ECHO OFF
SETLOCAL
CD /D %~dp0
@echo on

IF EXIST "%VS70COMNTOOLS%\..\..\VC\vcvarsall.bat" SET VCVARSALL=%VS70COMNTOOLS%\..\..\VC\vcvarsall.bat
IF EXIST "%VS71COMNTOOLS%\..\..\VC\vcvarsall.bat" SET VCVARSALL=%VS71COMNTOOLS%\..\..\VC\vcvarsall.bat
IF EXIST "%VS80COMNTOOLS%\..\..\VC\vcvarsall.bat" SET VCVARSALL=%VS80COMNTOOLS%\..\..\VC\vcvarsall.bat
IF EXIST "%VS90COMNTOOLS%\..\..\VC\vcvarsall.bat" SET VCVARSALL=%VS90COMNTOOLS%\..\..\VC\vcvarsall.bat
IF EXIST "%VS100COMNTOOLS%\..\..\VC\vcvarsall.bat" SET VCVARSALL=%VS100COMNTOOLS%\..\..\VC\vcvarsall.bat
IF EXIST "%VS110COMNTOOLS%\..\..\VC\vcvarsall.bat" SET VCVARSALL=%VS110COMNTOOLS%\..\..\VC\vcvarsall.bat
IF EXIST "%VS120COMNTOOLS%\..\..\VC\vcvarsall.bat" SET VCVARSALL=%VS120COMNTOOLS%\..\..\VC\vcvarsall.bat
IF EXIST "%VS130COMNTOOLS%\..\..\VC\vcvarsall.bat" SET VCVARSALL=%VS130COMNTOOLS%\..\..\VC\vcvarsall.bat
IF EXIST "%VS140COMNTOOLS%\..\..\VC\vcvarsall.bat" SET VCVARSALL=%VS140COMNTOOLS%\..\..\VC\vcvarsall.bat
IF EXIST "%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" SET VCVARSALL=%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat

ECHO Setting environment variables for C compiler... %VCVARSALL%
call "%VCVARSALL%" x86

ECHO.
ECHO LIB=%LIB%
ECHO.
ECHO INCLUDE=%INCLUDE%
ECHO.
ECHO LIBPATH=%LIBPATH%
ECHO.
ECHO WINDOWSSDKDIR=%WindowsSdkDir%
ECHO.
ECHO WINDOWSSDKVERSION=%WindowsSDKVersion%
ECHO.


:COMPILE
ECHO Compiling...
cl -c /DNO_MMAP /EHsc /I"..\include" /Tc"omapi-download.c" /Tc"omapi-internal.c" /Tc"omapi-main.c" /Tc"omapi-reader.c" /Tc"omapi-settings.c" /Tc"omapi-status.c" /Tp"omapi-devicefinder-win.cpp"
IF ERRORLEVEL 1 GOTO ERROR

:LINK
ECHO Linking...
link /dll /out:libomapi.dll omapi-download omapi-internal omapi-main omapi-reader omapi-settings omapi-status omapi-devicefinder-win
IF ERRORLEVEL 1 GOTO ERROR

GOTO END

:ERROR
ECHO ERROR: An error occured.
pause
GOTO END

:END
ENDLOCAL
