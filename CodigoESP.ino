//bibliotecas
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <ArduinoJson.h>

//pinos da esp
#define SS_PIN 15
#define RST_PIN 2
#define LED_VERDE 3 

// DADOS DO WI-FI
const char* ssid = "DAINE E FELIPE";
const char* password = "@Felipe2002";

// Endereço do servidor Flask
const char* servidor = "http://192.168.1.18:5000/api/rfid";

MFRC522 rfid(SS_PIN, RST_PIN);

bool aulaAberta = false;

void setup() {
 
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 

  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("=================================");
  Serial.println(" Sistema de Controle de Presença ");
  Serial.println("=================================");

  // CONECTA AO WI-FI
  Serial.print("Conectando ao Wi-Fi");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi conectado com sucesso!");
  Serial.print("IP da ESP32: ");
  Serial.println(WiFi.localIP());

  Serial.print("Servidor: ");
  Serial.println(servidor);

  // RFID
  SPI.begin(14, 12, 13, 15);
  rfid.PCD_Init();

  // LED
  pinMode(LED_VERDE, OUTPUT);
  digitalWrite(LED_VERDE, LOW);

  Serial.println();
  Serial.println("Status da aula: FECHADA");
  Serial.println("Aproxime o cartão...");
  Serial.println("=================================");
}

void loop() {

  // Aguarda um cartão
  if (!rfid.PICC_IsNewCardPresent())
    return;

  if (!rfid.PICC_ReadCardSerial())
    return;

  String uid = "";

Serial.print("UID do cartão: ");

for (byte i = 0; i < rfid.uid.size; i++) {

  if (rfid.uid.uidByte[i] < 0x10) {
    Serial.print("0");
    uid += "0";
  }

  Serial.print(rfid.uid.uidByte[i], HEX);
  uid += String(rfid.uid.uidByte[i], HEX);

  Serial.print(" ");
}

uid.toUpperCase();

Serial.println();

Serial.print("UID para enviar ao servidor: ");
Serial.println(uid);

// ENVIA UID PARA O SERVIDOR
if (WiFi.status() == WL_CONNECTED) {

  HTTPClient http;

  http.begin(servidor);

  http.addHeader("Content-Type", "application/json");

  String json = "{\"uid\":\"" + uid + "\"}";

  int codigoHTTP = http.POST(json);

  Serial.print("Código HTTP: ");
  Serial.println(codigoHTTP);

  if (codigoHTTP > 0) {

    String resposta = http.getString();

    Serial.println("Resposta do servidor:");
    Serial.println(resposta);

    // Aqui vamos interpretar o JSON
    JsonDocument doc;
    deserializeJson(doc, resposta);

    String acao = doc["acao"];

    if (acao == "abrir") {

      Serial.println("===== AULA INICIADA =====");
      digitalWrite(LED_VERDE, HIGH);

    } else if (acao == "fechar") {

      Serial.println("===== AULA ENCERRADA =====");
      digitalWrite(LED_VERDE, LOW);

    }

  } else {

    Serial.println("Erro ao enviar para o servidor!");

  }

  http.end();

} else {

  Serial.println("Wi-Fi desconectado!");

}

  Serial.println("--------------------------------");

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  delay(1500);
}
