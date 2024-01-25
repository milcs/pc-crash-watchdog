@echo off
setlocal
::Running watchdog
del /S pc-crash-watchdog.log >nul 2>&1
:: powershell.exe -File pc-crash-watchdog.ps1
pwsh -File pc-crash-watchdog.ps1

endlocal
exit