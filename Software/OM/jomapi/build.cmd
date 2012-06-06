@ECHO OFF
rem Daniel Jackson, 2006-2012

ECHO Checking Java environment variable...
IF NOT "%JAVA_HOME%"=="" GOTO JAVA_COMPILER
ECHO ERROR: 'JAVA_HOME' environment variable not set.
GOTO END

:JAVA_COMPILER
ECHO Compiling Java file...
"%JAVA_HOME%\bin\javac.exe" JOMAPI.java
IF ERRORLEVEL 1 GOTO ERROR

ECHO Creating JNI header file...
"%JAVA_HOME%\bin\javah.exe" -classpath . -jni JOMAPI
IF ERRORLEVEL 1 GOTO ERROR

IF EXIST "%VS100COMNTOOLS%\vsvars32.bat" SET COMNTOOLS=%VS100COMNTOOLS%
IF NOT "%COMNTOOLS%"=="" GOTO C_COMPILER
ECHO ERROR: No known 'VS##COMNTOOLS' environment variable set - couldn't find vsvars32.bat for C compiler.
GOTO END

:C_COMPILER
SET PLATFORM=%1
if %PLATFORM%!==! echo Platform not specified (x86 / x64), defaulting to x86...
if %PLATFORM%!==! SET PLATFORM=x86
ECHO Setting environment variables for C compiler (%PLATFORM%)...
call "%COMNTOOLS%\..\..\VC\vcvarsall.bat" %PLATFORM%

ECHO Compiling JNI file...
cl -c /D WIN32 /EHsc /I "%JAVA_HOME%\include" /I "%JAVA_HOME%\include\win32" /I "..\omapi\include" JOMAPI.c "..\omapi\src\DeviceFinder.cpp" "..\omapi\src\omapi-devicefinder.cpp" "..\omapi\src\omapi-download.c" "..\omapi\src\omapi-internal.c" "..\omapi\src\omapi-main.c" "..\omapi\src\omapi-reader.c" "..\omapi\src\omapi-settings.c" "..\omapi\src\omapi-status.c"
IF ERRORLEVEL 1 GOTO ERROR

ECHO Linking JNI files...
link /dll /defaultlib:user32.lib JOMAPI DeviceFinder omapi-devicefinder omapi-download omapi-internal omapi-main omapi-reader omapi-settings omapi-status
IF ERRORLEVEL 1 GOTO ERROR

rem ECHO Copying DLL file...
rem copy /Y JOMAPI.dll "%JAVA_HOME%\bin"


rem GOTO END


:RUN
ECHO Compiling test program...
"%JAVA_HOME%\bin\javac.exe" JOM.java
IF ERRORLEVEL 1 GOTO ERROR

ECHO Running test program...
java.exe -cp . JOM
IF ERRORLEVEL 1 GOTO ERROR
GOTO END


:ERROR
ECHO ERROR: An error occured.
pause
GOTO END

:END
