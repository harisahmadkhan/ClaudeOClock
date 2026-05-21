@echo off
setlocal

echo ClaudeOClock Daemon — Windows installer
echo =========================================
echo Registers as a Windows Task Scheduler job that starts at login.
echo Uses pythonw so no console window appears.
echo.

where pythonw >nul 2>&1
if errorlevel 1 (
    echo ERROR: pythonw not found. Install Python from python.org (check "Add to PATH").
    pause
    exit /b 1
)

set SCRIPT_DIR=%~dp0
set SCRIPT_PATH=%SCRIPT_DIR%claudeoclock_daemon.py

echo Installing pip dependencies...
pip install -r "%SCRIPT_DIR%requirements.txt"
if errorlevel 1 (
    echo ERROR: pip install failed.
    pause
    exit /b 1
)

echo.
echo Creating Task Scheduler job...
schtasks /delete /tn "ClaudeOclock Daemon" /f >nul 2>&1
schtasks /create /tn "ClaudeOclock Daemon" /tr "pythonw \"%SCRIPT_PATH%\"" /sc onlogon /rl limited /f

if errorlevel 1 (
    echo ERROR: Could not create scheduled task. Try running as Administrator.
    pause
    exit /b 1
)

echo.
echo Installed! Starting daemon now...
start "" pythonw "%SCRIPT_PATH%"

echo.
echo Done. The daemon will start automatically at next login.
echo Log file: %%USERPROFILE%%\.config\claudeoclock\daemon.log
pause
