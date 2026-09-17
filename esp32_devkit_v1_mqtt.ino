/*
 * ============================================================================
 * PROJETO: SMART HELIPONTOS - CONTROLE DE BALIZAMENTO NOTURNO
 * HARDWARE: ESP32 DevKit V1 (ESP-WROOM-32 de 30 ou 38 pinos)
 * COMUNICAÇÃO: Nuvem MQTT Global (Acionamento de qualquer rede / 4G / Wi-Fi)
 * ============================================================================
 * 
 * MAPA DE PINOS SEGUROS NO ESP32 DevKit V1:
 * - Pinos livres de strapping e sem pulsos no boot (evita disparo indesejado dos relés)
 * 
 *   [ESTÁGIO 1 - 30%]
 *   - Relé 1: GPIO 18
 *   - Relé 2: GPIO 19
 * 
 *   [ESTÁGIO 2 - 70%]
 *   - Relé 3: GPIO 21
 *   - Relé 4: GPIO 22
 * 
 *   [ESTÁGIO 3 - 100%]
 *   - Relé 5: GPIO 25
 *   - Relé 6: GPIO 26
 * 
 * REGRA DO SISTEMA:
 * - Cada estágio aciona estritamente 2 relés simultâneos.
 * - Ao acionar um estágio, os outros são imediatamente ANULADOS (máximo 2 relés ativos).
 * - Clicar no mesmo estágio desliga o balizamento.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h> // Instale a biblioteca "PubSubClient" de Nick O'Leary na Arduino IDE

// ============================================================================
// 1. CONFIGURAÇÃO DE WI-FI DO HELIPONTO
// ============================================================================
const char* WIFI_SSID     = "SEU_WIFI_NOME";
const char* WIFI_PASSWORD = "SUA_SENHA_WIFI";

// ============================================================================
// 2. CONFIGURAÇÃO DO BROKER MQTT EM NUVEM (EMQX Broker Público / Seguro)
// ============================================================================
const char* MQTT_BROKER    = "broker.emqx.io";
const int   MQTT_PORT      = 1883;
const char* TOPICO_COMANDO = "smarthelipontos/balizamento/comando";
const char* TOPICO_STATUS  = "smarthelipontos/balizamento/status";

// ============================================================================
// 3. DEFINIÇÃO DOS PINOS DO ESP32 DEVKIT V1
// ============================================================================
const uint8_t NUM_RELES = 6;
const uint8_t PINOS_RELE[NUM_RELES] = {18, 19, 21, 22, 25, 26};

// A maioria dos módulos de relé comerciais trabalha em Lógica Invertida (Active LOW)
// Se os seus relés forem Active HIGH, inverta as definições abaixo:
#define RELE_LIGADO    LOW
#define RELE_DESLIGADO HIGH

// Agrupamento dos 3 estágios de brilho (2 relés por estágio)
const uint8_t ESTAGIOS[3][2] = {
  {0, 1}, // Estágio 1 (30%):  Relés 1 (GPIO 18) e 2 (GPIO 19)
  {2, 3}, // Estágio 2 (70%):  Relés 3 (GPIO 21) e 4 (GPIO 22)
  {4, 5}  // Estágio 3 (100%): Relés 5 (GPIO 25) e 6 (GPIO 26)
};

int brilhoAtual = 0; // 0 = Desligado, 1 = 30%, 2 = 70%, 3 = 100%

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Função que garante o desligamento físico de todos os 6 relés
void desligarTodosReles() {
  for (int i = 0; i < NUM_RELES; i++) {
    digitalWrite(PINOS_RELE[i], RELE_DESLIGADO);
  }
}

// Lógica estrita de comutação com intertravamento
void aplicarNivelBrilho(int nivel) {
  if (nivel < 1 || nivel > 3) {
    desligarTodosReles();
    brilhoAtual = 0;
    Serial.println("[ESP32 DevKit V1] Balizamento DESLIGADO.");
  } else {
    // 1º Garante desligar todos os outros antes de ligar o novo (sem sobreposição)
    desligarTodosReles();

    // 2º Liga apenas os 2 relés correspondentes ao estágio selecionado
    uint8_t r1 = ESTAGIOS[nivel - 1][0];
    uint8_t r2 = ESTAGIOS[nivel - 1][1];
    digitalWrite(PINOS_RELE[r1], RELE_LIGADO);
    digitalWrite(PINOS_RELE[r2], RELE_LIGADO);

    brilhoAtual = nivel;
    Serial.printf("[ESP32 DevKit V1] Estágio %d ATIVADO! Relé %d (GPIO %d) e Relé %d (GPIO %d) LIGADOS.\n",
                  nivel, r1 + 1, PINOS_RELE[r1], r2 + 1, PINOS_RELE[r2]);
  }

  // Notifica o novo estado para a nuvem (para sincronizar celulares e PCs)
  char payload[4];
  sprintf(payload, "%d", brilhoAtual);
  mqttClient.publish(TOPICO_STATUS, payload, true);
}

// Callback acionado instantaneamente quando chega comando do painel web
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
    Serial.println("\n[Wi-Fi] Falha ao conectar. Verifique nome e senha.");
  }
}

void reconectarMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Conectando ao Broker MQTT Nuvem (broker.emqx.io)... ");
    String clientId = "DevKitV1-Heliponto-" + String(random(0xffff), HEX);

    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("CONECTADO A NUVEM!");
      // Assina o tópico para escutar os comandos do celular e do PC
      mqttClient.subscribe(TOPICO_COMANDO);
      // Publica o estado atual
      char payload[4];
      sprintf(payload, "%d", brilhoAtual);
      mqttClient.publish(TOPICO_STATUS, payload, true);
    } else {
      Serial.printf("Falha (código rc=%d). Tentando novamente em 3s...\n", mqttClient.state());
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Inicializa os pinos de relé garantindo que iniciem todos DESLIGADOS
  for (int i = 0; i < NUM_RELES; i++) {
    pinMode(PINOS_RELE[i], OUTPUT);
    digitalWrite(PINOS_RELE[i], RELE_DESLIGADO);
  }

  Serial.println("\n==============================================");
  Serial.println("   SMART HELIPONTOS - ESP32 DevKit V1");
  Serial.println("==============================================");

  reconectarWiFi();

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(callbackMQTT);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    reconectarWiFi();
  }

  if (!mqttClient.connected()) {
    reconectarMQTT();
  }

  mqttClient.loop();
}
