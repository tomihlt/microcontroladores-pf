#define WIFI
// #define NFC
#define KEYPAD
#define SSD1306
#define BUZZER

#ifdef WIFI
  #include <WiFi.h>
  #include <HTTPClient.h>
#endif
#ifdef NFC
  #include <Adafruit_PN532.h>
  #include <Wire.h>
#endif
#ifdef KEYPAD
  #include <Keypad.h>
#endif
#ifdef SSD1306
  #include <Adafruit_SSD1306.h>
  #include <Adafruit_GFX.h>
#endif

// Pines recomendados para GPIO
#define GPIO_NFC_SDA  21 // GPIO21 -> I2C SDA
#define GPIO_NFC_SCL  22 // GPIO22 -> I2C SCL
#define GPIO_PN532_IRQ 27 // GPIO27 -> OK para uso general
#define GPIO_PN532_RESET 26 // GPIO26 -> OK para uso general

#define GPIO_KEYPAD_R1  17 // GPIO13 -> OK para uso general
#define GPIO_KEYPAD_R2  5 // GPIO14 -> OK para uso general
#define GPIO_KEYPAD_R3  18 // GPIO15 -> OK para uso general
#define GPIO_KEYPAD_R4  19 // GPIO16 -> OK para uso general

#define GPIO_KEYPAD_C1  2 // GPIO25 -> OK para uso general
#define GPIO_KEYPAD_C2  0 // GPIO33 -> OK para uso general
#define GPIO_KEYPAD_C3  4 // GPIO32 -> OK para uso general
#define GPIO_KEYPAD_C4  16 // GPIO27 -> OK para uso general

#define LED_BUILTIN 2

#define BUZZER_PIN 25

//////////////////////////////////
// Configuración Pantall SSD1306
#ifdef SSD1306
  #define SCREEN_WIDTH 128
  #define SCREEN_HEIGHT 64
  #define OLED_RESET -1

  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#endif
//////////////////////////////////


//////////////////////////////////
// Configuración Keypad
#ifdef KEYPAD  
  const uint8_t ROWS = 4;
  const uint8_t COLS = 4;
  char keys[ROWS][COLS] = {
    { '1', '2', '3', 'A' },
    { '4', '5', '6', 'B' },
    { '7', '8', '9', 'C' },
    { '*', '0', '#', 'D' }
  };
  // Usando pines recomendados para ESP32
  uint8_t colPins[COLS] = { GPIO_KEYPAD_C1, GPIO_KEYPAD_C2, GPIO_KEYPAD_C3, GPIO_KEYPAD_C4 };
  uint8_t rowPins[ROWS] = { GPIO_KEYPAD_R1, GPIO_KEYPAD_R2, GPIO_KEYPAD_R3, GPIO_KEYPAD_R4};

  Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

  char szDni[15 + 1] = ""; // 15 digitos dni + '\0'
  uint8_t dniLen = 0;
#endif
//////////////////////////////////

//////////////////////////////////
// Configuración WiFi
#ifdef WIFI
  #define WIFI_SSID "fibertel wifi 817 2.4"
  #define WIFI_PASSWORD "barhaulet1"
  #define WIFI_CHANNEL 6
#endif
//////////////////////////////////

//////////////////////////////////
// Configuración del PN532
#ifdef NFC
  #define SDA_PIN GPIO_NFC_SDA  
  #define SCL_PIN GPIO_NFC_SCL  
  #define PN532_IRQ   GPIO_PN532_IRQ
  #define PN532_RESET GPIO_PN532_RESET
  Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET, &Wire);
#endif
//////////////////////////////////

#define ENDPOINT_URL_UID "http://192.168.0.187:8080/turnero"
#define ENDPOINT_URL_DNI "http://72.60.1.76:8080/api/attendance/nfc/9551674a19bae81d4d27f5436470c9ee6ecd0b371088686f6afc58d6bf68df30"

