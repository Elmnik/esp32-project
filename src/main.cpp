/**
 * @file main.cpp
 * @brief Sistema Inteligente de Control Climático (SICC) - Finca La Fátima
 * @framework PlatformIO - Arduino
 */

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "DHT.h"

// --- Configuración de Red y MQTT ---
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "broker.emqx.io"; // Broker público para pruebas

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
float t_aire = 0;
float h_aire = 0;
float porc_humedad_suelo = 0;

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
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 5000) { // Cada 5 segundos
    lastMsg = now;
    loopControlAguacate();
    publishTelemetry();
  }
}

void setup_wifi() {
  delay(10);
  Serial.println("\nConectando a WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado - IP: " + WiFi.localIP().toString());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) message += (char)payload[i];
  
  Serial.println("Mensaje recibido [" + String(topic) + "]: " + message);

  if (String(topic) == "finca/fatima/control/riego") {
    digitalWrite(RELAY_PUMP, (message == "ON") ? HIGH : LOW);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    String clientId = "SICC-Fatima-Client-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("Conectado");
      client.subscribe("finca/fatima/control/#");
    } else {
      Serial.print("falló, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

void loopControlAguacate() {
    int h_suelo_raw = analogRead(SOIL_ADC);
    t_aire = dht.readTemperature();
    h_aire = dht.readHumidity();
    
    porc_humedad_suelo = map(h_suelo_raw, 4095, 0, 0, 100);

    // Lógica de Histéresis
    if (porc_humedad_suelo < 60.0) digitalWrite(RELAY_PUMP, HIGH);
    else if (porc_humedad_suelo > 75.0) digitalWrite(RELAY_PUMP, LOW);

    if (t_aire > 26.0) digitalWrite(RELAY_FAN, HIGH);
    else if (t_aire < 23.0) digitalWrite(RELAY_FAN, LOW);
}

void publishTelemetry() {
    StaticJsonDocument<200> doc;
    doc["temp"] = t_aire;
    doc["hum_aire"] = h_aire;
    doc["hum_suelo"] = porc_humedad_suelo;
    doc["bomba"] = digitalRead(RELAY_PUMP);
    doc["ventilador"] = digitalRead(RELAY_FAN);

    char buffer[200];
    serializeJson(doc, buffer);
    client.publish("finca/fatima/telemetria", buffer);
    Serial.println("Datos publicados: " + String(buffer));
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