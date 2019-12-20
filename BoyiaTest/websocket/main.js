const WebSocket = require('ws');

function initWebSocketServer() {
  const server = new WebSocket.Server(
    {
      host: '192.168.0.10',
      port: 6666
    }
  );

  server.on('listening', socket => {
    console.log('Test WebSocket listening address=' + JSON.stringify(server.address()) + ' port=' + server.address().port);
  });

  server.on('connection', (socket, req) => {
    console.log('Test WebSocket connection url=' + req.url);
    socket.send('hello world');

    socket.on('message', event => {
      console.log('Test WebSocket message data=' + event.toString());
    });
  });


}

initWebSocketServer();