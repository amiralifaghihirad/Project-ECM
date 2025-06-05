const WebSocket = require('ws');
const server = new WebSocket.Server({ port: 8080 });

let clients = [];

server.on('connection', ws => {
    clients.push(ws);
    console.log('Client connected');