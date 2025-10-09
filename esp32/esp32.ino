#include <Arduino.h>

// Directives for conditional compilation
#define WIFI
//#define NFC
#define KEYPAD
#define SSD1306
// #define BUZZER

// Include necessary libraries based on defined directives
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

// Pin definitions
#define GPIO_NFC_SDA 21
#define GPIO_NFC_SCL 22
#define GPIO_PN532_IRQ 27
#define GPIO_PN532_RESET 26

#define GPIO_KEYPAD_R1 17
#define GPIO_KEYPAD_R2 5
#define GPIO_KEYPAD_R3 18
#define GPIO_KEYPAD_R4 19

#define GPIO_KEYPAD_C1 2
#define GPIO_KEYPAD_C2 0
#define GPIO_KEYPAD_C3 4
#define GPIO_KEYPAD_C4 16

#define LED_BUILTIN 2
#define BUZZER_PIN 25

// API Endpoint URLs
#define ENDPOINT_URL_UID "http://192.168.0.187:8080/turnero"
#define ENDPOINT_URL_DNI "http://72.60.1.76:8080/api/attendance/nfc/9551674a19bae81d4d27f5436470c9ee6ecd0b371088686f6afc58d6bf68df30"

// --- Global Objects and Configurations ---

#ifdef SSD1306
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// SDA en GPIO21
// SDC en GPIO22
#endif

#ifdef KEYPAD
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

char szDni[16] = ""; // 15 digits + null terminator
uint8_t dniLen = 0;
#endif

#ifdef WIFI
#define WIFI_SSID "gutierrez"
#define WIFI_PASSWORD "claro5034"
#endif

#ifdef NFC
Adafruit_PN532 nfc(GPIO_PN532_IRQ, GPIO_PN532_RESET, &Wire);
#endif

// --- Utility Functions ---

#ifdef SSD1306
void printDisplay(const char* text, int textSize = 2) {
    display.clearDisplay();
    display.setTextSize(textSize);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(text);
    display.display();
}

void showStatus(bool success) {
    if (success) {
        printDisplay("OK!", 4);
        #ifdef BUZZER
        digitalWrite(BUZZER_PIN, HIGH);
        #endif
        delay(500);
    } else {
        printDisplay("ERROR", 4);
        #ifdef BUZZER
        digitalWrite(BUZZER_PIN, HIGH);
        #endif
        delay(1500);
    }
    printDisplay("");
    #ifdef BUZZER
    digitalWrite(BUZZER_PIN, LOW);
    #endif
}
#endif

#ifdef WIFI
void reconnectWiFi() {
    if (WiFi.status() == WL_CONNECTED) return;
    Serial.println("WiFi disconnected, attempting to reconnect...");
    digitalWrite(LED_BUILTIN, HIGH);
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
        delay(500);
        Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nReconnected to WiFi!");
        Serial.print("Assigned IP: ");
        Serial.println(WiFi.localIP());
        digitalWrite(LED_BUILTIN, LOW);
    } else {
        Serial.println("\nFailed to reconnect to WiFi.");
        digitalWrite(LED_BUILTIN, HIGH);
    }
}

void sendAttendance(const char* payload, const char* endpointUrl) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Error: WiFi not connected");
        reconnectWiFi();
        return;
    }
    HTTPClient http;
    http.begin(endpointUrl);
    http.addHeader("Content-Type", "application/json");
    Serial.printf("Sending payload: %s\n", payload);
    int httpCode = http.POST(payload);
    if (httpCode > 0) {
        Serial.printf("HTTP Code: %d\n", httpCode);
        if (httpCode == HTTP_CODE_OK) {
            String response = http.getString();
            Serial.printf("Server response: %s\n", response.c_str());
            #ifdef SSD1306
            showStatus(true);
            #endif
        } else {
            String errorResponse = http.getString();
            Serial.printf("Unexpected response: %d, Server error: %s\n", httpCode, errorResponse.c_str());
            #ifdef SSD1306
            showStatus(false);
            #endif
        }
    } else {
        Serial.printf("Connection error: %s\n", http.errorToString(httpCode).c_str());
        #ifdef SSD1306
        showStatus(false);
        #endif
    }
    http.end();
    delay(1000);
}

