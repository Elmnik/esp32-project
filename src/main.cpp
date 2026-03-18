/**
 * @file main.cpp
 * @brief SICC - Finca La Fátima (Edición Resiliente)
 * @updates: Re-conector WiFi, Validación NaN, y LWT (Last Will).
 */

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "DHT.h"

// --- Configuración de Red y MQTT ---
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "broker.emqx.io";

// --- Configuración de Pines ---
#define DHTPIN 15          
#define SOIL_ADC 32        
#define RELAY_PUMP 18      
#define RELAY_FAN 19       

// --- Instancias ---
DHT dht(DHTPIN, DHT22);
WiFiClient espClient;
PubSubClient client(espClient);

// --- Variables Globales ---
unsigned long lastMsg = 0;
float t_aire = 0, h_aire = 0, porc_humedad_suelo = 0;

// --- Prototipos de Funciones ---
void setup_wifi();
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();
void loopControlAguacate();
void publishTelemetry();

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PUMP, OUTPUT);
  pinMode(RELAY_FAN, OUTPUT);
  
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  // Verificación constante de WiFi
  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }

  // Verificación de conexión MQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    loopControlAguacate();
    publishTelemetry();
  }
}

void setup_wifi() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.println("\nConectando a WiFi...");
  WiFi.begin(ssid, password);
  
  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 20) {
    delay(500);
    Serial.print(".");
    intentos++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi conectado - IP: " + WiFi.localIP().toString());
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) message += (char)payload[i];
  
  Serial.println("Comando remoto recibido [" + String(topic) + "]: " + message);

  // La lógica de sensores sobreescribirá esto en el siguiente ciclo)
  if (String(topic) == "finca/fatima/control/riego") {
    digitalWrite(RELAY_PUMP, (message == "ON") ? HIGH : LOW);
  }
  if (String(topic) == "finca/fatima/control/ventilacion") {
    digitalWrite(RELAY_FAN, (message == "ON") ? HIGH : LOW);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    String clientId = "SICC-Fatima-" + String(random(0xffff), HEX);
    
    // Conexión con "Last Will" para monitorear si el equipo se desconecta bruscamente
    if (client.connect(clientId.c_str(), "finca/fatima/estado", 0, true, "OFFLINE")) {
      Serial.println("Conectado");
      client.publish("finca/fatima/estado", "ONLINE", true);
      client.subscribe("finca/fatima/control/#");
    } else {
      Serial.print("falló, rc=");
      Serial.print(client.state());
      delay(5000);
      if (WiFi.status() != WL_CONNECTED) return; 
    }
  }
}

void loopControlAguacate() {
    int h_suelo_raw = analogRead(SOIL_ADC);
    float t_temp = dht.readTemperature();
    float h_temp = dht.readHumidity();
    
    // Validación de sensor
    if (isnan(t_temp) || isnan(h_temp)) {
        Serial.println("⚠️ Error de lectura en DHT22!");
        return; 
    }

    t_aire = t_temp;
    h_aire = h_temp;
    porc_humedad_suelo = map(h_suelo_raw, 4095, 0, 0, 100);

    // Lógica de Histéresis 
    if (porc_humedad_suelo < 60.0) digitalWrite(RELAY_PUMP, HIGH); // Si la humedad del suelo es menor al 60%, activamos el riego
    else if (porc_humedad_suelo > 75.0) digitalWrite(RELAY_PUMP, LOW); // Si la humedad del suelo es mayor al 75%, desactivamos el riego

    if (t_aire > 26.0) digitalWrite(RELAY_FAN, HIGH); // Si la temperatura del aire es mayor a 26°C, activamos el ventilador
    else if (t_aire < 23.0) digitalWrite(RELAY_FAN, LOW); // Si la temperatura del aire es menor a 23°C, desactivamos el ventilador
}

void publishTelemetry() {
    StaticJsonDocument<256> doc;
    doc["temp"] = t_aire;
    doc["hum_aire"] = h_aire;
    doc["hum_suelo"] = porc_humedad_suelo;
    doc["bomba"] = digitalRead(RELAY_PUMP);
    doc["ventilador"] = digitalRead(RELAY_FAN);
    doc["wifi_rssi"] = WiFi.RSSI();

    char buffer[256];
    serializeJson(doc, buffer);
    client.publish("finca/fatima/telemetria", buffer);
    Serial.println("Telemetría enviada: " + String(buffer));
}

/* Hemos configurado el ESP32 para gestionar un ambiente controlado. 
El LED Verde actúa como un actuador de climatización: 
si la temperatura sube de 24°C, el sistema activa el enfriamiento. 
Por otro lado, el LED Azul gestiona el nivel de hidratación: 
si la humedad cae por debajo del 40%, se activa el sistema de riego (LeD Azul) */

/* Además del control de temperatura y humedad, 
incluímos una medida de seguridad. Ese componente que parece una báscula 
mide el peso del tanque de agua. Si el peso baja de cierto nivel (cuando muevo la perilla),
 el LED Azul se enciende para avisar que, aunque la planta necesite riego, 
 el tanque está vacío y hay que rellenarlo */