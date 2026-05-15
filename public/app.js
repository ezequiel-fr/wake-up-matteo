// Init global URI
const uri = new URL(location.toString());

uri.pathname = '/';
uri.protocol = 'ws';

function createConnection() {
    const ws = new WebSocket(uri);

    ws.onopen = function (ev) {
        ev.preventDefault();
        ws.send('ping');
    };

    ws.onmessage = ev => console.log(ev.data);
}

(async () => {
    createConnection();
})();