#ifdef SSD1306
  void printDisplay(String text, int textSize = 2) {
    display.clearDisplay();
    display.setTextSize(textSize);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(text);
    display.display();
  }

  void mostrarOk(){
    printDisplay("OK!", 4);
    #ifdef BUZZER
      digitalWrite(BUZZER_PIN, HIGH);
    #endif
    delay(500);
    printDisplay("");
    #ifdef BUZZER
      digitalWrite(BUZZER_PIN, LOW);
    #endif
  }

  void mostrarError(){
    printDisplay("ERROR", 4);
    #ifdef BUZZER
      digitalWrite(BUZZER_PIN, HIGH);
    #endif
    delay(1500);
    printDisplay("");
    #ifdef BUZZER
      digitalWrite(BUZZER_PIN, LOW);
    #endif
  }
#endif

void setup() {
  Serial.begin(115200);
  delay(500);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("== ESP32 BOOT ==");

  #ifdef BUZZER
    pinMode(BUZZER_PIN, OUTPUT);
  #endif

  #ifdef SSD1306
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
      // La mayoría de los módulos usan la dirección 0x3C
      Serial.println(F("SSD1306 no encontrado"));
    }
  #endif

  // Seteamos el internet
  #ifdef WIFI
    #ifdef SSD1306
      printDisplay("Conectando a internet...");
    #endif
    Serial.printf("Conectando a la red WiFi: %s\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // El ESP32 no necesita el canal en begin()

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("");
    Serial.printf("Conectado a %s\n", WIFI_SSID);
    Serial.print("IP asignada: ");
    #ifdef SSD1306
      printDisplay("Conectado a internet.", 2);
    #endif
    Serial.println(WiFi.localIP());
  #endif

  // Comenzar lectura NFC
  #ifdef NFC
    Serial.printf("Iniciando conexión con PN532\n");
    Wire.begin(SDA_PIN, SCL_PIN); // Usando los pines I2C definidos
    nfc.begin();

    uint32_t versiondata = nfc.getFirmwareVersion();
    if (!versiondata) {
      Serial.println("No se pudo encontrar PN532");
      while(1) delay(5000);
    }

    Serial.print("PN532 encontrado, firmware ver: 0x");
    Serial.println(versiondata, HEX);

    nfc.SAMConfig();
    Serial.println("Listo para leer llaveros NFC.");
  #endif

  digitalWrite(LED_BUILTIN, LOW);
  // En el ESP32, un LOW enciende el LED_BUILTIN.
}

void loop() {
  #if defined(NFC) && !defined(KEYPAD)
    uint8_t success;
    uint8_t uid[7];
    uint8_t uidLength;
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
    if (success) {
      Serial.print("UID leído: ");
      for (uint8_t i = 0; i < uidLength; i++) {
        Serial.print(uid[i], HEX);
        if (i < uidLength - 1) Serial.print(":");
      }
      #ifdef WIFI
        String szUid = uidToString(uid, uidLength);
        sendAttendanceUid(szUid.c_str());
      #endif
      Serial.println();
      delay(1000);
    }
  #elif !defined(NFC) && defined(KEYPAD)
    char key = keypad.getKey();
    if(key != NO_KEY) {

        if(key == '1' || key == '2' || key == '3' ||
        key == '4' || key == '5' || key == '6' ||
        key == '7' || key == '8' || key == '9' || key == '0'){
          szDni[dniLen] = key;
          dniLen++;
          szDni[dniLen] = '\0';
          Serial.println(szDni);
        }
        
        #ifdef SSD1306
          printDisplay(String(szDni));
        #endif
        if(dniLen == sizeof(szDni) - 1){
          Serial.print("Enviando dni al back... [");
          Serial.print(szDni);
          Serial.println("]");
          #ifdef WIFI
            sendAttendanceDni(szDni);
          #endif
          for(int i=0; i < sizeof(szDni); i++) szDni[i] = '\0';
          dniLen = 0;
        }else if(key == 'A'){
          Serial.print("Enviando dni al back... [");
          Serial.print(szDni);
          Serial.println("]");
          #ifdef WIFI
            sendAttendanceDni(szDni);
          #endif
          for(int i=0; i < sizeof(szDni); i++) szDni[i] = '\0';
          dniLen = 0;
        }else if(key == 'D'){
          for(int i=0; i < sizeof(szDni); i++) szDni[i] = '\0';
          dniLen = 0;
          #ifdef SSD1306
            printDisplay("");
          #endif
        }
    }
  #else
    // Codigo por si ninguno de los 2 está
  #endif
}

