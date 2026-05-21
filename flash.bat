@echo off
setlocal enabledelayedexpansion

echo ClaudeOClock Flash Tool
echo =======================

REM Find ESP32-S3 COM port
set FOUND_PORT=
for /f "tokens=*" %%i in ('mode 2^>nul') do (
    echo %%i | find "COM" >nul
    if not errorlevel 1 (
        set LINE=%%i
        set PORT=!LINE:~4,4!
        set PORT=!PORT: =!
        if not "!PORT!"=="" set FOUND_PORT=!PORT!
    )
)

REM If only one COM port, use it; otherwise list them
if "%FOUND_PORT%"=="" (
    echo No COM ports found. Is the board plugged in?
    pause
    exit /b 1
)

echo Detected port: %FOUND_PORT%
echo.
set /p CONFIRM_PORT="Use %FOUND_PORT%? Press Enter to confirm or type another port (e.g. COM5): "
if not "%CONFIRM_PORT%"=="" set FOUND_PORT=%CONFIRM_PORT%

echo.
echo Building and flashing to %FOUND_PORT%...
pio run -d firmware -e claudeoclock_amoled_216 -t upload --upload-port %FOUND_PORT%

if errorlevel 1 (
    echo.
    echo Flash FAILED. Check the port and try again.
    pause
    exit /b 1
)

echo.
echo Flash complete! Opening serial monitor...
pio device monitor --port %FOUND_PORT% --baud 115200
