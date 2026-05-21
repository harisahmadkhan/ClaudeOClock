// ClaudeOClock content script — injected into https://claude.ai/* pages
// Reads usage data from the DOM and POSTs to the local daemon every 90 seconds.
//
// SELECTOR NOTES (last verified: May 2026):
// Claude.ai shows usage limits in the left sidebar / settings modal.
// The selectors below use a cascade — first match wins.
// When Anthropic updates their frontend, only this block needs updating.

const DAEMON_PORT = 47821;
const POLL_INTERVAL_MS = 90_000;

function extractUsage() {
    let session_pct = -1;
    let session_reset_mins = -1;
    let weekly_pct = -1;
    let weekly_reset_mins = -1;

    try {
        // Strategy 1: data-testid attributes (most stable)
        const usageBars = document.querySelectorAll('[data-testid*="usage"], [data-testid*="limit"]');
        for (const el of usageBars) {
            const text = el.textContent || '';
            const pctMatch = text.match(/(\d+)\s*%/);
            if (pctMatch && session_pct === -1) session_pct = parseInt(pctMatch[1], 10);
        }

        // Strategy 2: aria-label containing "usage" or "limit"
        if (session_pct === -1) {
            const ariaEls = document.querySelectorAll('[aria-label*="usage" i], [aria-label*="limit" i], [aria-label*="message" i]');
            for (const el of ariaEls) {
                const label = el.getAttribute('aria-label') || '';
                const pctMatch = label.match(/(\d+)\s*%/);
                if (pctMatch) { session_pct = parseInt(pctMatch[1], 10); break; }

                // "X messages left" style
                const msgMatch = label.match(/(\d+)\s*(?:messages?|tokens?)\s*(?:left|remaining)/i);
                if (msgMatch) {
                    // We have an absolute count, not a %. Store as -1 (can't convert without total).
                    break;
                }
            }
        }

        // Strategy 3: progress / meter elements
        if (session_pct === -1) {
            const progressEls = document.querySelectorAll('progress, [role="progressbar"], meter');
            for (const el of progressEls) {
                const value = parseFloat(el.getAttribute('value') || el.getAttribute('aria-valuenow') || '');
                const max = parseFloat(el.getAttribute('max') || el.getAttribute('aria-valuemax') || '100');
                if (!isNaN(value) && !isNaN(max) && max > 0) {
                    session_pct = Math.round((value / max) * 100);
                    break;
                }
            }
        }

        // Strategy 4: text content scan for "X messages left" or "resets in Xh"
        const allText = document.body?.innerText || '';

        if (session_pct === -1) {
            // Look for percentage in proximity to "limit" or "usage" text
            const pctNearUsage = allText.match(/(?:usage|limit|used)[^%\d]*(\d+)\s*%/i);
            if (pctNearUsage) session_pct = parseInt(pctNearUsage[1], 10);
        }

        // Extract reset time from text like "Resets in 2h 30m" or "Resets in 1d 2h"
        const resetMatch = allText.match(/[Rr]esets?\s+in\s+(?:(\d+)d\s*)?(?:(\d+)h\s*)?(?:(\d+)m)?/);
        if (resetMatch) {
            const days = parseInt(resetMatch[1] || '0', 10);
            const hours = parseInt(resetMatch[2] || '0', 10);
            const mins = parseInt(resetMatch[3] || '0', 10);
            session_reset_mins = days * 1440 + hours * 60 + mins;
        }

        // Weekly usage — look for second progress bar or "weekly" text
        const weeklyMatch = allText.match(/weekly[^%\d]*(\d+)\s*%/i);
        if (weeklyMatch) weekly_pct = parseInt(weeklyMatch[1], 10);

    } catch (e) {
        // Silent fail — DOM may not have usage info on this page
    }

    return {
        ai_session_pct: session_pct,
        ai_session_reset_mins: session_reset_mins,
        ai_weekly_pct: weekly_pct,
        ai_weekly_reset_mins: weekly_reset_mins,
        timestamp: Math.floor(Date.now() / 1000),
    };
}

function sendToDaemon(data) {
    fetch(`http://localhost:${DAEMON_PORT}/usage`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(data),
    })
    .then(r => {
        chrome.runtime.sendMessage({ type: 'DAEMON_STATUS', success: r.ok, data }).catch(() => {});
    })
    .catch(() => {
        chrome.runtime.sendMessage({ type: 'DAEMON_STATUS', success: false, data }).catch(() => {});
    });
}

// Send immediately and then on interval
sendToDaemon(extractUsage());
setInterval(() => sendToDaemon(extractUsage()), POLL_INTERVAL_MS);
