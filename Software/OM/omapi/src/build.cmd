@ECHO OFF
SETLOCAL
CD /D %~dp0
@echo on

set ARCH=x86

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
IF EXIST "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" SET VCVARSALL=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat
ECHO Setting environment variables for C compiler... %VCVARSALL%
call "%VCVARSALL%" %ARCH%

ECHO.
ECHO ARCH=%ARCH%
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
cl -D_WINDLL -c /DNO_MMAP /EHsc /I"..\include" /Tc"omapi-download.c" /Tc"omapi-internal.c" /Tc"omapi-main.c" /Tc"omapi-reader.c" /Tc"omapi-settings.c" /Tc"omapi-status.c" /Tp"omapi-devicefinder-win.cpp"
IF ERRORLEVEL 1 GOTO ERROR

:LINK
ECHO Linking...
link /dll /out:libomapi.dll omapi-download omapi-internal omapi-main omapi-reader omapi-settings omapi-status omapi-devicefinder-win
IF ERRORLEVEL 1 GOTO ERROR

:DEF
ECHO Creating an import library .lib...
rem --- Parse the exports to create a .DEF file ---
ECHO.LIBRARY LIBOMAPI>libomapi.def
ECHO.EXPORTS>>libomapi.def
FOR /F "usebackq tokens=1,4 delims= " %%F IN (`dumpbin /exports libomapi.dll ^| FINDSTR /X /R /C:"^  *[0-9][0-9]*  *[0-9A-F][0-9A-F]*  *[0-9A-F][0-9A-F]*  *[A-Za-z_][A-Za-z_0-9]*$"`) DO (
  rem ECHO.  %%G  @%%F>>libomapi.def
  ECHO.  %%G>>libomapi.def
)
lib /def:libomapi.def /out:libomapi.lib /machine:%ARCH%
rem dumpbin /EXPORTS libomapi.lib
rem dumpbin /LINKERMEMBER libomapi.lib
IF ERRORLEVEL 1 GOTO ERROR

:TEST
ECHO Building test program...
cl /EHsc /DOMAPI_DYNLIB_IMPORT /Dtest_main=main /I"..\include" /Tc"test.c"
IF ERRORLEVEL 1 GOTO ERROR
link /out:test.exe test libomapi.lib
IF ERRORLEVEL 1 GOTO ERROR
GOTO END

:ERROR
ECHO ERROR: An error occured.
pause
GOTO END

:END
ENDLOCAL