String uidToString(uint8_t *uid, uint8_t uidLength) {
  String uidString = "";
  for (uint8_t i = 0; i < uidLength; i++) {
    if (uid[i] < 0x10) uidString += "0";
    uidString += String(uid[i], HEX);
    if (i < uidLength - 1) uidString += ":";
  }
  uidString.toUpperCase();
  return uidString;
}

#ifdef WIFI
  void reconnectWiFi() {
    if (WiFi.status() == WL_CONNECTED) return;

    Serial.println("WiFi desconectado, intentando reconectar...");
    digitalWrite(LED_BUILTIN, HIGH);

    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
      delay(500);
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nReconectado al WiFi!");
      digitalWrite(LED_BUILTIN, LOW);
      Serial.print("IP asignada: ");
      Serial.println(WiFi.localIP());
      digitalWrite(LED_BUILTIN, LOW);
    } else {
      Serial.println("\nNo se pudo reconectar al WiFi.");
      digitalWrite(LED_BUILTIN, HIGH);
    }
  }

  void sendAttendanceUid(const char* uid) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Error: WiFi no conectado");
      reconnectWiFi();
      return;
    }

    WiFiClient client;
    HTTPClient http;

    http.begin(client, ENDPOINT_URL_UID);
    http.addHeader("Content-Type", "application/json");

    String jsonPayload = "{\"uid\":\"" + String(uid) + "\"}";
    Serial.print("Enviando attendance: ");
    Serial.println(jsonPayload);

    int httpCode = http.POST(jsonPayload);

    if (httpCode > 0) {
      Serial.printf("Código HTTP: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK) {
        String response = http.getString();
        Serial.print("Respuesta del servidor: ");
        Serial.println(response);
        #ifdef SSD1306
          mostrarOk();
        #endif
      } else {
        Serial.printf("Respuesta inesperada: %d\n", httpCode);
        String errorResponse = http.getString();
        Serial.print("Error response: ");
        Serial.println(errorResponse);
        #ifdef SSD1306
          mostrarError();
        #endif
      }
    } else {
      Serial.printf("Error de conexión: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
    delay(1000);
  }

  void sendAttendanceDni(const char* dni) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Error: WiFi no conectado");
      reconnectWiFi();
      return;
    }

    WiFiClient client;
    HTTPClient http;

    http.begin(client, ENDPOINT_URL_DNI);
    http.addHeader("Content-Type", "application/json");

    String jsonPayload = "{\"dni\":\"" + String(dni) + "\"}";
    Serial.print("Enviando attendance: ");
    Serial.println(jsonPayload);

    int httpCode = http.POST(jsonPayload);

    if (httpCode > 0) {
      Serial.printf("Código HTTP: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK) {
        String response = http.getString();
        Serial.print("Respuesta del servidor: ");
        Serial.println(response);
        #ifdef SSD1306
          mostrarOk();
        #endif
      } else {
        Serial.printf("Respuesta inesperada: %d\n", httpCode);
        String errorResponse = http.getString();
        Serial.print("Error response: ");
        Serial.println(errorResponse);
        #ifdef SSD1306
          mostrarError();
        #endif
      }
    } else {
      Serial.printf("Error de conexión: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
    delay(1000);
  }
#endif