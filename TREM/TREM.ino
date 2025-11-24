#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "env.h"

WiFiClientSecure wifiClient;
PubSubClient mqtt(wifiClient);

const byte dir1 = 33;  // Mudar depois se necessário
const byte dir2 = 25;

// Pinos LED RGB:
const byte vermelhinho = 5;
const byte verdinho = 18;
const byte azulzinho = 19;

// PWM (ledc) channels e configuração
const uint8_t CH_R = 0;
const uint8_t CH_G = 1;
const uint8_t CH_B = 2;
const uint32_t PWM_FREQ = 5000;
const uint8_t PWM_RES = 8; 

void statusLED(byte status);
void turnOffLEDs();
void setLEDColor(byte r, byte g, byte b);
void callback(char* topic, byte* payload, unsigned int length);
void connectToWifi();
void connectToMQTT();

// Controlando status do LED:
void statusLED(byte status) {
  turnOffLEDs();
  switch (status) {
    case 254:  // Erro (Vermelho)
      setLEDColor(255, 0, 0);
      break;
    case 1:  // Conectando ao Wi-Fi (Amarelo)
      setLEDColor(150, 255, 0);
      break;
    case 2:  // Conectando ao MQTT (Rosa)
      setLEDColor(150, 0, 255);
      break;
    case 3:  // Movendo para frente (Verde)
      setLEDColor(0, 255, 0);
      break;
    case 4:  // Movendo para trás (Ciano)
      setLEDColor(0, 255, 255);
      break;
    case 0:  // Parado (desliga LEDs)
      turnOffLEDs();
      break;
    default:
      for (byte i = 0; i < 4; i++) {
        setLEDColor(0, 0, 255);  // Erro no status (pisca azul)
        delay(100);
        turnOffLEDs();
        delay(100);
      }
      break;
  }
}

void turnOffLEDs() {
  setLEDColor(0, 0, 0);
}

void setLEDColor(byte r, byte g, byte b) {
  ledcWrite(CH_R, r);
  ledcWrite(CH_G, g);
  ledcWrite(CH_B, b);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida em ");
  Serial.println(topic);

  String msg = "";
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  if (String(topic) == "in/S4") {
    if (!msg.startsWith("speed")) return;
    String val = msg.substring(6);
    Serial.print("Velocidade recebida: ");
    Serial.println(val);
    int valor = val.toInt();
    if (valor > 0) {  // Frente
      digitalWrite(dir1, HIGH);
      digitalWrite(dir2, LOW);
      statusLED(3);
    } else if (valor < 0) {  // Ré
      digitalWrite(dir1, LOW);
      digitalWrite(dir2, HIGH);
      statusLED(4);
    } else {  // Parado
      digitalWrite(dir1, LOW);
      digitalWrite(dir2, LOW);
      statusLED(0);
    }
  }
}

void connectToWifi() {
  // Conectando à Internet:
  statusLED(1);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.println("Conectando à Internet...");

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - start > 10000) { 
      Serial.println("Falha ao conectar. Retentando...");
      statusLED(254);
      start = millis();
    }
    delay(200);
  }
  Serial.println("\nConectado à Internet!");
  statusLED(0);
}

void connectToMQTT() {
  // Comunicação com broker:
  statusLED(2);
  mqtt.setServer(BROKER_URL, BROKER_PORT);
  mqtt.setCallback(callback);

  String clientID = "TREM-";
  clientID += String(random(0xffff), HEX);
  Serial.println("Conectando ao Broker...");

  unsigned long start = millis();
  while (!mqtt.connected()) {
    if (mqtt.connect(clientID.c_str())) {
      Serial.println("\nConectado ao Broker!");
      mqtt.subscribe(TOPIC_SPEED);
      statusLED(0);
    } else {
      Serial.print("Falha ao conectar.");
      statusLED(254);
      delay(2000);
      if (millis() - start > 30000) { 
        break;
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0));
  wifiClient.setInsecure();

  pinMode(dir1, OUTPUT);
  pinMode(dir2, OUTPUT);

  // Configura PWM (ledc) corretamente: setup + attach pin
  ledcSetup(CH_R, PWM_FREQ, PWM_RES);
  ledcAttachPin(vermelhinho, CH_R);

  ledcSetup(CH_G, PWM_FREQ, PWM_RES);
  ledcAttachPin(verdinho, CH_G);

  ledcSetup(CH_B, PWM_FREQ, PWM_RES);
  ledcAttachPin(azulzinho, CH_B);

  // Conexões iniciais
  connectToWifi();
  connectToMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi desconectado. Tentando reconectar...");
    connectToWifi();
  }

  if (!mqtt.connected()) {
    Serial.println("MQTT não conectado. Tentando conectar...");
    connectToMQTT();
  }
  
  mqtt.loop();
}
