/*
 * ESP32-S3 (N16R8) - Controlador de Balizamento Smart Helipontos via Nuvem MQTT
 * - Funciona de QUALQUER REDE (Wi-Fi, 4G, 5G, outra cidade ou país)
 * - Não precisa abrir portas no roteador (sem port forwarding, sem IP fixo)
 * - Mapeamento:
 *    - Brilho 1 (30%): Relés 1 e 2 (GPIO 4 e 5)
 *    - Brilho 2 (70%): Relés 3 e 4 (GPIO 6 e 7)
 *    - Brilho 3 (100%): Relés 5 e 6 (GPIO 15 e 16)
 * - Intertravamento estrito: Máximo 2 relés ligados por vez
 */

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h> // Instale a biblioteca "PubSubClient" pela Arduino IDE

// 1. CONFIGURAÇÃO DA REDE WI-FI DO HELIPONTO
const char* WIFI_SSID     = "SEU_WIFI_NOME";
const char* WIFI_PASSWORD = "SUA_SENHA_WIFI";

// 2. CONFIGURAÇÃO DO BROKER MQTT EM NUVEM (Público e Gratuito)
const char* MQTT_BROKER   = "broker.emqx.io";
const int   MQTT_PORT     = 1883;
const char* TOPICO_COMANDO = "smarthelipontos/balizamento/comando";
const char* TOPICO_STATUS  = "smarthelipontos/balizamento/status";

// 3. PINAGEM DOS 6 RELÉS NO ESP32-S3
const uint8_t NUM_RELES = 6;
const uint8_t PINOS_RELE[NUM_RELES] = {4, 5, 6, 7, 15, 16};

// Lógica Active LOW (LOW liga, HIGH desliga na maioria dos módulos)
#define RELE_LIGADO    LOW
#define RELE_DESLIGADO HIGH

// Mapeamento dos 3 grupos
const uint8_t GRUPOS[3][2] = {
  {0, 1}, // Brilho 1: Relé 1 (GPIO 4) + Relé 2 (GPIO 5)
  {2, 3}, // Brilho 2: Relé 3 (GPIO 6) + Relé 4 (GPIO 7)
  {4, 5}  // Brilho 3: Relé 5 (GPIO 15) + Relé 6 (GPIO 16)
};

int brilhoAtual = 0; // 0 = Desligado, 1 = 30%, 2 = 70%, 3 = 100%

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void desligarTodosReles() {
  for (int i = 0; i < NUM_RELES; i++) {
    digitalWrite(PINOS_RELE[i], RELE_DESLIGADO);
  }
}

void aplicarNivelBrilho(int nivel) {
  if (nivel < 1 || nivel > 3) {
    desligarTodosReles();
    brilhoAtual = 0;
    Serial.println("[MQTT] Balizamento DESLIGADO.");
  } else {
    // Intertravamento: desliga todos antes de acionar o novo estágio
    desligarTodosReles();

    uint8_t r1 = GRUPOS[nivel - 1][0];
    uint8_t r2 = GRUPOS[nivel - 1][1];
    digitalWrite(PINOS_RELE[r1], RELE_LIGADO);
    digitalWrite(PINOS_RELE[r2], RELE_LIGADO);

    brilhoAtual = nivel;
    Serial.printf("[MQTT] Brilho %d ATIVADO (Reles %d e %d ligados)\n", nivel, r1 + 1, r2 + 1);
  }

  // Notifica o status atualizado para todas as telas (celular e PC)
  char payload[4];
  sprintf(payload, "%d", brilhoAtual);
  mqttClient.publish(TOPICO_STATUS, payload, true);
}

// Callback executado quando o ESP32 recebe comando do celular ou PC
void callbackMQTT(char* topic, byte* message, unsigned int length) {
  String msg = "";
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)message[i];
  }

  Serial.printf("[RECEBIDO NUVEM] Topico: %s | Comando: %s\n", topic, msg.c_str());

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
  while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi Conectado com sucesso!");
    Serial.print("IP na rede local: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFalha na conexao Wi-Fi.");
  }
}

void reconectarMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Conectando ao Broker MQTT na Nuvem (broker.emqx.io)... ");
    String clientId = "ESP32S3-Heliponto-" + String(random(0xffff), HEX);

    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("CONECTADO A NUVEM COM SUCESSO!");
      // Assina o tópico de comando
      mqttClient.subscribe(TOPICO_COMANDO);
      // Publica status atual
      char payload[4];
      sprintf(payload, "%d", brilhoAtual);
      mqttClient.publish(TOPICO_STATUS, payload, true);
    } else {
      Serial.print("Falha, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Tentando em 3 segundos...");
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Configura os 6 relés e inicia todos desligados
  for (int i = 0; i < NUM_RELES; i++) {
    pinMode(PINOS_RELE[i], OUTPUT);
    digitalWrite(PINOS_RELE[i], RELE_DESLIGADO);
  }

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
