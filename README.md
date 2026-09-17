# Smart Panel • ESP32-S3 (Smart Helipontos)

Sistema de automação para balizamento de heliponto homologado, com controle de 3 níveis de intensidade luminosa e intertravamento automático de 6 relés (máximo 2 simultâneos).

## 🚀 Funcionalidades
- **3 Níveis de Brilho:**
  - **Brilho 1 (30%):** Aciona Relés 1 e 2 (GPIO 4 e 5)
  - **Brilho 2 (70%):** Aciona Relés 3 e 4 (GPIO 6 e 7)
  - **Brilho 3 (100%):** Aciona Relés 5 e 6 (GPIO 15 e 16)
- **Intertravamento Dinâmico (Anulação Mútua):** Ao ligar um nível, o anterior é desligado na mesma hora (nunca mais de 2 relés ligados).
- **Função Toggle:** Clicar no nível já ativo desliga o balizamento.
- **Sincronização em Tempo Real (WebSocket):** PC e Celular sincronizados instantaneamente.
- **Design Personalizado:** Identidade oficial da Smart Helipontos (logotipo, imagens e botões de contato).

## 📂 Arquivos do Projeto
- `index.html`: Interface web responsiva para PC e celular com QR Code.
- `server.js`: Servidor Node.js com WebSocket para sincronização em tempo real.
- `esp32_firmware.ino`: Firmware C++ para o microcontrolador ESP32-S3 (N16R8).
- `package.json`: Configurações e dependências.

## 🛠️ Como Executar Localmente
```bash
npm install
node server.js
```
Acesse no navegador:
- PC: `http://localhost:5000`
- Celular: `http://192.168.31.236:5000`
