import { createServer } from 'node:http';

import bodyParser from 'body-parser';
import cors from 'cors';
import express from 'express';
import { WebSocketServer } from 'ws';

import config from '@/constants';
import router from '@/router';
import handleWebSocketServer from '@/utils/websocket';

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

// Trigger start
server.listen(config.APP.PORT, () => {
    console.info(`Server running at http://localhost:${config.APP.PORT}`);

    // init DB, ...
});
