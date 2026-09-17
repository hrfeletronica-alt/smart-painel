/*
 * ============================================================================
 * PROJETO: SMART HELIPONTOS - CONTROLE DE BALIZAMENTO NOTURNO
 * HARDWARE: ESP32 DevKit V1 (ESP-WROOM-32 de 30 ou 38 pinos)
 * RECURSOS:
 *  1. Controle via Nuvem MQTT (App Web / Celular 4G e PC)
 *  2. Controle Físico Local com 3 Push Buttons (Botoeiras no Painel)
 * ============================================================================
 * 
 * MAPA DE PINOS SEGUROS NO ESP32 DevKit V1:
 * 
 * [SAÍDAS PARA OS 6 RELÉS]
 *   - Estágio 1 (30%):  Relé 1 (GPIO 18) e Relé 2 (GPIO 19)
 *   - Estágio 2 (70%):  Relé 3 (GPIO 21) e Relé 4 (GPIO 22)
 *   - Estágio 3 (100%): Relé 5 (GPIO 25) e Relé 6 (GPIO 26)
 * 
 * [ENTRADAS PARA OS 3 PUSH BUTTONS (BOTOEIRAS)]
 *   - Usando PULL-UP interno (Ligue o botão entre o pino do ESP32 e o GND):
 *   - Botão Físico 1 (Brilho 1): GPIO 32 ➔ GND
 *   - Botão Físico 2 (Brilho 2): GPIO 33 ➔ GND
 *   - Botão Físico 3 (Brilho 3): GPIO 27 ➔ GND
 * 
 * REGRA DO SISTEMA:
 * - O botão físico e o aplicativo trabalham em conjunto e sincronizados.
 * - Pressionar um botão físico comuta o estágio e avisa o celular na mesma hora via MQTT.
 * - Intertravamento rígido: Nunca mais de 2 relés ligados simultaneamente.
 * - Clicar no mesmo botão desliga o balizamento (Toggle).
 */

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h> // Instale a biblioteca "PubSubClient" pela Arduino IDE

// ============================================================================
// 1. CONFIGURAÇÃO DE WI-FI DO HELIPONTO
// ============================================================================
const char* WIFI_SSID     = "SEU_WIFI_NOME";
const char* WIFI_PASSWORD = "SUA_SENHA_WIFI";

// ============================================================================
// 2. CONFIGURAÇÃO DO HELIPONTO E BROKER MQTT
// ============================================================================
// Altere este ID para cada cliente/heliponto instalado (ex: "tower-sp", "fazenda-sol", "padrao")
const char* HELIPONTO_ID   = "padrao"; 

const char* MQTT_BROKER    = "broker.emqx.io";
const int   MQTT_PORT      = 1883;

// Tópicos dinâmicos isolados por cliente:
String TOPICO_COMANDO      = "smarthelipontos/" + String(HELIPONTO_ID) + "/balizamento/comando";
String TOPICO_STATUS       = "smarthelipontos/" + String(HELIPONTO_ID) + "/balizamento/status";

// ============================================================================
// 3. PINAGEM DOS RELÉS E DOS 3 BOTÕES FÍSICOS (ESP32 DevKit V1)
// ============================================================================
// Relés 1, 2 e 3 ATIVOS:
//   - Brilho 1 (30%):  Relé 1 (GPIO 18)
//   - Brilho 2 (70%):  Relé 2 (GPIO 19)
//   - Brilho 3 (100%): Relé 3 (GPIO 21)
// Relés 4, 5 e 6 DESABILITADOS (GPIOs 22, 25, 26 mantidos desligados)
const uint8_t NUM_RELES = 6;
const uint8_t PINOS_RELE[NUM_RELES] = {18, 19, 21, 22, 25, 26};

// Mapeamento de 1 relé por estágio:
const uint8_t RELE_ESTAGIO[3] = {0, 1, 2}; // Estágio 1 -> Relé 1, Estágio 2 -> Relé 2, Estágio 3 -> Relé 3

// Pinos dos 3 Push Buttons
const uint8_t NUM_BOTOES = 3;
const uint8_t PINOS_BOTAO[NUM_BOTOES] = {32, 33, 27};

// Lógica de relé: Active LOW (LOW liga, HIGH desliga)
#define RELE_LIGADO    LOW
#define RELE_DESLIGADO HIGH

int brilhoAtual = 0; // 0 = Desligado, 1 = 30%, 2 = 70%, 3 = 100%

// Variáveis para Debounce dos botões físicos
bool ultimoEstadoBotao[NUM_BOTOES] = {HIGH, HIGH, HIGH};
unsigned long ultimoTempoDebounce[NUM_BOTOES] = {0, 0, 0};
const unsigned long DELAY_DEBOUNCE = 50; // 50 milissegundos para filtrar ruído mecânico

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Desliga todos os relés imediatamente
void desligarTodosReles() {
  for (int i = 0; i < NUM_RELES; i++) {
    digitalWrite(PINOS_RELE[i], RELE_DESLIGADO);
  }
}

