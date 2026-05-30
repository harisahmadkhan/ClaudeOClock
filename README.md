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
- Wi-Fi 802.11 b/g/n (data transport)
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

### 2. Connect the device to Wi-Fi

On first boot the device shows the **WiFi Setup** screen automatically.
Tap your network from the scan list, scan the QR code with your phone
(or visit `http://192.168.4.1`), enter your Wi-Fi password, and tap Connect.
The device reboots into normal mode once connected.

### 3. Install the Python daemon

**Run this after the device is on Wi-Fi** — the installer fetches the auth
token from the device automatically.

```batch
cd daemon
install-windows.bat
```

The daemon runs at login as a Windows Task Scheduler job and pushes data
to the device at `http://claudeoclock.local/data` every 60 seconds.

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
                     ESP32 over Wi-Fi (HTTP POST)
                     http://claudeoclock.local/data
                                │
                                ▼
                         AMOLED display
```

The daemon merges both data sources and pushes a JSON payload to the ESP32
over Wi-Fi every 60 seconds. The device is always reachable by hostname —
no IP address needed.

---

## Buttons

| Button | Short press | Long press (>2s) |
|--------|-------------|-----------------|
| PWR (AXP PKEY) | Cycle screens / cycle animations on splash | WiFi setup screen |
| LEFT (GPIO 0 / BOOT) | Reserved | — |
| RIGHT (GPIO 18) | Reserved | — |

Touch anywhere → toggle between current screen and splash.

---

## Screens

1. **Splash** — pixel-art creature that gets more frantic as usage climbs
2. **Combined** — side-by-side claude.ai + Claude Code view (default)
3. **claude.ai** — full detail, session + weekly
4. **Claude Code** — full detail, session + weekly + status badge
5. **WiFi Setup** — network picker + QR code provisioning (first boot or PWR long press)

---

## Credits

```
built and designed with curiosity by Haris Ahmad Khan
```
