@echo off
chcp 65001 >nul
setlocal
set PYTHONUTF8=1
set PYTHONIOENCODING=utf-8
set LANG=C.UTF-8
set LC_ALL=C.UTF-8
rem Visible launch: the player can see update check, download and install before the GUI opens.
rem The window closes by itself once the GUI/backend has started.
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0DinotofuLauncher.ps1" -Mode Auto
exit /b
