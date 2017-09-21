@echo off
if not "%~1"=="" (
for %%f in ("%~1") do (
python "%~dp0cwa_metadata.py" "%%~f"
)
)
