@ECHO OFF
rem Daniel Jackson, 2006-2012
SETLOCAL
CD /D %~dp0

ECHO Checking Java environment variable...
IF NOT "%JAVA_HOME%"=="" GOTO JAVA_COMPILER
ECHO ERROR: 'JAVA_HOME' environment variable not set.
GOTO END

:JAVA_COMPILER
ECHO Compiling Java files...
if not exist class mkdir class
"%JAVA_HOME%\bin\javac.exe" -classpath class -d class -sourcepath src src/openmovement/JOM.java
IF ERRORLEVEL 1 GOTO ERROR

ECHO Creating JNI header file...
"%JAVA_HOME%\bin\javah.exe" -classpath class -d c -jni openmovement.JOMAPI
IF ERRORLEVEL 1 GOTO ERROR

IF EXIST "%VS100COMNTOOLS%\vsvars32.bat" SET COMNTOOLS=%VS100COMNTOOLS%
IF NOT "%COMNTOOLS%"=="" GOTO C_COMPILER
ECHO ERROR: No known 'VS##COMNTOOLS' environment variable set - couldn't find vsvars32.bat for C compiler.
GOTO END

:C_COMPILER
SET PLATFORM=%1
IF NOT %PLATFORM%!==! GOTO PLATFORM_SPECIFIED
echo Platform not specified, auto-detecting...
SET PLATFORM=x86
IF NOT "%ProgramFiles(x86)%"=="" SET PLATFORM=x64

:PLATFORM_SPECIFIED
echo Platform is: %PLATFORM%
ECHO Setting environment variables for C compiler (%PLATFORM%)...
call "%COMNTOOLS%\..\..\VC\vcvarsall.bat" %PLATFORM%

ECHO Compiling JNI file...
cl -c /D WIN32 /EHsc /I "%JAVA_HOME%\include" /I "%JAVA_HOME%\include\win32" /I "..\omapi\include" /Tc"c\JOMAPI.c" /Tp"..\omapi\src\DeviceFinder.cpp" /Tp"..\omapi\src\omapi-devicefinder.cpp" /Tc"..\omapi\src\omapi-download.c" /Tc"..\omapi\src\omapi-internal.c" /Tc"..\omapi\src\omapi-main.c" /Tc"..\omapi\src\omapi-reader.c" /Tc"..\omapi\src\omapi-settings.c" /Tc"..\omapi\src\omapi-status.c"
IF ERRORLEVEL 1 GOTO ERROR

ECHO Linking JNI files... %PLATFORM%
SET POSTFIX=
IF /I %PLATFORM%!==x86! SET POSTFIX=32
IF /I %PLATFORM%!==x64! SET POSTFIX=64
rem  "%JAVA_HOME%\lib\jvm.lib" 
link /dll /defaultlib:user32.lib JOMAPI DeviceFinder omapi-devicefinder omapi-download omapi-internal omapi-main omapi-reader omapi-settings omapi-status /out:bin\JOMAPI%POSTFIX%.dll
IF ERRORLEVEL 1 GOTO ERROR

rem ECHO Copying DLL file...
rem copy /Y JOMAPI%POSTFIX%.dll "%JAVA_HOME%\bin"


rem GOTO END


:MAKE_JAR
jar cvfe lib\JOMAPI.jar openmovement.JOM -C class openmovement\JOMAPI.class -C class openmovement\JOM.class -C class openmovement\JOMAPIListener.class
IF ERRORLEVEL 1 GOTO ERROR

:RUN_TEST
ECHO Running test program...

SET JAVA=java.exe
IF %COMPUTERNAME%-%PLATFORM%==DINGO-x86 set JAVA=C:\Program Files (x86)\Java\jre6\bin\java.exe

"%JAVA%" -Djava.library.path="./bin" -jar lib\JOMAPI.jar openmovement.JOM
IF ERRORLEVEL 1 GOTO ERROR
GOTO END


:ERROR
ECHO ERROR: An error occured.
pause
GOTO END

:END
ENDLOCAL
