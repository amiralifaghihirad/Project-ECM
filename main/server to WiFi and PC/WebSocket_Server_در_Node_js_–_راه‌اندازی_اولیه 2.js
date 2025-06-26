// Import the 'ws' library for WebSocket functionality
const WebSocket = require('ws');

// Create a new WebSocket server listening on port 8080
const server = new WebSocket.Server({ port: 8080 });

// Initialize an array to store connected clients
let clients = [];

// Event listener for when a new client connects to the server
server.on('connection', ws => {
    // Add the new client to the clients array
    clients.push(ws);
    
    // Log connection event to the console
    console.log('Client connected');
});
