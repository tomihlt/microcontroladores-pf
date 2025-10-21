#include <Keypad.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "SPI.h"
#include <WiFi.h>
#include "HTTPClient.h"

#define GPIO_KEYPAD_R1 19
#define GPIO_KEYPAD_R2 18
#define GPIO_KEYPAD_R3 5
#define GPIO_KEYPAD_R4 17

#define GPIO_KEYPAD_C1 16
#define GPIO_KEYPAD_C2 4
#define GPIO_KEYPAD_C3 23
#define GPIO_KEYPAD_C4 22

#define TFT_CS 15
#define TFT_RST 2
#define TFT_DC 27

#define TFT_MOSI 13
#define TFT_SCK 14
#define TFT_MISO 12

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PW ""

#define ENDPOINT_URL_DNI "http://72.60.1.76:8080/api/attendance/nfc/9551674a19bae81d4d27f5436470c9ee6ecd0b371088686f6afc58d6bf68df30"

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

//////////////////////
// KEYPAD 
const uint8_t ROWS = 4;
const uint8_t COLS = 4;
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};
uint8_t rowPins[ROWS] = {GPIO_KEYPAD_R1, GPIO_KEYPAD_R2, GPIO_KEYPAD_R3, GPIO_KEYPAD_R4};
uint8_t colPins[COLS] = {GPIO_KEYPAD_C1, GPIO_KEYPAD_C2, GPIO_KEYPAD_C3, GPIO_KEYPAD_C4};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

//////////////////////
// DNI
char szDni[13] = ""; // hasta 12 dígitos
uint8_t dniLen = 0;
char feedback[32] = "Feedback"; // Mensaje inicial

void deleteDni() {
  szDni[0] = '\0';
  dniLen = 0;
}

void writeDni(char key) {
  if (dniLen < sizeof(szDni) - 1) {
    szDni[dniLen++] = key;
    szDni[dniLen] = '\0';
  }
}

void deleteLastLetter() {
  if (dniLen > 0) {
    dniLen--;
    szDni[dniLen] = '\0';
  }
}

//////////////////////
// DIBUJO DE PANTALLA
void drawLayout() {
  tft.fillScreen(ILI9341_BLACK);
  tft.setRotation(1);

  // Recuadro TÍTULO
  tft.drawRect(40, 30, 240, 50, ILI9341_WHITE);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(75, 50);
  tft.println("INGRESE SU DNI");

  // Recuadro DNI
  tft.drawRect(40, 100, 240, 50, ILI9341_WHITE);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(1);
  tft.setCursor(80, 120);
  tft.println("<ACA SE ESCRIBE EL DNI>");

  // Recuadro FEEDBACK
  tft.drawRect(40, 170, 240, 50, ILI9341_WHITE);
  tft.setTextColor(ILI9341_CYAN);
  tft.setTextSize(1);
  tft.setCursor(60, 190);
  tft.println("<ACA SE ESCRIBE EL FEEDBACK>");
}

void updateDni() {
  // Limpia solo el área del DNI
  tft.fillRect(45, 105, 230, 40, ILI9341_BLACK);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(3);
  tft.setCursor(60, 115);
  tft.println(szDni);
}

void updateFeedback(int textSize = 1) {
  // Limpia el área del feedback
  tft.fillRect(45, 175, 230, 40, ILI9341_BLACK);
  tft.setTextColor(ILI9341_CYAN);
  tft.setTextSize(textSize);
  tft.setCursor(60, 190);
  tft.println(feedback);
}

void clearScreen(uint16_t color = ILI9341_BLACK) {
  tft.fillScreen(color);
}

