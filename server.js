const net = require('net');
const fs = require('fs');
const port = 1337;
const host = '192.168.0.196';

const server = net.createServer(); // creer le serveur

server.listen(port, host, () => { // démarer l'écoute sur le port
    console.log('TCP Server is running on port ' + port + '.');
});

server.on('connection', function(sock) { // lorsqu'il y a une connexion
    console.log('CONNECTED: ' + sock.remoteAddress + ':' + sock.remotePort);

    // recevoir les data
    sock.on('data', function(json){
        console.log('Received: ' + json);
	    sock.destroy();
    })

    sock.on('close', function() {
        console.log('Connection closed');
    });
});