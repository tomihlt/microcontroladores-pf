#include <Keypad.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "SPI.h"
#include <WiFi.h>
#include "HTTPClient.h"

// --- DEFINICIONES DE PINES (MANTENIDOS TAL CUAL) ---
#define GPIO_KEYPAD_R1 19
#define GPIO_KEYPAD_R2 18
#define GPIO_KEYPAD_R3 5
#define GPIO_KEYPAD_R4 17

#define GPIO_KEYPAD_C1 16
#define GPIO_KEYPAD_C2 4
#define GPIO_KEYPAD_C3 25
#define GPIO_KEYPAD_C4 26

// ATENCION: Hay conflictos de pines si se activan NFC y BUZZER.
// BUZZER_PIN (25) es el mismo que GPIO_KEYPAD_C3 (25).
// GPIO_PN532_RESET (26) es el mismo que GPIO_KEYPAD_C4 (26).
#define GPIO_NFC_SDA 21
#define GPIO_NFC_SCL 22
#define GPIO_PN532_IRQ 27
#define GPIO_PN532_RESET 26
#define BUZZER_PIN 25

// --- OBJETOS GLOBALES ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const uint8_t ROWS = 4;
const uint8_t COLS = 4;
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
uint8_t rowPins[ROWS] = {GPIO_KEYPAD_R1, GPIO_KEYPAD_R2, GPIO_KEYPAD_R3, GPIO_KEYPAD_R4};
uint8_t colPins[COLS] = {GPIO_KEYPAD_C1, GPIO_KEYPAD_C2, GPIO_KEYPAD_C3, GPIO_KEYPAD_C4};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// --- VARIABLES GLOBALES ---
#define WIFI_SSID "fibertel wifi 817 2.4"
#define WIFI_PASSWORD "barhaulet1"
#define ENDPOINT_URL_DNI "http://72.60.1.76:8080/api/attendance/nfc/9551674a19bae81d4d27f5436470c9ee6ecd0b371088686f6afc58d6bf68df30"

char szDni[11] = ""; // 15 digits + null terminator
uint8_t dniLen = 0;

// --- FUNCIONES DE LÓGICA DNI ---
void writeDni(char key) {
  if (dniLen < sizeof(szDni) - 1) {
    szDni[dniLen++] = key;
    szDni[dniLen] = '\0';
  }
}

void deleteDni() {
  szDni[0] = '\0';
  dniLen = 0;
}

// --- FUNCIONES DE PANTALLA MEJORADAS ---
void updateDniDisplay() {
  // Limpia solo el área donde se escribe el DNI
  display.fillRect(0, 25, SCREEN_WIDTH, 30, SSD1306_BLACK);
  // Escribe el DNI actual
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 30);
  display.println(szDni);
  display.display();
}

void drawMainLayout() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 5);
  display.println("Ingrese DNI:");
  display.display();
}

void showStatus(bool success) {
  display.clearDisplay();
  display.setTextSize(3);
  display.setCursor(35, 20); // Centrado
  if (success) {
    display.setTextColor(SSD1306_WHITE);
    display.println("OK!");
  } else {
    display.setTextColor(SSD1306_WHITE);
    display.println("ERROR");
  }
  display.display();
  delay(success ? 400 : 900); // <-- DELAY REDUCIDO

  // Después de mostrar el estado, vuelve a la pantalla principal
  drawMainLayout();
  updateDniDisplay();
}

// --- FUNCIÓN DE RED ---
void sendAttendanceDni(const char* dni) {
  if (WiFi.status() != WL_CONNECTED) {
    showStatus(false);
    return;
  }
  HTTPClient http;
  http.begin(ENDPOINT_URL_DNI);
  http.addHeader("Content-Type", "application/json");

  char jsonPayload[50];
  snprintf(jsonPayload, sizeof(jsonPayload), "{\"dni\":\"%s\"}", dni);

  int httpCode = http.POST(jsonPayload);
  
  if (httpCode == HTTP_CODE_OK) {
    showStatus(true);
  } else {
    showStatus(false);
  }
  
  http.end();
  deleteDni(); // Limpia el DNI después de enviar
}

// --- PROGRAMA PRINCIPAL ---
void setup() {
  Serial.begin(115200);
  Serial.println("== ESP32 BOOT ==");

  // --- CORRECCIÓN CRÍTICA PARA EVITAR "TECLAS FANTASMA" ---
  for (int i = 0; i < COLS; i++) {
    pinMode(colPins[i], INPUT_PULLUP);
  }

  // Inicializa la pantalla
  Wire.begin(); // Usa los pines I2C por defecto (21, 22)
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 no encontrado"));
  }

  // Conexión a WiFi
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.println("Conectando a WiFi...");
  display.display();
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado a WiFi.");
  
  // Dibuja la interfaz principal por primera vez
  drawMainLayout();
  updateDniDisplay();
}

void loop() {
  char key = keypad.getKey();
  if (key != NO_KEY) {
    if (isdigit(key)) {
      writeDni(key);
      updateDniDisplay();
    } else if (key == 'D') {
      deleteDni();
      updateDniDisplay();
    } else if (key == 'A') {
      if (dniLen > 0) {
        sendAttendanceDni(szDni);
      }
    }
  }
}