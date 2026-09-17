/*
 * ESP32-S3 (N16R8) - Controlador de 6 Relés com 3 Canais (Exclusão Mútua)
 * Regra:
 * - Botão 1: Aciona Relé 1 (GPIO 4) e Relé 2 (GPIO 5)
 * - Botão 2: Aciona Relé 3 (GPIO 6) e Relé 4 (GPIO 7)
 * - Botão 3: Aciona Relé 5 (GPIO 15) e Relé 6 (GPIO 16)
 * - Ao acionar um botão, o outro é ANULADO imediatamente (nunca mais de 2 relés ativos).
 * - Clicar no mesmo botão desliga os 2 relés (toggle).
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// Pinos dos 6 Relés
const uint8_t NUM_RELES = 6;
const uint8_t PINOS_RELE[NUM_RELES] = {4, 5, 6, 7, 15, 16};

// Módulos relé normalmente são Active LOW
#define RELE_LIGADO    LOW
#define RELE_DESLIGADO HIGH

// Definição dos 3 grupos (2 relés por grupo)
const uint8_t GRUPOS[3][2] = {
  {0, 1}, // Grupo 1: Relé 1 e 2
  {2, 3}, // Grupo 2: Relé 3 e 4
  {4, 5}  // Grupo 3: Relé 5 e 6
};

// -1 = Nenhum ativo, 0 = Botão 1, 1 = Botão 2, 2 = Botão 3
int botaoAtivo = -1;

WebServer server(80);

void desligarTodosReles() {
  for (int i = 0; i < NUM_RELES; i++) {
    digitalWrite(PINOS_RELE[i], RELE_DESLIGADO);
  }
}

void acionarGrupo(int indiceBotao) {
  // Se for -1 ou comando de desligar tudo (ex: 0 no endpoint)
  if (indiceBotao < 0 || indiceBotao >= 3) {
    desligarTodosReles();
    botaoAtivo = -1;
    Serial.println("[OFF] Todos os relés foram desligados.");
    return;
  }

  // CASO 1: Clicou no botão que já está ativo -> DESLIGA (Toggle)
  if (botaoAtivo == indiceBotao) {
    desligarTodosReles();
    botaoAtivo = -1;
    Serial.printf("[TOGGLE OFF] Botão %d desacionado.\n", indiceBotao + 1);
  }
  // CASO 2: Clicou em um botão diferente -> ANULA O ANTERIOR E LIGA O NOVO
  else {
    // 1º Desliga todos para garantir a anulação imediata sem transitório
    desligarTodosReles();

    // 2º Liga apenas os 2 relés do novo grupo
    uint8_t r1 = GRUPOS[indiceBotao][0];
    uint8_t r2 = GRUPOS[indiceBotao][1];
    digitalWrite(PINOS_RELE[r1], RELE_LIGADO);
    digitalWrite(PINOS_RELE[r2], RELE_LIGADO);

    botaoAtivo = indiceBotao;
    Serial.printf("[ON] Botão %d acionado! Relés %d e %d ligados.\n", 
                  indiceBotao + 1, r1 + 1, r2 + 1);
  }
}

void setup() {
  Serial.begin(115200);

  // Configura pinos como saída e inicia tudo desligado
  for (int i = 0; i < NUM_RELES; i++) {
    pinMode(PINOS_RELE[i], OUTPUT);
    digitalWrite(PINOS_RELE[i], RELE_DESLIGADO);
  }

  // Ponto de Acesso Wi-Fi do ESP32 para o Painel conectar
  WiFi.softAP("ESP32-SmartPanel", "12345678");
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Access Point Criado. IP do ESP32: ");
  Serial.println(IP);

  // Rota HTTP chamada pelo Painel Web
  server.on("/setGroup", []() {
    if (server.hasArg("btn")) {
      int btn = server.arg("btn").toInt(); // 1, 2, 3 ou 0 (desliga tudo)
      if (btn == 0) {
        acionarGrupo(-1);
      } else {
        acionarGrupo(btn - 1);
      }
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Falta parametro btn");
    }
  });

  server.begin();
  Serial.println("Servidor Web iniciado. Digite 1, 2 ou 3 no Serial para testar.");
}

void loop() {
  server.handleClient();

  // Teste interativo pelo Monitor Serial (digite 1, 2, 3 ou 0)
  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c >= '1' && c <= '3') {
      acionarGrupo(c - '1');
    } else if (c == '0') {
      acionarGrupo(-1);
    }
  }
}
