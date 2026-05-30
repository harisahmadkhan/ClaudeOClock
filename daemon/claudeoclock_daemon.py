"""
ClaudeOClock Daemon — Windows
Merges claude.ai data (from Chrome extension) with Claude Code API data
and pushes to the ESP32 over Wi-Fi every 60 seconds.

Dependencies: pip install httpx
"""

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

# ── Paths ──────────────────────────────────────────────────────────────────────

LOG_DIR    = Path.home() / ".config" / "claudeoclock"
TOKEN_PATH = LOG_DIR / "token.txt"
LOG_DIR.mkdir(parents=True, exist_ok=True)

# ── Logging ────────────────────────────────────────────────────────────────────

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    handlers=[
        logging.FileHandler(LOG_DIR / "daemon.log", encoding="utf-8"),
        logging.StreamHandler(sys.stdout),
    ],
)
log = logging.getLogger("claudeoclock")

# ── Constants ──────────────────────────────────────────────────────────────────

DEVICE_URL    = "http://claudeoclock.local/data"
PUSH_INTERVAL = 60  # seconds between Wi-Fi pushes

API_URL           = "https://api.anthropic.com/v1/messages"
API_POLL_INTERVAL = 60

CREDENTIAL_PATHS = [
    Path.home() / ".claude" / ".credentials.json",
    Path(os.environ.get("APPDATA", "")) / "Claude" / "claude_desktop_config.json",
    Path.home() / "AppData" / "Roaming" / "Claude" / ".credentials.json",
]

# ── Shared state ───────────────────────────────────────────────────────────────

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

# ── HTTP server (receives claude.ai data from extension) ───────────────────────

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
            log.debug("Extension: session=%s%% weekly=%s%%",
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


# ── OAuth token loader ─────────────────────────────────────────────────────────

def load_oauth_token() -> str | None:
    for path in CREDENTIAL_PATHS:
        if not path.exists():
            continue
        try:
            with open(path, encoding="utf-8") as f:
                obj = json.load(f)
            if "accessToken" in obj:
                return obj["accessToken"]
            nested = obj.get("claudeAiOauth", {})
            if "accessToken" in nested:
                return nested["accessToken"]
        except Exception as e:
            log.debug("Could not read %s: %s", path, e)
    log.warning("OAuth token not found in any credential path.")
    return None


# ── Device token loader ────────────────────────────────────────────────────────

def load_device_token() -> str | None:
    if TOKEN_PATH.exists():
        token = TOKEN_PATH.read_text().strip()
        if token:
            return token
    log.warning("Device token not found at %s — run install-windows.bat first.", TOKEN_PATH)
    return None


# ── Anthropic API poller ───────────────────────────────────────────────────────

def _mins_until(unix_ts: float) -> int:
    now = datetime.now(timezone.utc).timestamp()
    return max(0, int((unix_ts - now) / 60))


def poll_anthropic_api():
    while True:
        token = load_oauth_token()
        if not token:
            time.sleep(API_POLL_INTERVAL)
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
            with httpx.Client(timeout=20) as client:
                resp = client.post(API_URL, headers=headers, json=body)
            h = resp.headers

            def pct_hdr(name: str) -> int:
                v = h.get(name)
                try:
                    return min(100, max(0, round(float(v) * 100))) if v else 0
                except ValueError:
                    return 0

            def reset_hdr(name: str) -> int:
                v = h.get(name)
                try:
                    return _mins_until(float(v)) if v else -1
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

        time.sleep(API_POLL_INTERVAL)


# ── Payload builder ────────────────────────────────────────────────────────────

def build_payload() -> dict:
    with cc_data_lock:
        cc = dict(cc_data)
    with ai_data_lock:
        ai = dict(ai_data)
    return {
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


# ── Wi-Fi push loop ────────────────────────────────────────────────────────────

def push_to_device(payload: dict, token: str):
    try:
        r = httpx.post(
            DEVICE_URL,
            json=payload,
            headers={"X-ClaudeOclock-Token": token},
            timeout=5.0,
        )
        if r.status_code == 200:
            log.info("[push] OK — session=%s%% weekly=%s%%",
                     payload["s"], payload["w"])
        elif r.status_code == 401:
            log.warning("[push] Unauthorized — token in token.txt doesn't match device")
        else:
            log.warning("[push] Device returned %s", r.status_code)
    except httpx.ConnectError:
        log.warning("[push] Device not found — is ClaudeOclock on the same WiFi?")
    except httpx.TimeoutException:
        log.warning("[push] Timeout — device may be sleeping")
    except Exception as e:
        log.warning("[push] Error: %s", e)


def wifi_push_loop():
    while True:
        device_token = load_device_token()
        if device_token:
            payload = build_payload()
            push_to_device(payload, device_token)
        time.sleep(PUSH_INTERVAL)


# ── Entry point ────────────────────────────────────────────────────────────────

def main():
    log.info("ClaudeOClock daemon starting…")

    # HTTP server on its own thread (blocking, localhost only)
    threading.Thread(target=start_http_server, daemon=True).start()

    # API poller on its own thread
    threading.Thread(target=poll_anthropic_api, daemon=True).start()

    # Wi-Fi push loop — runs in main thread
    wifi_push_loop()


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        log.info("Daemon stopped.")
