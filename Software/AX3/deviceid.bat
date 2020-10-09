@echo off

rem Single use example:
rem     PowerShell -Command "& {$f='D:\CWA-DATA.CWA';$s=[System.IO.File]::OpenRead($f);$b=New-Object byte[] 13;$c=$s.Read($b,0,13);$s.close();$id=16777216*$b[12]+65536*$b[11]+256*$b[6]+$b[5];Write-Output($id);[Console]::ReadKey()}"

set FILECOUNT=0
:next
set FILE=%~1
if "%FILE%%FILECOUNT%"=="0" call :choosefile
if "%FILE%%FILECOUNT%"=="0" goto cancel
if "%FILE%"=="" goto end
set /A FILECOUNT=FILECOUNT+1
call :process "%FILE%"
shift
goto next

:cancel
ECHO.--- No file selected, listing connected devices... ---
for %%D in (C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z) do (
  if exist %%D:\CWA-DATA.CWA (
    call :process "%%D:\CWA-DATA.CWA"
  )
)
goto end

:process
set FILE=%~1
if /i not "%~x1"==".CWA" (
  echo WARNING, Unknown file type '%~x1' for file %~1
)
PowerShell -Command "& {$f='%FILE%';$s=[System.IO.File]::OpenRead($f);$b=New-Object byte[] 13;$c=$s.Read($b,0,13);$s.close();$id=16777216*$b[12]+65536*$b[11]+256*$b[6]+$b[5];Write-Output($f+','+$id);}"
goto :eof

:choosefile
set DIALOG="about:<input type=file id=file><script>file.click();new ActiveXObject('Scripting.FileSystemObject').GetStandardStream(1).WriteLine(file.value);close();resizeTo(0,0);</script>"
for /f "tokens=* delims=" %%f in ('mshta.exe %DIALOG%') do set "FILE=%%f"
goto :eof

:end
pause
