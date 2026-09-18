# Smart Panel • ESP32 DevKit V1 (Smart Helipontos)

Sistema de automação profissional para controle e acionamento de balizamento aeronáutico de helipontos e iluminação auxiliar (FOOT LIGHT), com suporte a controle remoto via Nuvem MQTT (Web/Mobile 4G/5G) e botoeiras físicas locais com debounce e proteção contra ruído.

---

## 🚁 Funcionalidades do Sistema

- **Balizamento Aeronáutico (3 Níveis de Intensidade):**
  - **Brilho 1 (30%):** Aciona **Relé 1 (GPIO 18)**
  - **Brilho 2 (70%):** Aciona **Relé 2 (GPIO 19)**
  - **Brilho 3 (100%):** Aciona **Relé 3 (GPIO 21)**
  - **Intertravamento Automático:** Apenas 1 nível de brilho ativo por vez. Comutação instantânea e proteção de circuitos.
  - **Desligamento Rápido:** Clicar novamente no nível ativo desliga o balizamento.

- **FOOT LIGHT (Iluminação de Solo / Auxiliar):**
  - Aciona **Relé 4 (GPIO 22)**
  - **100% Independente:** Pode ser ligado e desligado livremente sem interferir no balizamento.

- **Botoeiras Físicas no Painel (Push Buttons):**
  - **Botão 1 (GPIO 32):** Alterna Brilho 1 (30%)
  - **Botão 2 (GPIO 33):** Alterna Brilho 2 (70%)
  - **Botão 3 (GPIO 27):** Alterna Brilho 3 (100%)
  - **Botão 4 (GPIO 14):** Alterna FOOT LIGHT
  - Conectados entre o pino GPIO e o **GND** (com pull-up interno e filtro contra ruído elétrico).

- **Inicialização Silenciosa (Zero Disparos no Boot):**
  - Os pinos de relé são configurados diretamente como DESLIGADOS no primeiro ciclo do `setup()`, sem pulsar nem bater os relés ao ligar ou reiniciar o ESP32.

- **Conectividade Nuvem MQTT (Acesso Global):**
  - Opera em tempo real através do broker MQTT (`broker.emqx.io:8084` via WSS na Web e porta `1883` no ESP32).
  - Controle de qualquer lugar do mundo (Wi-Fi, 4G, 5G) sem necessidade de IP fixo ou abertura de portas.
  - Suporte a múltiplos helipontos via parâmetro de URL (`?id=heliponto01`).

---

## 📌 Pinagem Oficial (ESP32 DevKit V1)

### Relés
| Circuito | Pino ESP32 | Entrada Relé | Função |
|---|---|---|---|
| **Brilho 1 (30%)** | **GPIO 18** | IN 1 | Balizamento Pista - 30% |
| **Brilho 2 (70%)** | **GPIO 19** | IN 2 | Balizamento Pista - 70% |
| **Brilho 3 (100%)** | **GPIO 21** | IN 3 | Balizamento Pista - 100% |
| **FOOT LIGHT** | **GPIO 22** | IN 4 | Iluminação Auxiliar de Solo |
| *Desabilitado* | GPIO 25 | IN 5 | Reserva Técnica |
| *Desabilitado* | GPIO 26 | IN 6 | Reserva Técnica |

### Botoeiras Físicas (Push Buttons)
| Botão | Pino ESP32 | Segundo Terminal | Ação |
|---|---|---|---|
| **Botão 1** | **GPIO 32** | GND | Liga / Desliga Brilho 1 (30%) |
| **Botão 2** | **GPIO 33** | GND | Liga / Desliga Brilho 2 (70%) |
| **Botão 3** | **GPIO 27** | GND | Liga / Desliga Brilho 3 (100%) |
| **Botão 4** | **GPIO 14** | GND | Liga / Desliga FOOT LIGHT |

---

## 📂 Arquivos do Repositório

- `index.html`: Interface Web moderna com suporte a QR Code para celular e conexão MQTT.
- `esp32_devkit_v1_mqtt/esp32_devkit_v1_mqtt.ino`: Firmware oficial completo para ESP32 DevKit V1.
- `Esquema_Ligacao_ESP32_DevKitV1_Reles.pdf`: Manual e esquema elétrico técnico pronto para impressão.
- `gerar_pdf.py`: Script Python para geração automática do PDF técnico.
- `teste_hardware_reles.ino`: Script simples para validação em bancada dos 4 relés.

---

## 🌐 Acesso Online

- **Painel Web:** [https://hrfeletronica-alt.github.io/smart-painel/](https://hrfeletronica-alt.github.io/smart-painel/)
- **Domínio Personalizado:** `painel.smarthelipontos.com.br`
