📌 IoT Edge Protocol: ESP32 Intelligent Gateway

Este repositorio contiene la implementación de un protocolo de comunicación robusto diseñado para dispositivos ESP32 en entornos de IoT. El proyecto se centra en la eficiencia del consumo de datos, la gestión de la latencia y la resiliencia de la conexión en redes inestables, permitiendo la telemetría y el control de actuadores en tiempo real.

---

🏗️ Arquitectura del Sistema

El sistema utiliza una arquitectura de capas para separar la lógica de adquisición de datos de la lógica de transmisión:

- Hardware Layer: Gestión de GPIOs, ADC y buses de comunicación (I2C/SPI).

- Protocol Layer: Serialización de datos en formato JSON/MessagePack.

- Communication Layer: Gestión de stack TCP/IP y protocolos de transporte (MQTT/HTTP).

- ecurity Layer: Implementación de TLS y autenticación por Token.

---

🔌 Pinout y Conectividad

- Sensor de Red: PIN GPIO (32) ; Tipo Input, Protocolo Analog

- Actuador Principal

- Display OLED

- Debug Console

---

📡 Especificación del Protocolo

El protocolo está optimizado para minimizar el payload y asegurar la entrega de mensajes críticos.

---

🚀 Instalación y Despliegue

- Visual Studio Code + PlatformIO (recomendado) o Arduino IDE.

- Toolchain de Espressif ESP32 v2.0.x.

- Librerías: ArduinoJson, PubSubClient, WiFiClientSecure.

---

🚀 Pasos para Ejecutarlo

- Git clone

- Configurar Credenciales en include/Secrets.h o Config.h

- Compiar y Cargar el device : Pio Run --Target Upload

---

📄 Licencia
Este proyecto está bajo la Licencia MIT. Consulte el archivo LICENSE para más detalles.