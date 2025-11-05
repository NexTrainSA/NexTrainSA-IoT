#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "env.h"

// Conexão Wi-Fi e MQTT
WiFiClientSecure wifiClient;
PubSubClient mqtt(wifiClient);

// === Pinos LED RGB ===
const int redPin = 2;
const int greenPin = 4;
const int bluePin = 6;

// === Sensores ultrassônicos ===
const int trig1 = 18;
const int echo1 = 19;

// === Tópicos MQTT ===
const String topic_led = "NexTrain/S2/Atuadores/LED_RGB";
const String topic_sensor = "NexTrain/S2/Sensores/Distancia";

// === Função para medir distância ===
float medirDistancia(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(5);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  long duracao = pulseIn(echo, HIGH);
  float distancia = duracao * 0.034 / 2.0; // cm
  return distancia;
}

// === Callback: executa ao receber mensagens MQTT ===
void callback(char* topic, byte* payload, unsigned int length) {
  String mensagem = "";
  for (int i = 0; i < length; i++) {
    mensagem += (char)payload[i];
  }

  Serial.print("Mensagem recebida no tópico [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(mensagem);

  // Controle do LED RGB via MQTT
  if (String(topic) == topic_led) {
    if (mensagem == "vermelho") {
      analogWrite(redPin, 255);
      analogWrite(greenPin, 0);
      analogWrite(bluePin, 0);
      Serial.println("LED: Vermelho");
    } 
    else if (mensagem == "verde") {
      analogWrite(redPin, 0);
      analogWrite(greenPin, 255);
      analogWrite(bluePin, 0);
      Serial.println("LED: Verde");
    } 
    else if (mensagem == "azul") {
      analogWrite(redPin, 0);
      analogWrite(greenPin, 0);
      analogWrite(bluePin, 255);
      Serial.println("LED: Azul");
    } 
    else if (mensagem == "branco") {
      analogWrite(redPin, 255);
      analogWrite(greenPin, 255);
      analogWrite(bluePin, 255);
      Serial.println("LED: Branco");
    } 
    else if (mensagem == "off") {
      analogWrite(redPin, 0);
      analogWrite(greenPin, 0);
      analogWrite(bluePin, 0);
      Serial.println("LED apagado");
    }
  }
}

void setup() {
  Serial.begin(115200);
  wifiClient.setInsecure();

  pinMode(trig1, OUTPUT);
  pinMode(echo1, INPUT);

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  // Conectar ao Wi-Fi
  Serial.println("Conectando ao WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  // Conectar ao broker MQTT
  mqtt.setServer(BROKER_URL, BROKER_PORT);
  mqtt.setCallback(callback);

  Serial.print("Conectando ao broker MQTT...");
  String clientID = "S2_" + String(random(0xffff), HEX);

  while (!mqtt.connected()) {
    if (mqtt.connect(clientID.c_str(), BROKER_USR_ID, BROKER_PASS_USR_PASS)) {
      Serial.println(" conectado!");
    } else {
      Serial.print(" falhou, rc=");
      Serial.print(mqtt.state());
      Serial.println(" tentando novamente em 2 segundos...");
      delay(2000);
    }
  }

  mqtt.subscribe(topic_led.c_str());
  Serial.println("Inscrito no tópico de LED!");
}

void loop() {
  if (!mqtt.connected()) {
    // Reconecta se cair
    String clientID = "S2_" + String(random(0xffff), HEX);
    while (!mqtt.connect(clientID.c_str(), BROKER_USR_ID, BROKER_PASS_USR_PASS)) {
      delay(2000);
    }
    mqtt.subscribe(topic_led.c_str());
  }

  mqtt.loop();

  // Leitura do sensor ultrassônico
  float distancia = medirDistancia(trig1, echo1);
  Serial.printf("Distância: %.2f cm\n", distancia);
  
  mqtt.publish(topic_sensor.c_str(), String(distancia).c_str());// Publica valor da distância

  delay(2000); // A cada 2 segundos
}
