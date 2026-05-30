@echo off
setlocal

echo ClaudeOClock Daemon — Windows installer
echo =========================================
echo Registers as a Windows Task Scheduler job that starts at login.
echo Uses pythonw so no console window appears.
echo.
echo IMPORTANT: Complete WiFi setup on the device first, then run this script.
echo The device must be reachable at claudeoclock.local before continuing.
echo.

where pythonw >nul 2>&1
if errorlevel 1 (
    echo ERROR: pythonw not found. Install Python from python.org (check "Add to PATH").
    pause
    exit /b 1
)

set SCRIPT_DIR=%~dp0
set SCRIPT_PATH=%SCRIPT_DIR%claudeoclock_daemon.py
set TOKEN_DIR=%USERPROFILE%\.config\claudeoclock
set TOKEN_PATH=%TOKEN_DIR%\token.txt

echo Step 1: Installing pip dependencies...
pip install httpx
if errorlevel 1 (
    echo ERROR: pip install failed.
    pause
    exit /b 1
)

echo.
echo Step 2: Fetching device token from claudeoclock.local...
if not exist "%TOKEN_DIR%" mkdir "%TOKEN_DIR%"

curl -sf --max-time 10 "http://claudeoclock.local/setup/token" -o "%TOKEN_PATH%"
if errorlevel 1 (
    echo ERROR: Could not reach claudeoclock.local
    echo Make sure the device is on the same WiFi network and try again.
    pause
    exit /b 1
)

echo Token saved to %TOKEN_PATH%

echo.
echo Step 3: Creating Task Scheduler job...
schtasks /delete /tn "ClaudeOclock Daemon" /f >nul 2>&1
schtasks /create /tn "ClaudeOclock Daemon" /tr "pythonw \"%SCRIPT_PATH%\"" /sc onlogon /rl limited /f

if errorlevel 1 (
    echo ERROR: Could not create scheduled task. Try running as Administrator.
    pause
    exit /b 1
)

echo.
echo Step 4: Starting daemon now...
start "" pythonw "%SCRIPT_PATH%"

echo.
echo Done! The daemon will start automatically at next login.
echo Log file: %TOKEN_DIR%\daemon.log
pause
