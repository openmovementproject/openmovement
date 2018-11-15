@echo off
cd /d %~dp0
echo AX3 Bootload: %~n0
booter.exe -copy 0x2A000 8 -timeout 15 "%~n0.hex"
