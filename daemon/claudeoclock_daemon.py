"""
ClaudeOClock Daemon — Windows
Merges claude.ai data (from Chrome extension) with Claude Code API data
and pushes to the ESP32 over BLE every 60 seconds.

Dependencies: pip install bleak httpx
"""

import asyncio
import json
import logging
import os
import sys
import threading
import time
from datetime import datetime, timezone
from http.server import BaseHTTPRequestHandler, HTTPServer
from pathlib import Path

import httpx
from bleak import BleakClient, BleakScanner

# ── Logging ────────────────────────────────────────────────────────────────

LOG_DIR = Path.home() / ".config" / "claudeoclock"
LOG_DIR.mkdir(parents=True, exist_ok=True)

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    handlers=[
        logging.FileHandler(LOG_DIR / "daemon.log", encoding="utf-8"),
        logging.StreamHandler(sys.stdout),
    ],
)
log = logging.getLogger("claudeoclock")

# ── BLE constants ──────────────────────────────────────────────────────────

DEVICE_NAME   = "ClaudeOclock"
SERVICE_UUID  = "c1a7d001-c1a7-d001-c1a7-d001c1a7d001"
RX_CHAR_UUID  = "c1a7d001-c1a7-d001-c1a7-d001c1a7d002"
TX_CHAR_UUID  = "c1a7d001-c1a7-d001-c1a7-d001c1a7d003"
REQ_CHAR_UUID = "c1a7d001-c1a7-d001-c1a7-d001c1a7d004"

BLE_ADDRESS_CACHE = LOG_DIR / "ble-address.txt"

# ── Anthropic API constants ────────────────────────────────────────────────

API_URL = "https://api.anthropic.com/v1/messages"
API_POLL_INTERVAL = 60  # seconds

CREDENTIAL_PATHS = [
    Path.home() / ".claude" / ".credentials.json",
    Path(os.environ.get("APPDATA", "")) / "Claude" / "claude_desktop_config.json",
    Path.home() / "AppData" / "Roaming" / "Claude" / ".credentials.json",
]

# ── Shared state ───────────────────────────────────────────────────────────

ai_data_lock = threading.Lock()
ai_data = {
    "ai_session_pct":        -1,
    "ai_session_reset_mins": -1,
    "ai_weekly_pct":         -1,
    "ai_weekly_reset_mins":  -1,
}

cc_data_lock = threading.Lock()
cc_data = {
    "s":  0,
    "sr": -1,
    "w":  0,
    "wr": -1,
    "st": "unknown",
    "ok": False,
}

# ── HTTP server (receives claude.ai data from extension) ───────────────────

class UsageHandler(BaseHTTPRequestHandler):
    def do_POST(self):
        if self.path != "/usage":
            self.send_response(404)
            self.end_headers()
            return
        try:
            length = int(self.headers.get("Content-Length", 0))
            body = self.rfile.read(length)
            data = json.loads(body)
            with ai_data_lock:
                for k in ai_data:
                    if k in data:
                        ai_data[k] = data[k]
            log.debug("Extension update: session=%s%% weekly=%s%%",
                      ai_data["ai_session_pct"], ai_data["ai_weekly_pct"])
            self.send_response(200)
        except Exception as e:
            log.warning("Bad extension payload: %s", e)
            self.send_response(400)
        self.end_headers()

    def log_message(self, *args):
        pass  # suppress request log noise


def start_http_server():
    server = HTTPServer(("127.0.0.1", 47821), UsageHandler)
    log.info("HTTP server listening on 127.0.0.1:47821")
    server.serve_forever()


# ── Token loader ───────────────────────────────────────────────────────────

def load_oauth_token() -> str | None:
    for path in CREDENTIAL_PATHS:
        if not path.exists():
            continue
        try:
            with open(path, encoding="utf-8") as f:
                obj = json.load(f)
            # Direct key
            if "accessToken" in obj:
                return obj["accessToken"]
            # Nested under claudeAiOauth
            nested = obj.get("claudeAiOauth", {})
            if "accessToken" in nested:
                return nested["accessToken"]
        except Exception as e:
            log.debug("Could not read %s: %s", path, e)
    log.warning("OAuth token not found in any credential path.")
    return None


# ── Anthropic API poller ───────────────────────────────────────────────────

def _mins_until(unix_ts: float) -> int:
    now = datetime.now(timezone.utc).timestamp()
    delta = unix_ts - now
    return max(0, int(delta / 60))


