@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Scripts\tools\Start-HeroHour.ps1" -Mode Packaged
if errorlevel 1 pause
