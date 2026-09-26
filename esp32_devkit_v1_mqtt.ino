/*
 * ============================================================================
 * PROJETO: SMART HELIPONTOS - CONTROLE DE BALIZAMENTO NOTURNO E FOOT LIGHT
 * HARDWARE: ESP32 DevKit V1 (ESP-WROOM-32 de 30 ou 38 pinos)
 * 
 * RECURSOS:
 *  1. Conexão Wi-Fi Dinâmica via Celular (WiFiManager / Portal Cativo AP)
 *     - Cria rede "SmartHeliponto-Config" se não houver Wi-Fi memorizado
 *     - Permite configurar o Wi-Fi e o "ID do Heliponto" diretamente no celular
 *  2. Botão Dedicado de Reset de Configuração Wi-Fi (GPIO 23 -> GND)
 *     - Segure por 3 segundos a qualquer momento para abrir o portal de configuração
 *  3. Controle via Nuvem MQTT Global (App Web / Celular 4G e PC)
 *  4. Controle Físico Local com 4 Push Buttons (Botoeiras no Painel)
 *  5. Operação 100% autônoma (botoeiras funcionam mesmo sem internet)
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
 * [ENTRADAS PARA OS BOTÕES FÍSICOS (BOTOEIRAS)]
 *   - Usando PULL-UP interno (Ligue o botão entre o pino do ESP32 e o GND):
 *   - Botão 1 (Brilho 1):   GPIO 32 ➔ GND
 *   - Botão 2 (Brilho 2):   GPIO 33 ➔ GND
 *   - Botão 3 (Brilho 3):   GPIO 27 ➔ GND
 *   - Botão 4 (FOOT LIGHT): GPIO 14 ➔ GND
 *   - Botão 5 (RESET WI-FI): GPIO 23 ➔ GND (Segure por 3s para reconfigurar)
 * 
 * BIBLIOTECAS NECESSÁRIAS (Instalar no Gerenciador de Bibliotecas da Arduino IDE):
 *   1. "WiFiManager" por tzapu / tablatronix (versão 2.0.x ou superior)
 *   2. "PubSubClient" por Nick O'Leary
 * ============================================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>  // Gerenciador de Wi-Fi e Portal Cativo no Celular
#include <Preferences.h>  // Memória Flash permanente interna (NVS) do ESP32

// ============================================================================
// 1. IDENTIFICAÇÃO E BROKER MQTT (Armazenados na Memória Flash NVS)
// ============================================================================
Preferences preferences;

// Valor padrão caso nunca tenha sido configurado pelo portal
char heliponto_id[32] = "padrao";
const char* MQTT_BROKER = "broker.emqx.io";
const int   MQTT_PORT   = 1883;

// Tópicos dinâmicos montados a partir do heliponto_id
String TOPICO_COMANDO          = "";
String TOPICO_STATUS           = "";
String TOPICO_FOOTLIGHT_CMD    = "";
String TOPICO_FOOTLIGHT_STATUS = "";

void atualizarTopicosMQTT() {
  TOPICO_COMANDO          = "smarthelipontos/" + String(heliponto_id) + "/balizamento/comando";
  TOPICO_STATUS           = "smarthelipontos/" + String(heliponto_id) + "/balizamento/status";
  TOPICO_FOOTLIGHT_CMD    = "smarthelipontos/" + String(heliponto_id) + "/footlight/comando";
  TOPICO_FOOTLIGHT_STATUS = "smarthelipontos/" + String(heliponto_id) + "/footlight/status";
  
  Serial.println("[MQTT] Tópicos configurados para o Heliponto ID: " + String(heliponto_id));
}

// Flag que indica se o usuário salvou novas configurações no portal web
bool salvarConfiguracoesFlash = false;
void callbackSalvarConfig() {
  Serial.println("[PORTAL] Novas configurações recebidas da página web!");
  salvarConfiguracoesFlash = true;
}

// ============================================================================
// 2. PINAGEM DOS RELÉS E BOTÕES FÍSICOS
// ============================================================================
const uint8_t NUM_RELES = 6;
const uint8_t PINOS_RELE[NUM_RELES] = {18, 19, 21, 22, 25, 26};

// Mapeamento de 1 relé por estágio de brilho:
const uint8_t RELE_ESTAGIO[3] = {0, 1, 2}; // 1 -> Relé 1, 2 -> Relé 2, 3 -> Relé 3
const uint8_t PINO_RELE_FOOTLIGHT = 22;    // Relé 4 (GPIO 22) para FOOT LIGHT

// 4 Botões Operacionais do Painel:
const uint8_t NUM_BOTOES = 4;
const uint8_t PINOS_BOTAO[NUM_BOTOES] = {32, 33, 27, 14};

// NOVO: 5º Botão - Reset e Reconfiguração de Wi-Fi
const uint8_t PINO_BOTAO_RESET_WIFI = 23; // Ligue entre GPIO 23 e GND

// Lógica de relé: Active LOW (LOW liga, HIGH desliga)
#define RELE_LIGADO    LOW
#define RELE_DESLIGADO HIGH

int  brilhoAtual     = 0;     // 0 = Desligado, 1 = 30%, 2 = 70%, 3 = 100%
bool footLightAtivo  = false; // Estado do FOOT LIGHT

// Debounce dos botões físicos
bool ultimoEstadoBotao[NUM_BOTOES] = {HIGH, HIGH, HIGH, HIGH};
unsigned long ultimoTempoDebounce[NUM_BOTOES] = {0, 0, 0, 0};
const unsigned long DELAY_DEBOUNCE = 50; 
unsigned long tempoInicioBoot = 0;

// Variáveis de monitoramento do botão de Reset Wi-Fi
unsigned long tempoInicioPressionamentoReset = 0;
bool botaoResetPressionado = false;
const unsigned long TEMPO_RESET_WIFI_MS = 3000; // 3 segundos segurando para abrir portal

WiFiClient espClient;
PubSubClient mqttClient(espClient);
WiFiManager wifiManager;

// ============================================================================
// 3. CONTROLE FÍSICO DOS RELÉS (INTERTRAVAMENTO & SEGURANÇA)
// ============================================================================
void desligarBalizamentoReles() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(PINOS_RELE[i], RELE_DESLIGADO);
  }
}

void desligarTodosReles() {
  for (int i = 0; i < NUM_RELES; i++) {
    digitalWrite(PINOS_RELE[i], RELE_DESLIGADO);
  }
}

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

void alternarBrilho(int nivel) {
  if (brilhoAtual == nivel) {
    aplicarNivelBrilho(0); // Desliga (Toggle)
  } else {
    aplicarNivelBrilho(nivel);
  }
}

// ============================================================================
// 4. PROCESSAMENTO DE MENSAGENS MQTT DA NUVEM
// ============================================================================
void callbackMQTT(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  msg.trim();

  Serial.printf("\n[MQTT RECEBIDO] Tópico: %s | Comando: %s\n", topic, msg.c_str());

  String strTopic = String(topic);

  // Comando de Balizamento
  if (strTopic == TOPICO_COMANDO || strTopic.endsWith("/balizamento/comando")) {
    int nivel = msg.toInt();
    if (nivel >= 0 && nivel <= 3) {
      Serial.printf("[COMANDO NUVEM] Aplicando Balizamento Estágio: %d\n", nivel);
      aplicarNivelBrilho(nivel);
    }
  }
  // Comando de FOOT LIGHT
  else if (strTopic == TOPICO_FOOTLIGHT_CMD || strTopic.endsWith("/footlight/comando")) {
    if (msg == "1" || msg.equalsIgnoreCase("ON") || msg.equalsIgnoreCase("TRUE")) {
      definirFootLight(true);
    } else if (msg == "0" || msg.equalsIgnoreCase("OFF") || msg.equalsIgnoreCase("FALSE")) {
      definirFootLight(false);
    } else if (msg.equalsIgnoreCase("TOGGLE")) {
      alternarFootLight();
    }
  }
}

// ============================================================================
// 5. LEITURA CONTÍNUA DAS BOTOEIRAS FÍSICAS (EXECUÇÃO INSTANTÂNEA)
// ============================================================================
void verificarBotoesFisicos() {
  // Ignora ruído elétrico nos primeiros 1.5s após boot
  if (millis() - tempoInicioBoot < 1500) {
    for (int i = 0; i < NUM_BOTOES; i++) {
      ultimoEstadoBotao[i] = digitalRead(PINOS_BOTAO[i]);
    }
    return;
  }

  // 1. Botoeiras Operacionais (Balizamento e Foot Light)
  for (int i = 0; i < NUM_BOTOES; i++) {
    int leitura = digitalRead(PINOS_BOTAO[i]);

    if (leitura != ultimoEstadoBotao[i]) {
      ultimoTempoDebounce[i] = millis();
    }

    if ((millis() - ultimoTempoDebounce[i]) > DELAY_DEBOUNCE) {
      static bool estadoProcessado[NUM_BOTOES] = {false, false, false, false};

      if (leitura == LOW && !estadoProcessado[i]) {
        estadoProcessado[i] = true;
        if (i < 3) {
          Serial.printf("\n[BOTÃO FÍSICO %d PRESSIONADO (GPIO %d)]\n", i + 1, PINOS_BOTAO[i]);
          alternarBrilho(i + 1);
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

  // 2. Botão Dedicado de Reset de Wi-Fi (GPIO 23 ➔ GND)
  int leituraReset = digitalRead(PINO_BOTAO_RESET_WIFI);
  if (leituraReset == LOW) {
    if (!botaoResetPressionado) {
      botaoResetPressionado = true;
      tempoInicioPressionamentoReset = millis();
      Serial.println("[RESET WI-FI] Botão pressionado. Segure por 3 segundos para abrir o Portal de Configuração...");
    } else {
      unsigned long duracaoPressionado = millis() - tempoInicioPressionamentoReset;
      if (duracaoPressionado >= TEMPO_RESET_WIFI_MS) {
        Serial.println("\n=======================================================");
        Serial.println("  [RESET WI-FI CONFIRMADO!] ABRINDO PORTAL DE CONFIGURAÇÃO");
        Serial.println("  Conecte seu celular na rede Wi-Fi: SmartHeliponto-Config");
        Serial.println("  Acesse no navegador: http://192.168.4.1");
        Serial.println("=======================================================\n");

        iniciarPortalConfiguracao(true);
        botaoResetPressionado = false;
      }
    }
  } else {
    botaoResetPressionado = false;
  }
}

// ============================================================================
// 6. ROTINA DE CONFIGURAÇÃO WI-FI E PORTAL CATIVO NO CELULAR
// ============================================================================
void iniciarPortalConfiguracao(bool forcarPortalManual) {
  // Campo customizado para o operador digitar o ID do Heliponto no celular
  WiFiManagerParameter custom_heliponto_id("heli_id", "ID do Heliponto (Ex: padrao, torre-alfa)", heliponto_id, 32);
  
  wifiManager.addParameter(&custom_heliponto_id);
  wifiManager.setSaveConfigCallback(callbackSalvarConfig);
  
  // Timeout de 180 segundos (3 minutos) no portal. 
  // Se ninguém configurar, o painel sai do modo portal e mantém as botoeiras funcionando!
  wifiManager.setConfigPortalTimeout(180);

  bool conectado = false;
  if (forcarPortalManual) {
    // Inicia o portal de ponto de acesso diretamente
    conectado = wifiManager.startConfigPortal("SmartHeliponto-Config");
  } else {
    // Tenta conectar no Wi-Fi memorizado; se falhar, abre o portal
    conectado = wifiManager.autoConnect("SmartHeliponto-Config");
  }

  // Se o usuário preencheu e salvou no portal:
  if (salvarConfiguracoesFlash) {
    salvarConfiguracoesFlash = false;
    strncpy(heliponto_id, custom_heliponto_id.getValue(), sizeof(heliponto_id) - 1);
    heliponto_id[sizeof(heliponto_id) - 1] = '\0';

    // Grava na Flash NVS permanente
    preferences.begin("smartheli", false);
    preferences.putString("heli_id", heliponto_id);
    preferences.end();

    Serial.printf("[MEMÓRIA FLASH] Novo Heliponto ID gravado com sucesso: %s\n", heliponto_id);
    atualizarTopicosMQTT();
  }

  if (conectado) {
    Serial.println("\n[Wi-Fi] Conectado à rede com sucesso!");
    Serial.print("[Wi-Fi] Endereço IP Local: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n[Wi-Fi] Timeout do portal atingido. Continuando em modo manual local...");
  }
}

// ============================================================================
// 7. GERENCIAMENTO DE CONEXÃO NÃO-BLOQUEANTE (NÃO TRAVA BOTOEIRAS)
// ============================================================================
unsigned long ultimoCheckRede = 0;

void verificarConexoes() {
  if (millis() - ultimoCheckRede < 4000) return;
  ultimoCheckRede = millis();

  // 1. Checa status do Wi-Fi
  if (WiFi.status() != WL_CONNECTED) {
    return; // Em caso de perda, o WiFiManager reconecta em segundo plano
  }

  // 2. Checa status do Broker MQTT
  if (!mqttClient.connected()) {
    Serial.print("[MQTT] Conectando ao Broker (broker.emqx.io)... ");
    String clientId = "DevKitV1-" + String(heliponto_id) + "-" + String(random(0xffff), HEX);

    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("CONECTADO A NUVEM COM SUCESSO!");

      // Assina os tópicos deste heliponto
      mqttClient.subscribe(TOPICO_COMANDO.c_str());
      mqttClient.subscribe(TOPICO_FOOTLIGHT_CMD.c_str());
      mqttClient.subscribe("smarthelipontos/+/balizamento/comando");
      mqttClient.subscribe("smarthelipontos/+/footlight/comando");

      // Publica status atual imediatamente ao restabelecer conexão
      char payload[4];
      sprintf(payload, "%d", brilhoAtual);
      mqttClient.publish(TOPICO_STATUS.c_str(), payload, true);
      mqttClient.publish(TOPICO_FOOTLIGHT_STATUS.c_str(), footLightAtivo ? "1" : "0", true);
    } else {
      Serial.printf("Falha (rc=%d). Próxima tentativa em 4s.\n", mqttClient.state());
    }
  }
}

// ============================================================================
// 8. SETUP PRINCIPAL
// ============================================================================
void setup() {
  // 1. PRIMEIRA INSTRUÇÃO ABSOLUTA: Desliga todos os relés sem nenhum pulso indesejado
  for (int i = 0; i < NUM_RELES; i++) {
    pinMode(PINOS_RELE[i], OUTPUT);
    digitalWrite(PINOS_RELE[i], RELE_DESLIGADO);
  }
  desligarTodosReles();

  // 2. Inicializa a Serial
  Serial.begin(115200);
  tempoInicioBoot = millis();

  // 3. Inicializa os 4 botões operacionais com PULL-UP interno
  for (int i = 0; i < NUM_BOTOES; i++) {
    pinMode(PINOS_BOTAO[i], INPUT_PULLUP);
    ultimoEstadoBotao[i] = digitalRead(PINOS_BOTAO[i]);
  }

  // 4. Inicializa o botão de Reset de Wi-Fi com PULL-UP interno
  pinMode(PINO_BOTAO_RESET_WIFI, INPUT_PULLUP);

  // 5. Carrega o Heliponto ID salvo na memória Flash permanente (NVS)
  preferences.begin("smartheli", true); // Modo somente leitura
  String idSalvo = preferences.getString("heli_id", "padrao");
  preferences.end();
  idSalvo.toCharArray(heliponto_id, sizeof(heliponto_id));

  atualizarTopicosMQTT();

  Serial.println("\n=======================================================");
  Serial.println("       SMART HELIPONTOS - ESP32 DevKit V1");
  Serial.println("  Balizamento:   Relés 1, 2 e 3 (GPIOs 18, 19, 21)");
  Serial.println("  Foot Light:    Relé 4 (GPIO 22)");
  Serial.println("  Botoeiras:     GPIOs 32, 33, 27, 14 (➔ GND)");
  Serial.println("  Botão Reset:   GPIO 23 (➔ GND - Segure 3s para Wi-Fi)");
  Serial.printf ("  Heliponto ID:  [%s]\n", heliponto_id);
  Serial.println("=======================================================");

  // 6. Inicia o gerenciamento inteligente de Wi-Fi
  // Se o botão de reset já estiver sendo segurado no momento em que liga a energia:
  if (digitalRead(PINO_BOTAO_RESET_WIFI) == LOW) {
    Serial.println("[BOOT] Botão de Reset pressionado ao ligar! Abrindo portal direto...");
    iniciarPortalConfiguracao(true);
  } else {
    iniciarPortalConfiguracao(false);
  }

  // 7. Configura cliente MQTT
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(callbackMQTT);
}

// ============================================================================
// 9. LOOP PRINCIPAL
// ============================================================================
void loop() {
  // 1. Botões Físicos rodam LIVRES na velocidade máxima do processador (0ms de lag)
  verificarBotoesFisicos();

  // 2. Conexões de rede e MQTT em segundo plano (NUNCA travam os botões locais)
  verificarConexoes();

  // 3. Processa comandos MQTT recebidos da nuvem
  if (mqttClient.connected()) {
    mqttClient.loop();
  }
}
