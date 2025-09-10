#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_PN532.h>
#include <Wire.h>

#define WIFI
#define NFC

//////////////////////////////////
// Configuración WiFi
#ifdef WIFI
  #define WIFI_SSID "ssid de la red"
  #define WIFI_PASSWORD "pw"
  #define WIFI_CHANNEL 6
#endif
//////////////////////////////////

//////////////////////////////////
// Configuración del PN532
#ifdef NFC
  #define SDA_PIN 4  // D2
  #define SCL_PIN 5  // D1
  #define PN532_IRQ   2   // cualquier pin libre
  #define PN532_RESET 3   // cualquier pin libre
  Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET, &Wire);
#endif
//////////////////////////////////

#define ENDPOINT_URL "http://IP:8080/turnero"

void setup() {
  Serial.begin(115200);
  delay(100);
  pinMode(LED_BUILTIN, OUTPUT); // Configura LED como salida

  // Seteamos el internet
  #ifdef WIFI
    Serial.printf("Conectando a la red WiFi: %s\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    digitalWrite(LED_BUILTIN, LOW); // Enciende el led, en el esp8266 el low es el high, ni idea por que

    Serial.println("");
    Serial.printf("Conectado a %s\n", WIFI_SSID);
    Serial.print("IP asignada: ");
    Serial.println(WiFi.localIP());  // IP local del ESP8266
  #endif

  // Comenzar lectura NFC
  #ifdef NFC
    Serial.printf("Iniciando conexión con PN532\n");
    Wire.begin(SDA_PIN, SCL_PIN);
    nfc.begin();


    uint32_t versiondata = nfc.getFirmwareVersion();
    if (!versiondata) {
      Serial.println("No se pudo encontrar PN532");
      while(1) delay(5000);
    }

    Serial.print("PN532 encontrado, firmware ver: 0x");
    Serial.println(versiondata, HEX);

    nfc.SAMConfig(); // Configura el PN532 para lectura de tarjetas
    Serial.println("Listo para leer llaveros NFC.");
  #endif
}

void loop() {
  #ifdef NFC
  uint8_t success;
  uint8_t uid[7];      // Tamaño máximo UID
  uint8_t uidLength;

  // Espera a que un llavero NFC esté presente
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {
    Serial.print("UID leído: ");
    for (uint8_t i = 0; i < uidLength; i++) {
      Serial.print(uid[i], HEX);
      if (i < uidLength - 1) Serial.print(":");
    }
    #ifdef WIFI
      String szUid = uidToString(uid, uidLength);
      sendAttendance(szUid.c_str());
    #endif
    Serial.println();
    delay(1000); // Pequeño retardo para evitar lecturas repetidas inmediatas
  }
  #endif
}

String uidToString(uint8_t *uid, uint8_t uidLength) {
  String uidString = "";
  for (uint8_t i = 0; i < uidLength; i++) {
    if (uid[i] < 0x10) uidString += "0"; // asegura siempre 2 dígitos
    uidString += String(uid[i], HEX);
    if (i < uidLength - 1) uidString += ":"; // separador entre bytes
  }
  uidString.toUpperCase(); // opcional, pone en mayúsculas
  return uidString;
}


void sendAttendance(const char* dni) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Error: WiFi no conectado");
    reconnectWiFi();
    return;
  }

  WiFiClient client;
  // WiFiClientSecure client;
  HTTPClient http;
  // client.setInsecure();  // No valida certificado, útil para pruebas

  // Iniciar conexión HTTP
  http.begin(client, ENDPOINT_URL);

  http.addHeader("Content-Type", "application/json");
  http.setReuse(true); // Reutilizar conexión
  http.setTimeout(10000); // Timeout de 10 segundos

  // Crear el JSON con el DNI
  String jsonPayload = "{\"uid\":\"" + String(dni) + "\"}";
  
  Serial.print("Enviando attendance: ");
  Serial.println(jsonPayload);

  // Enviar solicitud POST (no GET, porque estás enviando datos)
  int httpCode = http.POST(jsonPayload);
  // int httpCode = http.GET();

  // Verificar respuesta
  if (httpCode > 0) {
    Serial.printf("Código HTTP: %d\n", httpCode);
    
    if (httpCode == HTTP_CODE_OK) {
      String response = http.getString();
      Serial.print("Respuesta del servidor: ");
      Serial.println(response);
    } else {
      Serial.printf("Respuesta inesperada: %d\n", httpCode);
      String errorResponse = http.getString();
      Serial.print("Error response: ");
      Serial.println(errorResponse);
    }
  } else {
    Serial.printf("Error de conexión: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end(); // Siempre cerrar la conexión
  delay(1000);
}

#ifdef WIFI
  void reconnectWiFi() {
    if (WiFi.status() == WL_CONNECTED) return; // ya está conectado

    Serial.println("WiFi desconectado, intentando reconectar...");

    WiFi.disconnect(); // fuerza desconexión previa
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);

    unsigned long startAttemptTime = millis();

    // esperar hasta 10 segundos
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
      delay(500);
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nReconectado al WiFi!");
      digitalWrite(LED_BUILTIN, LOW); // prender LED
      Serial.print("IP asignada: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("\nNo se pudo reconectar al WiFi.");
      digitalWrite(LED_BUILTIN, HIGH); // apagar LED
    }
  }
#endif
