import { createServer } from 'node:http';
import { watch } from 'node:fs';

import bodyParser from 'body-parser';
import cors from 'cors';
import express from 'express';
import { WebSocketServer } from 'ws';

import config from '@/constants';
import router from '@/routes';
import handleWebSocketServer, { broadcastJson } from '@/utils/websocket';

// Complete imports
import 'colors';

const app = express();

// App setup
app.use(bodyParser.json({
    limit: "300mb",
    verify: (req: any, _res, buf) => { req.rawBody = buf },
}));
app.use(bodyParser.urlencoded({ extended: true, limit: "300mb" }));

app.use(cors({
    credentials: true,
    origin: (_origin, cb) => cb(null, true),
}));

app.set("trust proxy", true);

// Routing setup
app.use(router);

// Create a new HTTP server and start it
const server = createServer(app);

// WSS setup
const wss = new WebSocketServer({ autoPong: true, server });
handleWebSocketServer(wss);

// Watch public dir for changes and trigger dev reload
if (config.__DEV__) {
    let reloadTimer: NodeJS.Timeout | undefined;

    watch(config.APP.PUBLIC_DIR, { recursive: true }, (_eventType, filename) => {
        if (!filename) return;
        clearTimeout(reloadTimer);

        reloadTimer = setTimeout(() => {
            const path = filename.toString().split('\\').join('/');
            broadcastJson(wss, { type: 'dev:reload', path });
        }, 100);
    });
}

// Trigger start
server.listen(config.APP.PORT, () => {
    console.info(`Server running at http://localhost:${config.APP.PORT}`);

    // init DB, ...
});
