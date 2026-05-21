// ClaudeOClock background service worker
// Tracks daemon connection status for the popup display.

let lastContact = null;
let lastData = null;

chrome.runtime.onMessage.addListener((msg, _sender, respond) => {
    if (msg.type === 'DAEMON_STATUS') {
        if (msg.success) lastContact = Date.now();
        if (msg.data) lastData = msg.data;
        respond({ lastContact, lastData });
        return true;
    }
    if (msg.type === 'GET_STATUS') {
        respond({ lastContact, lastData });
        return true;
    }
});
