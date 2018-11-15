@echo off
cd /d %~dp0
echo AX6 Bootload: %~n0
booter.exe -copy 0x3A800 8 -timeout 15 "%~n0.hex"