async def poll_anthropic_api():
    while True:
        token = load_oauth_token()
        if not token:
            await asyncio.sleep(API_POLL_INTERVAL)
            continue

        headers = {
            "anthropic-version": "2023-06-01",
            "anthropic-beta":    "oauth-2025-04-20",
            "Content-Type":      "application/json",
            "User-Agent":        "claude-code/2.1.5",
            "Authorization":     f"Bearer {token}",
        }
        body = {
            "model": "claude-haiku-4-5-20251001",
            "max_tokens": 1,
            "messages": [{"role": "user", "content": "hi"}],
        }

        try:
            async with httpx.AsyncClient(timeout=20) as client:
                resp = await client.post(API_URL, headers=headers, json=body)
            h = resp.headers

            def pct_hdr(name: str) -> int:
                v = h.get(name)
                if v is None:
                    return 0
                try:
                    return min(100, max(0, round(float(v) * 100)))
                except ValueError:
                    return 0

            def reset_hdr(name: str) -> int:
                v = h.get(name)
                if v is None:
                    return -1
                try:
                    return _mins_until(float(v))
                except ValueError:
                    return -1

            with cc_data_lock:
                cc_data["s"]  = pct_hdr("anthropic-ratelimit-unified-5h-utilization")
                cc_data["sr"] = reset_hdr("anthropic-ratelimit-unified-5h-reset")
                cc_data["w"]  = pct_hdr("anthropic-ratelimit-unified-7d-utilization")
                cc_data["wr"] = reset_hdr("anthropic-ratelimit-unified-7d-reset")
                cc_data["st"] = h.get("anthropic-ratelimit-unified-5h-status", "unknown")
                cc_data["ok"] = True

            log.info("API poll: session=%s%% weekly=%s%%", cc_data["s"], cc_data["w"])

        except Exception as e:
            log.warning("API poll failed: %s", e)
            with cc_data_lock:
                cc_data["ok"] = False

        await asyncio.sleep(API_POLL_INTERVAL)


# ── Payload builder ────────────────────────────────────────────────────────

def build_payload() -> bytes:
    with cc_data_lock:
        cc = dict(cc_data)
    with ai_data_lock:
        ai = dict(ai_data)

    payload = {
        "s":     cc["s"],
        "sr":    cc["sr"],
        "w":     cc["w"],
        "wr":    cc["wr"],
        "st":    cc["st"],
        "ai_s":  ai["ai_session_pct"],
        "ai_sr": ai["ai_session_reset_mins"],
        "ai_w":  ai["ai_weekly_pct"],
        "ai_wr": ai["ai_weekly_reset_mins"],
        "ok":    cc["ok"],
    }
    return json.dumps(payload, separators=(",", ":")).encode()


# ── BLE layer ──────────────────────────────────────────────────────────────

async def find_device() -> str | None:
    # Try cached address first
    if BLE_ADDRESS_CACHE.exists():
        addr = BLE_ADDRESS_CACHE.read_text().strip()
        if addr:
            log.info("Trying cached BLE address: %s", addr)
            return addr

    log.info("Scanning for '%s'…", DEVICE_NAME)
    device = await BleakScanner.find_device_by_name(DEVICE_NAME, timeout=15)
    if device:
        log.info("Found device: %s (%s)", device.name, device.address)
        BLE_ADDRESS_CACHE.write_text(device.address)
        return device.address

    log.warning("Device '%s' not found in scan.", DEVICE_NAME)
    return None


async def ble_loop():
    refresh_requested = asyncio.Event()

    backoff = 5

    while True:
        addr = await find_device()
        if not addr:
            await asyncio.sleep(backoff)
            backoff = min(backoff * 2, 120)
            continue

        backoff = 5  # reset on successful find

        try:
            async with BleakClient(addr, timeout=20) as client:
                log.info("BLE connected to %s", addr)

                def on_req(_char, data: bytearray):
                    log.debug("Device requested refresh")
                    refresh_requested.set()

                try:
                    await client.start_notify(REQ_CHAR_UUID, on_req)
                except Exception:
                    pass  # REQ char may not exist on older firmware

                last_push = 0.0

                while client.is_connected:
                    now = time.monotonic()
                    if refresh_requested.is_set() or (now - last_push) >= API_POLL_INTERVAL:
                        refresh_requested.clear()
                        payload = build_payload()
                        if len(payload) > 512:
                            log.error("Payload too large: %d bytes", len(payload))
                        else:
                            await client.write_gatt_char(RX_CHAR_UUID, payload, response=False)
                            log.info("Pushed %d bytes to device", len(payload))
                        last_push = time.monotonic()
                    await asyncio.sleep(1)

        except Exception as e:
            log.warning("BLE disconnected: %s", e)
            # Clear cached address on connect failure so we re-scan
            BLE_ADDRESS_CACHE.unlink(missing_ok=True)

        log.info("BLE reconnecting in %ss…", backoff)
        await asyncio.sleep(backoff)
        backoff = min(backoff * 2, 60)


# ── Entry point ────────────────────────────────────────────────────────────

async def main():
    log.info("ClaudeOClock daemon starting…")

    # HTTP server on its own thread (blocking)
    http_thread = threading.Thread(target=start_http_server, daemon=True)
    http_thread.start()

    # Run API poller and BLE loop concurrently
    await asyncio.gather(
        poll_anthropic_api(),
        ble_loop(),
    )


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        log.info("Daemon stopped.")