void printCenteredMessage(const char* message, int textSize = 4, uint16_t color = ILI9341_BLUE) {
  // 1. Establece el tamaño de la fuente ANTES de medir.
  tft.setTextSize(textSize);

  // 2. Variables para almacenar las dimensiones del texto.
  int16_t x1, y1;
  uint16_t w, h;

  // 3. Calcula el ancho (w) y alto (h) del texto.
  tft.getTextBounds(message, 0, 0, &x1, &y1, &w, &h);

  // 4. Calcula las coordenadas X e Y para que el texto quede centrado.
  int16_t cursorX = (tft.width() - w) / 2;
  int16_t cursorY = (tft.height() - h) / 2;

  // 5. Establece el cursor, el color y finalmente imprime el mensaje.
  tft.setCursor(cursorX, cursorY);
  tft.setTextColor(color);
  tft.println(message);
}

/**
 * @brief Verifica el estado de la conexión y muestra la información en pantalla.
 */
void showWifiInfo() {
  clearScreen(); // Limpia la pantalla
  tft.setRotation(1);

  // Comprueba si el dispositivo está conectado a una red Wi-Fi
  if (WiFi.status() == WL_CONNECTED) {
    // --- SI ESTÁ CONECTADO, MUESTRA LA INFORMACIÓN ---

    // Dibuja un título grande
    tft.setCursor(20, 30);
    tft.setTextColor(ILI9341_GREEN);
    tft.setTextSize(3);
    tft.println("Conectado!");

    // Prepara para dibujar la información
    tft.setTextSize(2);

    // Muestra el nombre de la red (SSID)
    tft.setTextColor(ILI9341_WHITE);
    tft.setCursor(20, 80);
    tft.print("Red: ");
    tft.setTextColor(ILI9341_YELLOW);
    tft.println(WiFi.SSID());

    // Muestra la dirección IP asignada
    tft.setTextColor(ILI9341_WHITE);
    tft.setCursor(20, 110);
    tft.print("IP: ");
    tft.setTextColor(ILI9341_YELLOW);
    tft.println(WiFi.localIP());

    // Muestra la intensidad de la señal
    tft.setTextColor(ILI9341_WHITE);
    tft.setCursor(20, 140);
    tft.print("Senal: ");
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(WiFi.RSSI());
    tft.println(" dBm");

  } else {
    // --- SI NO ESTÁ CONECTADO, MUESTRA UN MENSAJE DE ERROR ---
    printCenteredMessage("No hay conexion\n a WiFi", 3, ILI9341_RED);
  }

  // Espera 3 segundos para que el usuario pueda leer la información
  delay(3000);
}

void sendAttendanceDni(const char* dni) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Error: WiFi no conectado");
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
        strcpy(feedback, "Asistencia registrada");
        updateFeedback();
      } else {
        Serial.printf("Respuesta inesperada: %d\n", httpCode);
        String errorResponse = http.getString();
        Serial.print("Error response: ");
        Serial.println(errorResponse);
        strcpy(feedback, "Ocurrió un error inesperado");
        updateFeedback();
      }
    } else {
      Serial.printf("Error de conexión: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();

    delay(2500);
    deleteDni();
    strcpy(feedback, "");
    updateDni();
    updateFeedback();
    delay(1000);
  }

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("ESP32 + ILI9341 listo");
  SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, -1);
  tft.begin();

  clearScreen();
  tft.setRotation(1);
  printCenteredMessage("Conectando a la red WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PW);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("."); // Imprime un punto para mostrar que está trabajando
  }

  clearScreen();
  printCenteredMessage("Conectado");
  showWifiInfo();

  clearScreen();
  drawLayout();
}

void loop() {
  char key = keypad.getKey();
  if (key != NO_KEY) {
    if (key == 'D') {
      deleteDni();
      updateDni();
    } else if (key == 'B') {
      deleteLastLetter();
      updateDni();
    } else if (key == '*') {
      showWifiInfo();
      drawLayout();
      updateDni();
      updateFeedback();
    } else if (isdigit(key)) {
      writeDni(key);
      updateDni();
      Serial.println(szDni);
    } else if (key == 'A') {
      sendAttendanceDni(szDni);
    }
  }
}
