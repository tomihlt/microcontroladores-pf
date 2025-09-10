#include <Wire.h>
#include <Adafruit_PN532.h>

#define SDA_PIN 4
#define SCL_PIN 5

Adafruit_PN532 nfc(SDA_PIN, SCL_PIN);

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.println("No se pudo encontrar PN532");
    while (1);
  }

  Serial.print("PN532 Firmware Version: 0x");
  Serial.println(versiondata, HEX);

  nfc.SAMConfig();
  Serial.println("Listo para leer NFC");
}

void loop() {
  uint8_t uid[7];
  uint8_t uidLength;
  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {
    Serial.print("UID: ");
    for (uint8_t i = 0; i < uidLength; i++) {
      Serial.print(uid[i], HEX);
      if (i < uidLength - 1) Serial.print(":");
    }
    Serial.println();
  }
  delay(1000);
}
