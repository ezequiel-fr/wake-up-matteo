// Init global URI
const uri = new URL(location.toString());

uri.pathname = '/';
uri.protocol = location.protocol === 'https:' ? 'wss:' : 'ws:';

const maxReconnectDelay = 20e3;

/** @type {WebSocket} */
let ws;
/** @type {number} */
let reconnectAttempt = 0;
/** @type {number | undefined} */
let reconnectTimer;
/** @type {number | undefined} */
let heartbeatTimer;

function getReconnectDelay() {
    const delay = Math.min(500 * 2 ** reconnectAttempt, maxReconnectDelay);
    reconnectAttempt += 1;

    return delay;
}

function clearHeartbeat() {
    clearInterval(heartbeatTimer);
    heartbeatTimer = undefined;
}

function startHeartbeat(socket) {
    clearHeartbeat();

    heartbeatTimer = setInterval(() => {
        if (socket.readyState === WebSocket.OPEN) socket.send('ping');
    }, 25000);
}

function scheduleReconnect() {
    if (reconnectTimer) return;

    const delay = getReconnectDelay();

    reconnectTimer = setTimeout(() => {
        reconnectTimer = undefined;
        createConnection();
    }, delay);
}

function handleMessage(data) {
    if (data === 'pong') return;

    try {
        const message = JSON.parse(data);
        if (message.type === 'dev:reload') location.reload();
    } catch (_err) {
        console.log(data);
    }
}

function createConnection() {
    ws = new WebSocket(uri);

    ws.onopen = function (ev) {
        reconnectAttempt = 0;

        ws.send('ping');
        startHeartbeat(ws);
    };

    ws.onmessage = ev => handleMessage(ev.data);

    ws.onclose = () => {
        clearHeartbeat();
        scheduleReconnect();
    };

    ws.onerror = () => {
        ws.close();
    };
}

(async () => {
    createConnection();
})();
