@ECHO OFF

CD /D %~dp0

SET DOXYGEN=doxygen.exe
IF EXIST "%ProgramFiles(x86)%\doxygen\bin\doxygen.exe" SET DOXYGEN="%ProgramFiles(x86)%\doxygen\bin\doxygen.exe"
IF EXIST "%ProgramFiles%\doxygen\bin\doxygen.exe" SET DOXYGEN="%ProgramFiles%\doxygen\bin\doxygen.exe"
IF %DOXYGEN%!==! GOTO ERROR

%DOXYGEN%

IF ERRORLEVEL 1 GOTO ERROR
GOTO END

:ERROR
ECHO ERROR!
PAUSE
GOTO END

:END
