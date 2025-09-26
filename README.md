# ESP32 Access System (WiFi + SSD1306 + Keypad 4x4 + NFC opcional)

Este proyecto implementa un sistema de acceso utilizando un **ESP32**, con interfaz en una pantalla **OLED SSD1306**, un **teclado matricial 4x4** para ingreso de códigos, conexión a **WiFi** y soporte opcional para un lector **NFC**.

---

## 🔧 Hardware necesario

- **ESP32** (cualquier modelo con WiFi integrado)
- **Pantalla OLED SSD1306** (I²C recomendado, 128x64)
- **Keypad matricial 4x4**
- **Módulo NFC opcional** (ej: MFRC522 o PN532)
- Fuente de alimentación 5V / USB
- Protoboard y cables Dupont

---

## 📌 Conexiones principales

### Pantalla SSD1306 (I²C, 4 pines)
- VCC → 3.3V
- GND → GND
- SDA → GPIO21 (configurable)
- SCL → GPIO22 (configurable)

### Keypad 4x4
- 8 pines (4 filas + 4 columnas) conectados a GPIOs libres del ESP32  
- Los pines específicos se definen en el código (`esp32.ino`).

### NFC (opcional)
- Depende del módulo usado:
  - **MFRC522 (SPI):**
    - SDA → GPIO5
    - SCK → GPIO18
    - MOSI → GPIO23
    - MISO → GPIO19
    - RST → GPIO22 (configurable)
  - **PN532 (I²C o SPI):** ajustar pines según configuración.

---

## 📡 Conexión WiFi

El ESP32 se conecta a tu red WiFi local.  
En el archivo `esp32.ino` deberás modificar estas líneas con tus credenciales:

```cpp
const char* ssid = "TU_SSID";
const char* password = "TU_PASSWORD";
