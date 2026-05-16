import { WebSocket, WebSocketServer } from 'ws';

let wsId = 0;

function broadcast(wss: WebSocketServer, data: string) {
    wss.clients.forEach(ws => {
        if (ws.readyState === WebSocket.OPEN) ws.send(data);
    });
}

function broadcastJson(wss: WebSocketServer, data: unknown) {
    broadcast(wss, JSON.stringify(data));
}

function handleWebsocketServer(wss: WebSocketServer) {
    wss.on('connection', ws => {
        // const id = ++wsId;
        ++wsId;
        let isAlive = true;

        ws.on('pong', () => isAlive = true);

        ws.on('message', message => {
            // Pong
            if (message.toString() === 'ping') return ws.send('pong');

            // Process any other message
            console.log(message);
        });

        const heartbeat = setInterval(() => {
            if (!isAlive) return ws.terminate();

            isAlive = false;
            ws.ping();
        }, 30e3);

        ws.on('close', () => clearInterval(heartbeat));
    });
}

export default handleWebsocketServer;
export { broadcast, broadcastJson };
