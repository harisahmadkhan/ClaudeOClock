@echo off
setlocal
echo ClaudeOClock — Font Generator
echo ================================
echo Requires: Node.js, npm

where npm >nul 2>&1
if errorlevel 1 (
    echo ERROR: npm not found. Install Node.js from https://nodejs.org
    pause
    exit /b 1
)

echo Installing lv_font_conv...
call npm install -g lv_font_conv

if not exist fonts mkdir fonts
cd fonts

echo Downloading Inter font...
curl -L "https://github.com/rsms/inter/releases/download/v4.0/Inter-4.0.zip" -o Inter.zip
tar -xf Inter.zip
cd ..

echo Generating LVGL fonts...

lv_font_conv --font fonts/Inter-4.0/extras/ttf/Inter-Bold.ttf -r 0x20-0x7F --size 52 --format lvgl -o firmware/src/font_main_large.c --force-fast-kern-format
lv_font_conv --font fonts/Inter-4.0/extras/ttf/Inter-Regular.ttf -r 0x20-0x7F --size 28 --format lvgl -o firmware/src/font_main_medium.c --force-fast-kern-format
lv_font_conv --font fonts/Inter-4.0/extras/ttf/Inter-Regular.ttf -r 0x20-0x7F --size 18 --format lvgl -o firmware/src/font_main_small.c --force-fast-kern-format
lv_font_conv --font fonts/Inter-4.0/extras/ttf/Inter-Regular.ttf -r 0x20-0x7F --size 24 --format lvgl -o firmware/src/font_mono.c --force-fast-kern-format

echo.
echo Applying LVGL 9 patches...
echo (Removes LVGL 8 guards, removes .cache field, adds required LVGL 9 fields)

python patch_fonts.py firmware/src/font_main_large.c
python patch_fonts.py firmware/src/font_main_medium.c
python patch_fonts.py firmware/src/font_main_small.c
python patch_fonts.py firmware/src/font_mono.c

echo.
echo Done! Add -DFONTS_GENERATED to build_flags in firmware/platformio.ini
echo Then rebuild with: pio run -d firmware -e claudeoclock_amoled_216
pause