// Aplica o nível de brilho com intertravamento (1 relé ativo por vez)
void aplicarNivelBrilho(int nivel) {
  if (nivel < 1 || nivel > 3) {
    desligarTodosReles();
    brilhoAtual = 0;
    Serial.println("[STATUS] Balizamento DESLIGADO (Todos os reles off).");
  } else {
    // Intertravamento: desliga todos antes de ligar o novo estágio
    desligarTodosReles();

    // Aciona apenas o relé correspondente ao nível (Relés 4, 5 e 6 permanecem desabilitados)
    uint8_t releIndice = RELE_ESTAGIO[nivel - 1];
    digitalWrite(PINOS_RELE[releIndice], RELE_LIGADO);

    brilhoAtual = nivel;
    Serial.printf("[STATUS] Estágio %d ATIVADO! Apenas Relé %d (GPIO %d) LIGADO. (Relés 4, 5 e 6 desabilitados)\n",
                  nivel, releIndice + 1, PINOS_RELE[releIndice]);
  }

  // Notifica o novo estado para a nuvem MQTT (atualiza celular e PC na mesma hora)
  if (mqttClient.connected()) {
    char payload[4];
    sprintf(payload, "%d", brilhoAtual);
    mqttClient.publish(TOPICO_STATUS.c_str(), payload, true);
  }
}

// Alterna o brilho (se já estiver ativo, desliga)
void alternarBrilho(int nivel) {
  if (brilhoAtual == nivel) {
    aplicarNivelBrilho(0); // Desliga (Toggle)
  } else {
    aplicarNivelBrilho(nivel);
  }
}

// Trata os comandos recebidos pela internet (MQTT)
void callbackMQTT(char* topic, byte* message, unsigned int length) {
  String msg = "";
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)message[i];
  }

  Serial.printf("[COMANDO NUVEM] Tópico: %s | Mensagem: %s\n", topic, msg.c_str());

  if (String(topic) == TOPICO_COMANDO) {
    int nivel = msg.toInt();
    aplicarNivelBrilho(nivel);
  }
}

// Leitura contínua dos 3 botões físicos no painel com debounce
void verificarBotoesFisicos() {
  for (int i = 0; i < NUM_BOTOES; i++) {
    int leitura = digitalRead(PINOS_BOTAO[i]);

    // Detecta transição de solto (HIGH) para pressionado (LOW)
    if (leitura != ultimoEstadoBotao[i]) {
      ultimoTempoDebounce[i] = millis();
    }

    if ((millis() - ultimoTempoDebounce[i]) > DELAY_DEBOUNCE) {
      // Se o botão está realmente pressionado (LOW com PULL-UP)
      static bool estadoProcessado[NUM_BOTOES] = {false, false, false};

      if (leitura == LOW && !estadoProcessado[i]) {
        estadoProcessado[i] = true;
        Serial.printf("\n[BOTÃO FÍSICO %d PRESSIONADO (GPIO %d)]\n", i + 1, PINOS_BOTAO[i]);
        alternarBrilho(i + 1); // 1 = Brilho 1, 2 = Brilho 2, 3 = Brilho 3
      } else if (leitura == HIGH) {
        estadoProcessado[i] = false;
      }
    }

    ultimoEstadoBotao[i] = leitura;
  }
}

void reconectarWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.print("Conectando ao Wi-Fi: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 25) {
    delay(400);
    Serial.print(".");
    tentativas++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[Wi-Fi] Conectado com sucesso!");
    Serial.print("[IP Local]: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n[Wi-Fi] Falha ao conectar.");
  }
}

void reconectarMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Conectando ao Broker MQTT Nuvem (broker.emqx.io)... ");
    String clientId = "DevKitV1-Heliponto-" + String(random(0xffff), HEX);

    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("CONECTADO A NUVEM!");
      mqttClient.subscribe(TOPICO_COMANDO.c_str());
      char payload[4];
      sprintf(payload, "%d", brilhoAtual);
      mqttClient.publish(TOPICO_STATUS.c_str(), payload, true);
    } else {
      Serial.printf("Falha (rc=%d). Tentando em 3s...\n", mqttClient.state());
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // 1. Inicializa os pinos de relé (Saídas)
  for (int i = 0; i < NUM_RELES; i++) {
    pinMode(PINOS_RELE[i], OUTPUT);
    digitalWrite(PINOS_RELE[i], RELE_DESLIGADO);
  }

  // 2. Inicializa os 3 Push Buttons com PULL-UP interno (Entradas)
  for (int i = 0; i < NUM_BOTOES; i++) {
    pinMode(PINOS_BOTAO[i], INPUT_PULLUP);
  }

  Serial.println("\n==============================================");
  Serial.println("   SMART HELIPONTOS - ESP32 DevKit V1");
  Serial.println("   Relés nos GPIOs 18, 19, 21, 22, 25, 26");
  Serial.println("   Botoeiras nos GPIOs 32, 33, 27 (GND)");
  Serial.println("==============================================");

  reconectarWiFi();

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(callbackMQTT);
}

void loop() {
  // 1. Monitora os botões físicos continuamente com debounce
  verificarBotoesFisicos();

  // 2. Mantém a conexão Wi-Fi e MQTT ativa
  if (WiFi.status() != WL_CONNECTED) {
    reconectarWiFi();
  }

  if (!mqttClient.connected()) {
    reconectarMQTT();
  }

  mqttClient.loop();
}
