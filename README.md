# ClaudeOClock

A physical desk gadget — an ESP32-S3 with an AMOLED display — that shows
your Claude usage limits in real time with custom pixel-art animations.

Tracks two sources simultaneously:
- **claude.ai** — captured by the included Chrome extension
- **Claude Code** — captured by polling the Anthropic API (same OAuth token the CLI uses)

Built entirely from scratch with Claude Code.

---

## Hardware

**Waveshare ESP32-S3-Touch-AMOLED-2.16**

- 480×480 AMOLED via QSPI
- BLE 5.0 (no Wi-Fi needed after setup)
- LiPo battery header for wire-free desk use

No 3D printing needed — comes with enclosure.

---

## Quick start

### 1. Flash the firmware

```batch
# Install PlatformIO CLI first: pip install platformio
flash.bat
```

Or manually:
```batch
pio run -d firmware -e claudeoclock_amoled_216 -t upload --upload-port COM3
```

### 2. Generate fonts (required before build)

Install Node.js, then:

```batch
generate_fonts.bat
```

This downloads Inter and generates the four LVGL font files.
After generation, add `-DFONTS_GENERATED` to `build_flags` in `firmware/platformio.ini`.

### 3. Install the Python daemon

```batch
cd daemon
pip install -r requirements.txt
install-windows.bat
```

The daemon runs at login as a Windows Task Scheduler job.

### 4. Install the Chrome extension

Open Chrome → Extensions → Load unpacked → select the `extension/` folder.

Navigate to `claude.ai` — the extension will start sending usage data to the daemon.

---

## How it works

```
claude.ai DOM
     │
     ▼
Chrome extension ──POST──► daemon (127.0.0.1:47821)
                                │
Anthropic API ──────poll────────┤
                                │
                                ▼
                          ESP32 via BLE
                                │
                                ▼
                         AMOLED display
```

The daemon merges both data sources and pushes a JSON payload to the ESP32
over BLE every 60 seconds.

---

## Buttons

| Button | Short press | Long press (>1.5s) |
|--------|-------------|-------------------|
| LEFT (GPIO 0 / BOOT) | HID Space (Claude Code voice PTT) | Bluetooth status screen |
| RIGHT (GPIO 18) | HID Shift+Tab | — |
| PWR (AXP PKEY) | Cycle screens / cycle animations on splash | — |

Touch anywhere → toggle between current screen and splash.

---

## Screens

1. **Splash** — pixel-art creature that gets more frantic as usage climbs
2. **Combined** — side-by-side claude.ai + Claude Code view (default)
3. **claude.ai** — full detail, session + weekly
4. **Claude Code** — full detail, session + weekly + status badge

---

## BLE UUIDs

| Name | UUID |
|------|------|
| Custom service | `12345678-1234-1234-1234-123456789abc` |
| RX characteristic (phone → device) | `12345678-1234-1234-1234-123456789abd` |
| TX characteristic (device → phone) | `12345678-1234-1234-1234-123456789abe` |
| REQ characteristic | `12345678-1234-1234-1234-123456789abf` |
| HID service | `0x1812` (standard) |

---

## Credits

```
ClaudeOClock — built from scratch with Claude Code
By Haris Ahmad Khan
claude.ai + Claude Code dual usage monitor for the desk
Hardware: Waveshare ESP32-S3-Touch-AMOLED-2.16
```
