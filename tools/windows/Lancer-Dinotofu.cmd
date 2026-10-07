@echo off
chcp 65001 >nul
setlocal
set PYTHONUTF8=1
set PYTHONIOENCODING=utf-8
set LANG=C.UTF-8
set LC_ALL=C.UTF-8
if exist "%~dp0DinotofuLauncher.ps1" (
    powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0DinotofuLauncher.ps1" -Mode Auto
) else if exist "%~dp0tools\windows\DinotofuLauncher.ps1" (
    powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\windows\DinotofuLauncher.ps1" -Mode Auto
) else (
    echo DinotofuLauncher.ps1 introuvable.
    pause
)
exit /b
