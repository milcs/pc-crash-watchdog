@echo off
setlocal
::Running watchdog
del /S pc-crash-watchdog.log >nul 2>&1
:: powershell.exe -File pc-crash-watchdog.ps1 -reset_now yes
pwsh -File pc-crash-watchdog.ps1 -reset_now yes
pause
endlocal
exit