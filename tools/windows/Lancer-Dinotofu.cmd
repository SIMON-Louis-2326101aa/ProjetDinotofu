@echo off
setlocal
set PYTHONUTF8=1
set PYTHONIOENCODING=utf-8
set LANG=C.UTF-8
set LC_ALL=C.UTF-8
if exist "%~dp0Lancer-Dinotofu.vbs" (
    wscript.exe "%~dp0Lancer-Dinotofu.vbs"
) else (
    start "" /b powershell.exe -WindowStyle Hidden -NoProfile -ExecutionPolicy Bypass -File "%~dp0DinotofuLauncher.ps1" -Mode Auto
)
exit /b
