/*
 * ============================================================================
 * PROJETO: SMART HELIPONTOS - CONTROLE DE BALIZAMENTO NOTURNO E FOOT LIGHT
 * HARDWARE: ESP32 DevKit V1 (ESP-WROOM-32 de 30 ou 38 pinos)
 * RECURSOS:
 *  1. Controle via Nuvem MQTT (App Web / Celular 4G e PC)
 *  2. Controle Físico Local com 4 Push Buttons (Botoeiras no Painel)
 * ============================================================================
 * 
 * MAPA DE PINOS SEGUROS NO ESP32 DevKit V1:
 * 
 * [SAÍDAS PARA OS RELÉS]
 *   - Brilho 1 (30%):  Relé 1 (GPIO 18)
 *   - Brilho 2 (70%):  Relé 2 (GPIO 19)
 *   - Brilho 3 (100%): Relé 3 (GPIO 21)
 *   - FOOT LIGHT:      Relé 4 (GPIO 22) -> Circuito Independente de Solo
 *   - Desabilitados:   Relé 5 (GPIO 25) e Relé 6 (GPIO 26) mantidos desligados
 * 
 * [ENTRADAS PARA OS 4 PUSH BUTTONS (BOTOEIRAS)]
 *   - Usando PULL-UP interno (Ligue o botão entre o pino do ESP32 e o GND):
 *   - Botão Físico 1 (Brilho 1):   GPIO 32 ➔ GND
 *   - Botão Físico 2 (Brilho 2):   GPIO 33 ➔ GND
 *   - Botão Físico 3 (Brilho 3):   GPIO 27 ➔ GND
 *   - Botão Físico 4 (FOOT LIGHT): GPIO 14 ➔ GND
 * 
 * REGRA DO SISTEMA:
 * - O botão físico e o aplicativo trabalham em conjunto e sincronizados.
 * - Pressionar um botão físico comuta o estado e avisa o celular na mesma hora via MQTT.
 * - Intertravamento rígido no balizamento: Apenas 1 nível de brilho ativo por vez.
 * - Clicar no mesmo botão desliga o estágio (Toggle).
 * - O FOOT LIGHT opera de forma independente do balizamento.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h> // Instale a biblioteca "PubSubClient" pela Arduino IDE

// ============================================================================
// 1. CONFIGURAÇÃO DE WI-FI DO HELIPONTO
// ============================================================================
const char* WIFI_SSID     = "Xiaomi_346D";
const char* WIFI_PASSWORD = "2SD3773d65";

// ============================================================================
// 2. CONFIGURAÇÃO DO HELIPONTO E BROKER MQTT
// ============================================================================
// Altere este ID para cada cliente/heliponto instalado (ex: "tower-sp", "fazenda-sol", "padrao")
const char* HELIPONTO_ID   = "padrao"; 

const char* MQTT_BROKER    = "broker.emqx.io";
const int   MQTT_PORT      = 1883;

// Tópicos dinâmicos isolados por cliente:
String TOPICO_COMANDO          = "smarthelipontos/" + String(HELIPONTO_ID) + "/balizamento/comando";
String TOPICO_STATUS           = "smarthelipontos/" + String(HELIPONTO_ID) + "/balizamento/status";
String TOPICO_FOOTLIGHT_CMD    = "smarthelipontos/" + String(HELIPONTO_ID) + "/footlight/comando";
String TOPICO_FOOTLIGHT_STATUS = "smarthelipontos/" + String(HELIPONTO_ID) + "/footlight/status";

// ============================================================================
// 3. PINAGEM DOS RELÉS E BOTÕES FÍSICOS (ESP32 DevKit V1)
// ============================================================================
// Relés:
//   - Brilho 1 (30%):  Relé 1 (GPIO 18)
//   - Brilho 2 (70%):  Relé 2 (GPIO 19)
//   - Brilho 3 (100%): Relé 3 (GPIO 21)
//   - FOOT LIGHT:      Relé 4 (GPIO 22) <--- NOVO
//   - Desabilitados:   Relé 5 (GPIO 25) e Relé 6 (GPIO 26)
const uint8_t NUM_RELES = 6;
const uint8_t PINOS_RELE[NUM_RELES] = {18, 19, 21, 22, 25, 26};

// Mapeamento de 1 relé por estágio de brilho:
const uint8_t RELE_ESTAGIO[3] = {0, 1, 2}; // Estágio 1 -> Relé 1, Estágio 2 -> Relé 2, Estágio 3 -> Relé 3
const uint8_t PINO_RELE_FOOTLIGHT = 22;    // Relé 4 (GPIO 22) para FOOT LIGHT

// Pinos dos Push Buttons Físicos:
//   - Botão 1: GPIO 32 -> Brilho 1
//   - Botão 2: GPIO 33 -> Brilho 2
//   - Botão 3: GPIO 27 -> Brilho 3
//   - Botão 4: GPIO 14 -> FOOT LIGHT (opcional no painel)
const uint8_t NUM_BOTOES = 4;
const uint8_t PINOS_BOTAO[NUM_BOTOES] = {32, 33, 27, 14};

// Lógica de relé: Active LOW (LOW liga, HIGH desliga)
#define RELE_LIGADO    LOW
#define RELE_DESLIGADO HIGH

int brilhoAtual = 0;       // 0 = Desligado, 1 = 30%, 2 = 70%, 3 = 100%
bool footLightAtivo = false; // Estado do FOOT LIGHT

// Variáveis para Debounce dos botões físicos
bool ultimoEstadoBotao[NUM_BOTOES] = {HIGH, HIGH, HIGH, HIGH};
unsigned long ultimoTempoDebounce[NUM_BOTOES] = {0, 0, 0, 0};
const unsigned long DELAY_DEBOUNCE = 50; // 50 milissegundos para filtrar ruído mecânico
unsigned long tempoInicioBoot = 0;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Desliga os relés de balizamento (preserva o FOOT LIGHT)
void desligarBalizamentoReles() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(PINOS_RELE[i], RELE_DESLIGADO);
  }
}

// Desliga todos os relés da placa
void desligarTodosReles() {
  for (int i = 0; i < NUM_RELES; i++) {
    digitalWrite(PINOS_RELE[i], RELE_DESLIGADO);
  }
}

// Aplica o nível de brilho com intertravamento nos 3 primeiros relés
void aplicarNivelBrilho(int nivel) {
  if (nivel < 1 || nivel > 3) {
    desligarBalizamentoReles();
    brilhoAtual = 0;
    Serial.println("[STATUS] Balizamento DESLIGADO.");
  } else {
    // Intertravamento: desliga os outros relés de balizamento antes de ligar o novo
    desligarBalizamentoReles();

    uint8_t releIndice = RELE_ESTAGIO[nivel - 1];
    digitalWrite(PINOS_RELE[releIndice], RELE_LIGADO);

    brilhoAtual = nivel;
    Serial.printf("[STATUS] Estágio %d ATIVADO! Relé %d (GPIO %d) LIGADO.\n",
                  nivel, releIndice + 1, PINOS_RELE[releIndice]);
  }

  // Notifica o novo estado do balizamento via MQTT
  if (mqttClient.connected()) {
    char payload[4];
    sprintf(payload, "%d", brilhoAtual);
    mqttClient.publish(TOPICO_STATUS.c_str(), payload, true);
  }
}

// Controle do relé auxiliar FOOT LIGHT (independente do balizamento)
void definirFootLight(bool estado) {
  footLightAtivo = estado;
  digitalWrite(PINO_RELE_FOOTLIGHT, footLightAtivo ? RELE_LIGADO : RELE_DESLIGADO);
  Serial.printf("[STATUS] FOOT LIGHT %s! Relé 4 (GPIO %d)\n",
                footLightAtivo ? "LIGADO" : "DESLIGADO", PINO_RELE_FOOTLIGHT);

  if (mqttClient.connected()) {
    mqttClient.publish(TOPICO_FOOTLIGHT_STATUS.c_str(), footLightAtivo ? "1" : "0", true);
  }
}

void alternarFootLight() {
  definirFootLight(!footLightAtivo);
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
  msg.trim();

  Serial.println("\n----------------------------------------------");
  Serial.printf("[COMANDO NUVEM RECEBIDO]\n  Tópico: %s\n  Mensagem: '%s'\n", topic, msg.c_str());

  String topicoStr = String(topic);

  // Aceita tanto o tópico padrão quanto qualquer ID ou tópico legado
  if (topicoStr.endsWith("/balizamento/comando") || topicoStr == "smarthelipontos/balizamento/comando") {
    int nivel = msg.toInt();
    Serial.printf("➔ Acionando Nível de Brilho: %d\n", nivel);
    aplicarNivelBrilho(nivel);
  } else if (topicoStr.endsWith("/footlight/comando") || topicoStr == "smarthelipontos/footlight/comando") {
    Serial.printf("➔ Acionando FOOT LIGHT com comando: %s\n", msg.c_str());
    if (msg == "1") {
      definirFootLight(true);
    } else if (msg == "0") {
      definirFootLight(false);
    } else if (msg == "toggle") {
      alternarFootLight();
    }
  }
  Serial.println("----------------------------------------------");
}

// Leitura contínua dos 4 botões físicos no painel com debounce
void verificarBotoesFisicos() {
  // Ignora ruído elétrico de inicialização nos primeiros 1.5 segundos após boot
  if (millis() - tempoInicioBoot < 1500) {
    for (int i = 0; i < NUM_BOTOES; i++) {
      ultimoEstadoBotao[i] = digitalRead(PINOS_BOTAO[i]);
    }
    return;
  }

  for (int i = 0; i < NUM_BOTOES; i++) {
    int leitura = digitalRead(PINOS_BOTAO[i]);

    // Detecta transição de solto (HIGH) para pressionado (LOW)
    if (leitura != ultimoEstadoBotao[i]) {
      ultimoTempoDebounce[i] = millis();
    }

    if ((millis() - ultimoTempoDebounce[i]) > DELAY_DEBOUNCE) {
      // Se o botão está realmente pressionado (LOW com PULL-UP)
      static bool estadoProcessado[NUM_BOTOES] = {false, false, false, false};

      if (leitura == LOW && !estadoProcessado[i]) {
        estadoProcessado[i] = true;
        if (i < 3) {
          Serial.printf("\n[BOTÃO FÍSICO %d PRESSIONADO (GPIO %d)]\n", i + 1, PINOS_BOTAO[i]);
          alternarBrilho(i + 1); // 1 = Brilho 1, 2 = Brilho 2, 3 = Brilho 3
        } else if (i == 3) {
          Serial.printf("\n[BOTÃO FÍSICO FOOT LIGHT PRESSIONADO (GPIO %d)]\n", PINOS_BOTAO[i]);
          alternarFootLight();
        }
      } else if (leitura == HIGH) {
        estadoProcessado[i] = false;
      }
    }

    ultimoEstadoBotao[i] = leitura;
  }
}

// ============================================================================
// GERENCIAMENTO DE CONEXÃO NÃO-BLOQUEANTE (Não congela botoeiras nem relés)
// ============================================================================
unsigned long ultimoCheckRede = 0;

void verificarConexoes() {
  // Checa rede a cada 4 segundos sem travar a execução do processador
  if (millis() - ultimoCheckRede < 4000) return;
  ultimoCheckRede = millis();

  // 1. Checa status do Wi-Fi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[Wi-Fi] Reconectando ao Wi-Fi...");
    WiFi.disconnect();
    WiFi.reconnect();
    return;
  }

  // 2. Checa status do Broker MQTT
  if (!mqttClient.connected()) {
    Serial.print("[MQTT] Conectando ao Broker (broker.emqx.io)... ");
    String clientId = "DevKitV1-Heliponto-" + String(random(0xffff), HEX);

    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("CONECTADO A NUVEM COM SUCESSO!");
      // Assina os canais do heliponto e universais com coringa
      mqttClient.subscribe(TOPICO_COMANDO.c_str());
      mqttClient.subscribe(TOPICO_FOOTLIGHT_CMD.c_str());
      mqttClient.subscribe("smarthelipontos/+/balizamento/comando");
      mqttClient.subscribe("smarthelipontos/+/footlight/comando");
      mqttClient.subscribe("smarthelipontos/balizamento/comando");
      mqttClient.subscribe("smarthelipontos/footlight/comando");

      // Notifica estados atuais ao broker
      char payload[4];
      sprintf(payload, "%d", brilhoAtual);
      mqttClient.publish(TOPICO_STATUS.c_str(), payload, true);
      mqttClient.publish(TOPICO_FOOTLIGHT_STATUS.c_str(), footLightAtivo ? "1" : "0", true);
    } else {
      Serial.printf("Falha (rc=%d). Próxima tentativa em 4s.\n", mqttClient.state());
    }
  }
}

void setup() {
  // 1. PRIMEIRA INSTRUÇÃO ABSOLUTA: Configura os relés como DESLIGADOS imediatamente (sem delay, sem pulsar)
  for (int i = 0; i < NUM_RELES; i++) {
    pinMode(PINOS_RELE[i], OUTPUT);
    digitalWrite(PINOS_RELE[i], RELE_DESLIGADO);
  }
  desligarTodosReles();

  // 2. Inicializa a comunicação Serial
  Serial.begin(115200);
  tempoInicioBoot = millis();

  // 3. Inicializa os 4 Push Buttons com PULL-UP interno (Entradas)
  for (int i = 0; i < NUM_BOTOES; i++) {
    pinMode(PINOS_BOTAO[i], INPUT_PULLUP);
    ultimoEstadoBotao[i] = digitalRead(PINOS_BOTAO[i]);
  }

  Serial.println("\n==============================================");
  Serial.println("   SMART HELIPONTOS - ESP32 DevKit V1");
  Serial.println("   Balizamento: Relés 1, 2 e 3 (GPIOs 18, 19, 21)");
  Serial.println("   Foot Light:  Relé 4 (GPIO 22)");
  Serial.println("   Botoeiras:   GPIOs 32, 33, 27, 14 (➔ GND)");
  Serial.println("   Status:      Todos os relés desligados no boot");
  Serial.println("==============================================");

  // Inicia Wi-Fi em segundo plano
  Serial.print("Iniciando Wi-Fi: ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(callbackMQTT);
}

void loop() {
  // 1. Botões Físicos rodam LIVRES na velocidade máxima (resposta instantânea em 0ms)
  verificarBotoesFisicos();

  // 2. Conexões de rede gerenciadas em segundo plano (NUNCA travam os botões)
  verificarConexoes();

  // 3. Processa comandos MQTT se conectado
  if (mqttClient.connected()) {
    mqttClient.loop();
  }
}
