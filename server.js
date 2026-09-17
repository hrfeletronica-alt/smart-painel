const http = require('http');
const fs = require('fs');
const path = require('path');
const WebSocket = require('ws');

const PORT = 5000;
const HTML_FILE = path.join(__dirname, 'index.html');

// Estado compartilhado em tempo real (0 = Desligado, 1, 2 ou 3)
let estadoAtual = 0;

// Servidor HTTP simples para servir a página index.html
const server = http.createServer((req, res) => {
  if (req.url === '/' || req.url === '/index.html') {
    fs.readFile(HTML_FILE, (err, data) => {
      if (err) {
        res.writeHead(500, { 'Content-Type': 'text/plain' });
        res.end('Erro ao carregar página.');
        return;
      }
      res.writeHead(200, { 'Content-Type': 'text/html; charset=utf-8' });
      res.end(data);
    });
  } else if (req.url === '/api/estado') {
    res.writeHead(200, { 'Content-Type': 'application/json' });
    res.end(JSON.stringify({ brilho: estadoAtual }));
  } else {
    res.writeHead(404, { 'Content-Type': 'text/plain' });
    res.end('Não encontrado');
  }
});

// Servidor WebSocket integrado na mesma porta (5000)
const wss = new WebSocket.Server({ server });

function broadcastEstado(sourceSocket) {
  const payload = JSON.stringify({ type: 'SYNC_STATE', brilho: estadoAtual });
  wss.clients.forEach((client) => {
    if (client.readyState === WebSocket.OPEN) {
      client.send(payload);
    }
  });
}

wss.on('connection', (ws) => {
  // Envia estado atual imediatamente ao conectar (PC ou Celular)
  ws.send(JSON.stringify({ type: 'SYNC_STATE', brilho: estadoAtual }));

  ws.on('message', (message) => {
    try {
      const data = JSON.parse(message);
      if (data.type === 'SET_BRIGHTNESS') {
        const novoNivel = parseInt(data.brilho, 10);
        
        // Lógica de Toggle / Intertravamento:
        // Se já está ativo o mesmo, desliga (0). Senão, assume o novo nível.
        if (estadoAtual === novoNivel) {
          estadoAtual = 0;
        } else {
          estadoAtual = novoNivel;
        }

        console.log(`[SYNC] Estado atualizado para: ${estadoAtual} (acionado por cliente)`);
        // Sincroniza PC, celular e todos os outros navegadores abertos
        broadcastEstado(ws);
      }
    } catch (e) {
      console.error('Erro ao processar mensagem:', e);
    }
  });
});

server.listen(PORT, '0.0.0.0', () => {
  console.log(`Servidor sincronizado rodando em http://0.0.0.0:${PORT}`);
  console.log(`Acesse no PC: http://localhost:${PORT}`);
  console.log(`Acesse no Celular: http://192.168.31.236:${PORT}`);
});