void sendAttendanceUid(const char* uid) {
    char jsonPayload[50]; // Increased buffer size for JSON payload
    snprintf(jsonPayload, sizeof(jsonPayload), "{\"uid\":\"%s\"}", uid);
    sendAttendance(jsonPayload, ENDPOINT_URL_UID);
}

void sendAttendanceDni(const char* dni) {
    char jsonPayload[50]; // Increased buffer size for JSON payload
    snprintf(jsonPayload, sizeof(jsonPayload), "{\"dni\":\"%s\"}", dni);
    sendAttendance(jsonPayload, ENDPOINT_URL_DNI);
}
#endif

void uidToString(uint8_t *uid, uint8_t uidLength, char* uidString, size_t bufferSize) {
    memset(uidString, 0, bufferSize); // Clear buffer
    for (uint8_t i = 0; i < uidLength; i++) {
        snprintf(uidString + strlen(uidString), bufferSize - strlen(uidString), "%02X", uid[i]);
        if (i < uidLength - 1) {
            strcat(uidString, ":");
        }
    }
}

// --- Main Program Flow ---

void setup() {
    Serial.begin(115200);
    delay(500);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("== ESP32 BOOT ==");

    #ifdef BUZZER
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    #endif

    #ifdef SSD1306
    Wire.begin(GPIO_NFC_SDA, GPIO_NFC_SCL);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 not found"));
    }
    #endif

    #ifdef WIFI
    #ifdef SSD1306
    printDisplay("Connecting to WiFi...");
    #endif
    Serial.printf("Connecting to WiFi network: %s\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi.");
    Serial.printf("IP assigned: %s\n", WiFi.localIP().toString().c_str());
    #ifdef SSD1306
    printDisplay("Connected to WiFi.", 2);
    #endif
    #endif

    #ifdef NFC
    Serial.printf("Initializing connection with PN532\n");
    Wire.begin(GPIO_NFC_SDA, GPIO_NFC_SCL);
    nfc.begin();
    uint32_t versiondata = nfc.getFirmwareVersion();
    if (!versiondata) {
        Serial.println("PN532 not found");
        while (1) delay(5000);
    }
    Serial.printf("PN532 found, firmware ver: 0x%X\n", versiondata);
    nfc.SAMConfig();
    Serial.println("Ready to read NFC keychains.");
    #endif

    digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
    #if defined(NFC) && !defined(KEYPAD)
    uint8_t uid[7];
    uint8_t uidLength;
    if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {
        char szUid[25]; // Buffer to hold the UID string
        uidToString(uid, uidLength, szUid, sizeof(szUid));
        Serial.printf("UID read: %s\n", szUid);
        #ifdef WIFI
        sendAttendanceUid(szUid);
        #endif
        delay(1000);
    }
    #elif !defined(NFC) && defined(KEYPAD)
    char key = keypad.getKey();
    if (key != NO_KEY) {
        if (isDigit(key) && dniLen < sizeof(szDni) - 1) {
            szDni[dniLen++] = key;
            szDni[dniLen] = '\0';
            Serial.println(szDni);
            #ifdef SSD1306
            printDisplay(szDni);
            #endif
        } else if (key == 'A' || dniLen >= sizeof(szDni) - 1) {
            if (dniLen > 0) {
                Serial.printf("Sending DNI to backend: [%s]\n", szDni);
                #ifdef WIFI
                sendAttendanceDni(szDni);
                #endif
            }
            dniLen = 0;
            szDni[0] = '\0';
            #ifdef SSD1306
            if (key != 'A') printDisplay("");
            #endif
        } else if (key == 'D') {
            dniLen = 0;
            szDni[0] = '\0';
            #ifdef SSD1306
            printDisplay("");
            #endif
        }
    }
    #else
    #endif
}