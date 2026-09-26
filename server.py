import json
import socket
from pathlib import Path
from starlette.applications import Starlette
from starlette.responses import HTMLResponse, JSONResponse, PlainTextResponse, FileResponse
from starlette.routing import Route, WebSocketRoute
from starlette.websockets import WebSocket, WebSocketDisconnect
import uvicorn

BASE_DIR = Path(__file__).resolve().parent
HTML_FILE = BASE_DIR / "index.html"
PORT = 5000

# Estado compartilhado em tempo real (0 = Desligado, 1, 2 ou 3)
estado_atual = 0
connected_clients = set()

def get_local_ip():
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        ip = s.getsockname()[0]
        s.close()
        return ip
    except Exception:
        return "192.168.31.236"

async def get_index(request):
    if HTML_FILE.exists():
        return HTMLResponse(HTML_FILE.read_text(encoding="utf-8"))
    return PlainTextResponse("Erro ao carregar página.", status_code=500)

async def get_estado(request):
    return JSONResponse({"brilho": estado_atual})

async def broadcast_estado():
    payload = json.dumps({"type": "SYNC_STATE", "brilho": estado_atual})
    disconnected = set()
    for client in connected_clients:
        try:
            await client.send_text(payload)
        except Exception:
            disconnected.add(client)
    connected_clients.difference_update(disconnected)

async def websocket_endpoint(websocket: WebSocket):
    global estado_atual
    await websocket.accept()
    connected_clients.add(websocket)
    try:
        # Envia estado atual imediatamente ao conectar (PC ou Celular)
        await websocket.send_text(json.dumps({"type": "SYNC_STATE", "brilho": estado_atual}))
        while True:
            data_text = await websocket.receive_text()
            try:
                data = json.loads(data_text)
                if data.get("type") == "SET_BRIGHTNESS":
                    novo_nivel = int(data.get("brilho", 0))
                    
                    # Lógica de Toggle / Intertravamento:
                    # Se já está ativo o mesmo, desliga (0). Senão, assume o novo nível.
                    if estado_atual == novo_nivel:
                        estado_atual = 0
                    else:
                        estado_atual = novo_nivel

                    print(f"[SYNC] Estado atualizado para: {estado_atual} (acionado por cliente)")
                    await broadcast_estado()
            except Exception as e:
                print(f"Erro ao processar mensagem: {e}")
    except (WebSocketDisconnect, Exception):
        pass
    finally:
        connected_clients.discard(websocket)

app = Starlette(
    routes=[
        Route("/", get_index, methods=["GET"]),
        Route("/index.html", get_index, methods=["GET"]),
        Route("/api/estado", get_estado, methods=["GET"]),
        WebSocketRoute("/", websocket_endpoint),
    ]
)

if __name__ == "__main__":
    local_ip = get_local_ip()
    print("=" * 55)
    print(" Smart Helipontos - Servidor Python Sincronizado")
    print("=" * 55)
    print(f"Servidor rodando em http://0.0.0.0:{PORT}")
    print(f"Acesse no PC:     http://localhost:{PORT}")
    print(f"Acesse no Celular: http://{local_ip}:{PORT}")
    print("=" * 55)
    
    uvicorn.run(app, host="0.0.0.0", port=PORT, log_level="info")
