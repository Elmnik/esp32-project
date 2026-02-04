# 📌 IoT Edge Protocol: ESP32 Intelligent Gateway

Este repositorio contiene la implementación de un protocolo de comunicación robusto diseñado para dispositivos **ESP32** en entornos de IoT. El proyecto se centra en la eficiencia del consumo de datos, la gestión de la latencia y la resiliencia de la conexión en redes inestables, permitiendo la telemetría y el control de actuadores en tiempo real.

---

## 🏗️ Arquitectura del Sistema

El sistema utiliza una arquitectura de capas para asegurar la escalabilidad y mantenibilidad:

| Capa | Responsabilidad |
| :--- | :--- |
| **Hardware Layer** | Gestión de GPIOs, ADC y buses de comunicación (I2C/SPI). |
| **Protocol Layer** | Serialización de datos en formato **JSON/MessagePack**. |
| **Communication Layer** | Gestión de stack TCP/IP y protocolos de transporte (**MQTT/HTTP**). |
| **Security Layer** | Implementación de **TLS** y autenticación por Token. |

---

## 🔌 Pinout y Conectividad

Configuración base para el hardware del gateway:

* **Sensor de Red:** GPIO (32) | Tipo: *Input* | Protocolo: *Analog*
* **Actuador Principal:** [Definir PIN]
* **Display OLED:** I2C (SDA/SCL)
* **Debug Console:** UART @ 115200 baudios

---

## 📡 Especificación del Protocolo
El protocolo está optimizado para minimizar el *payload* y asegurar la entrega de mensajes críticos mediante mecanismos de QoS y validación de integridad.

---

## 🛠️ Requisitos Técnicos

* **IDE:** Visual Studio Code + [PlatformIO](https://platformio.org/) (recomendado).
* **Core:** Toolchain de Espressif ESP32 v2.0.x.
* **Librerías Clave:** * `ArduinoJson`
    * `PubSubClient`
    * `WiFiClientSecure`

---

## 🚀 Instalación y Despliegue

1.  **Clonar el repositorio:**
    ```bash
    git clone [https://github.com/Elmnik/esp32-project.git](https://github.com/Elmnik/esp32-project.git)
    ```
2.  **Configurar Credenciales:**
    Edita el archivo `include/Secrets.h` (basado en el ejemplo) con tus credenciales de Red y MQTT.
3.  **Compilar y Cargar:**
    ```bash
    pio run --target upload
    ```

---

## 📄 Licencia
Este proyecto está bajo la **Licencia MIT**. Consulte el archivo `LICENSE` para más detalles.