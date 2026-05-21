#!/usr/bin/env python3
"""
Patches lv_font_conv output from LVGL 8 format to LVGL 9 format.
Run after generate_fonts.bat.

Changes made:
  - Removes #if LVGL_VERSION_MAJOR >= 8 / #endif guards (keeps contents)
  - Removes .cache field from lv_font_t struct
  - Adds required LVGL 9 fields: .release_glyph, .kerning, .static_bitmap_data, .fallback, .user_data
"""
import sys
import re

def patch(path: str):
    with open(path, 'r', encoding='utf-8') as f:
        src = f.read()

    # Remove LVGL 8 version guards (keep content between them)
    src = re.sub(r'#if\s+LVGL_VERSION_MAJOR\s*>=\s*8\s*\n', '', src)
    src = re.sub(r'#endif\s*/\*\s*LVGL_VERSION_MAJOR\s*\*/\s*\n', '', src)

    # Remove .cache field line
    src = re.sub(r'\s*\.cache\s*=\s*\{.*?\},?\n', '\n', src, flags=re.DOTALL)

    # Insert LVGL 9 required fields before closing }; of the lv_font_t struct
    # Look for the last field before the closing };
    lvgl9_fields = (
        "    .release_glyph = NULL,\n"
        "    .kerning = NULL,\n"
        "    .static_bitmap_data = true,\n"
        "    .fallback = NULL,\n"
        "    .user_data = NULL,\n"
    )
    # Insert before the closing }; of the font struct
    src = re.sub(
        r'(\s*\.(get_glyph_dsc|line_height|subpx|underline_position)\s*=.*?\n)(\s*\};)',
        lambda m: m.group(1) + lvgl9_fields + m.group(3),
        src,
        count=1
    )

    with open(path, 'w', encoding='utf-8') as f:
        f.write(src)

    print(f"  Patched: {path}")

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: patch_fonts.py <font_file.c>")
        sys.exit(1)
    for p in sys.argv[1:]:
        patch(p)
