#include <Keypad.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "SPI.h"

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

//////////////////////
///// ILI9341 
// SPIClass spi = SPIClass(VSPI);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

//////////////////////
///// KEYPAD 
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

//////////////////////
///// DNI

char szDni[16] = ""; // 15 digitos DNI
uint8_t dniLen = 0;

void deleteDni(){
  szDni[0] = '\0';
  dniLen = 0;
}

void writeDni(char key) {
  szDni[dniLen] = key;
  dniLen++;
  szDni[dniLen] = '\0';
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello, ESP32!");

  // spi.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS);
  SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, -1);
  tft.begin();

  tft.fillScreen(ILI9341_BLACK);
  tft.setRotation(1);

  // Dibuja el estado inicial de la pantalla UNA SOLA VEZ
  tft.setCursor(26, 120);
  tft.setTextColor(ILI9341_RED);
  tft.setTextSize(3);
  tft.println(szDni); // Muestra el DNI vacío al principio
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(10); // this speeds up the simulation
  char key = keypad.getKey();
  if (key != NO_KEY) {
    if(key == 'A') Serial.println(key);
    else if (key == 'D') deleteDni();
    else if(isdigit(key)){
      writeDni(key);
      Serial.println(szDni);
      if((sizeof(szDni) - 1) == dniLen) {
        Serial.println("Enviando dni...");
        delay(1000);
        deleteDni();
      }
    }
  }

  // Ahora imprime el nuevo número
  tft.fillRect(20, 100, 280, 40, ILI9341_BLACK); // Borra el área del número anterior
  tft.setCursor(26, 120);
  tft.setTextColor(ILI9341_RED);
  tft.setTextSize(3);
  tft.setCursor(26, 120);
  tft.println(szDni);
}
