# Sistema de Control con ESP8266 + PN532 (NFC)

Este proyecto utiliza un **ESP8266** conectado a un lector **PN532 NFC** y a una red WiFi para registrar asistencias o accesos mediante el envío de los UID de las tarjetas/llaveros NFC a un servidor vía **HTTP POST**.

---

## 📌 Características principales
- Lectura de tarjetas **NFC** con el módulo **PN532**.
- Conexión automática a red WiFi (con rutina de reconexión en caso de caída).
- Envío del UID al servidor mediante **HTTP POST** en formato JSON.
- Indicación del estado WiFi con el LED integrado del ESP8266.
- Código escrito para el entorno **Arduino IDE**.

---

## 🛠️ Dependencias
Instalar las siguientes librerías desde el **Arduino IDE Library Manager**:

- `ESP8266WiFi`
- `ESP8266HTTPClient`
- `Adafruit PN532`
- `Wire`

---

## 🔌 Conexiones físicas

El PN532 está conectado al **ESP8266** en modo **I2C**:

| PN532 | ESP8266 (NodeMCU) |
|-------|------------------|
| SDA   | D2 (GPIO4)       |
| SCL   | D1 (GPIO5)       |
| VCC   | 3.3V             |
| GND   | GND              |
| IRQ   | D4 (GPIO2)       | No se usa cable
| RST   | D3 (GPIO0)       | No se usa cable

📷 Diagrama de conexión:

![Diagrama de conexión](./esp8266/diagrama%20de%20conexión.png)

---

## ⚙️ Configuración
En el archivo principal (`.ino`) modificar:

```cpp
#define WIFI_SSID "TU_SSID"
#define WIFI_PASSWORD "TU_PASSWORD"
#define ENDPOINT_URL "http://<IP_SERVIDOR>:8080/turnero"
