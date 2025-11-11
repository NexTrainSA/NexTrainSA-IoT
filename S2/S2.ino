#include <WiFi.h>
#include <PubSubClient.h>
#include "env.h"

// WiFi credentials
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// MQTT Broker
const char* mqtt_server = "179.155.211.130"; // Change to your MQTT broker
const int mqtt_port = 6883;
const char* mqtt_topic_out = "out/S2";
const char* mqtt_topic_in = "input/S2";


// DHT sensor
#define DHTTYPE DHT22
DHT dht(DHT_PIN, DHTTYPE);

// WiFi and MQTT clients
WiFiClient espClient;
PubSubClient client(espClient);

// Variables
float temperature = 0;
float humidity = 0;
int luminosity = 0;
int presence = 0;
bool ledState = false;
String ledRgbState = "Desligado";

unsigned long lastMsg = 0;
const long interval = 2000; // Send data every 2 seconds

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(message);

    if(message == "ping") {
      client.publish(mqtt_topic_out, "pong");
    }
    
    if (message == "TOGGLE_LED") {
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState ? HIGH : LOW);
      
      // Toggle RGB LED status
      if (ledState) {
        ledRgbState = "Ligado";
        digitalWrite(LED_R_PIN, HIGH);
        digitalWrite(LED_G_PIN, HIGH);
        digitalWrite(LED_B_PIN, HIGH);
      } else {
        ledRgbState = "Desligado";
        digitalWrite(LED_R_PIN, LOW);
        digitalWrite(LED_G_PIN, LOW);
        digitalWrite(LED_B_PIN, LOW);
      }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32_S1_";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str(), "nextrain", "nextrain")) {
      Serial.println("connected");
      client.subscribe(mqtt_topic_in);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void readSensors() {
  // Read DHT22
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  
  // Read LDR (photoresistor)
  int ldrValue = analogRead(LDR_PIN);
  luminosity = map(ldrValue, 0, 4095, 0, 1000); // Convert to lux approximation
  
  // Read PIR
  presence = digitalRead(PIR_PIN);
  
  // Check for DHT errors
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    temperature = 0;
    humidity = 0;
  }
}

void publishData() {
  StaticJsonDocument<300> doc;
  
  doc["TEMPERATURE"] = String(temperature, 1);
  doc["HUMIDITY"] = String(humidity, 1);
  doc["LUMINOSITY"] = String(luminosity);
  doc["PRESENCE"] = String(presence);
  doc["LED_STATE"] = ledState ? "Ligado" : "Desligado";
  doc["LED_RGB"] = ledRgbState;
  
  char jsonBuffer[300];
  serializeJson(doc, jsonBuffer);
  
  client.publish(mqtt_topic_out, jsonBuffer);
  
  Serial.println("Published data:");
  Serial.println(jsonBuffer);
}

void setup() {
  Serial.begin(115200);
  
  // Initialize pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_R_PIN, OUTPUT);
  pinMode(LED_G_PIN, OUTPUT);
  pinMode(LED_B_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  
  // Turn off all LEDs initially
  digitalWrite(LED_PIN, LOW);
  digitalWrite(LED_R_PIN, LOW);
  digitalWrite(LED_G_PIN, LOW);
  digitalWrite(LED_B_PIN, LOW);
  
  // Initialize DHT sensor
  dht.begin();
  
  // Connect to WiFi
  setup_wifi();
  
  // Configure MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  
  Serial.println("S1 Station initialized!");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;
    
    readSensors();
    publishData();
  }
}
