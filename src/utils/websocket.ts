import { WebSocketServer } from 'ws';

let wsId = 0;

function handleWebsocketServer(wss: WebSocketServer) {
    wss.on('connection', ws => {
        const id = ++wsId;
        console.info(`WebSocket client connected: ${id}`);

        ws.on('close', () => {
            console.info(`WebSocket client disconnected: ${id}`);
        });

        ws.on('message', message => {
            // Pong
            if (message.toString() === 'ping') return ws.send('pong');

            // Process any other message
            console.log(message);
        });
    });
}

export default handleWebsocketServer;
