#include "env.h"

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
// Pin definitions
#define LED_PIN 2

const int PINO_TRIG = 4; // Pino D4 conectado ao TRIG do HC-SR04
const int PINO_ECHO = 5; // Pino D2 conectado ao ECHO do HC-SR04

const int PINO_TRIG2 = 16; // Pino D16 conectado ao TRIG do HC-SR04
const int PINO_ECHO2 = 17; // Pino D17 conectado ao ECHO do HC-SR04

const int PINO_R = 18;
const int PINO_G = 19;
const int PINO_B = 21;

String currentColor = "NONE";

// WiFi and MQTT clients
WiFiClient espClient;
PubSubClient client(espClient);

// Variables
float presence2 = 0;
float presence4 = 0;
bool ledState = false;
String ledRgbState = "Desligado";
unsigned long lastMsg = 0;
const long interval = 2000; // Send data every 2 seconds

String rgbToHex(int r, int g, int b)
{  

  long colorValue = ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF);
  char hexString[7];
  snprintf(hexString, sizeof(hexString), "%06lX", colorValue);
  return String(hexString);
}



void setColor(int redValue, int greenValue,  int blueValue) {
  currentColor = rgbToHex(redValue, greenValue, blueValue);

  analogWrite(PINO_R, redValue);
  analogWrite(PINO_G,  greenValue);
  analogWrite(PINO_B, blueValue);
}

void setColor(String hex) {
  if (hex.startsWith("#")) {
    hex.remove(0, 1);
  }
  long colorValue = strtol(hex.c_str(), NULL, 16);

  int r = (colorValue >> 16) & 0xFF;
  int g = (colorValue >> 8) & 0xFF;
  int b = colorValue & 0xFF;
  analogWrite(PINO_R, r);
  analogWrite(PINO_G,  g);
  analogWrite(PINO_B, b);
}



void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);
  WiFi.begin(SSID, password);
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
  }

  if (message == "GREEN") {
    setColor(0,255,0);
  }

  if (message == "BLUE") {
    setColor(0,0,255);
  }

  if (message == "RED") {
    setColor(255,0,0);
  }

  if(message.startsWith("color/")) {
    String color = message.substring(7);
    Serial.println("Color: " + color);
    setColor(color);
  }
}



void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32_S2_";
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
  digitalWrite(PINO_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PINO_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PINO_TRIG, LOW);
  long duracao1 = pulseIn(PINO_ECHO, HIGH);
  presence2 = (duracao1 * 0.0343) / 2;
  delay(20); 
  digitalWrite(PINO_TRIG2, LOW);
  delayMicroseconds(2);
  digitalWrite(PINO_TRIG2, HIGH);
  delayMicroseconds(10);
  digitalWrite(PINO_TRIG2, LOW);
  long duracao2 = pulseIn(PINO_ECHO2, HIGH);
  presence4 = (duracao2 * 0.0343) / 2;
}



void publishData() {
  StaticJsonDocument<300> doc;

  doc["PRESENCE2"] = String(presence2);
  doc["PRESENCE4"] = String(presence4);
  doc["LED_STATE"] = ledState ? "Ligado" : "Desligado";
  doc["LED_RGB"] = currentColor;

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
  pinMode(PINO_TRIG, OUTPUT);
  pinMode(PINO_ECHO, INPUT);
  pinMode(PINO_TRIG2, OUTPUT);
  pinMode(PINO_ECHO2, INPUT);
  pinMode(PINO_R,  OUTPUT);              
  pinMode(PINO_G, OUTPUT);
  pinMode(PINO_B, OUTPUT);

  // Turn off all LEDs initially
  digitalWrite(LED_PIN, LOW);
  digitalWrite(PINO_R, LOW);
  digitalWrite(PINO_G, LOW);
  digitalWrite(PINO_B, LOW);
  // Connect to WiFi
  setup_wifi();

  // Configure MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  Serial.println("S2 Station initialized!");
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
