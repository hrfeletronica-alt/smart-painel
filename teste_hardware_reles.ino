/*
 * TESTE DE BANCADA - SMART HELIPONTOS
 * Testa apenas os relés fisicamente de 1 em 1 segundo.
 * Sem Wi-Fi, sem MQTT, sem interferência de rede.
 */

#include <Arduino.h>

// Pinos configurados para o ESP32 DevKit V1:
const uint8_t PINOS[4] = {18, 19, 21, 22};
const char* NOMES[4]   = {"RELE 1 (30%)", "RELE 2 (70%)", "RELE 3 (100%)", "RELE 4 (FOOT LIGHT)"};

// A maioria dos módulos relé usa Active LOW (LOW liga, HIGH desliga)
#define LIGAR    LOW
#define DESLIGAR HIGH

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n==============================================");
  Serial.println("   INICIANDO TESTE FISICO DOS 4 RELES");
  Serial.println("   Pinos testados: GPIO 18, 19, 21 e 22");
  Serial.println("==============================================");

  for (int i = 0; i < 4; i++) {
    pinMode(PINOS[i], OUTPUT);
    digitalWrite(PINOS[i], DESLIGAR); // Inicia desligado
  }
}

void loop() {
  for (int i = 0; i < 4; i++) {
    Serial.printf(">>> LIGANDO %s no GPIO %d...\n", NOMES[i], PINOS[i]);
    digitalWrite(PINOS[i], LIGAR);
    delay(1000); // Fica ligado por 1 segundo

    Serial.printf("<<< DESLIGANDO %s...\n\n", NOMES[i]);
    digitalWrite(PINOS[i], DESLIGAR);
    delay(500); // Pausa de meio segundo
  }

  Serial.println("--- Ciclo concluido. Reiniciando em 2 segundos ---\n");
  delay(2000);
}
