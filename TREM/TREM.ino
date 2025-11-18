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
  ledcWrite(vermelhinho, r);
  ledcWrite(verdinho, g);
  ledcWrite(azulzinho, b);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida em ");
  Serial.println(topic);

  String msg = "";
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  if (String(topic) == "in/S4") {
    if (!msg.startsWith("speed")) return;
    String val = msg.substring(6);
    Serial.print("Velocidade recebida: ");
    Serial.println(val);
    int valor = val.toInt();
    if (valor > 0) {  // Move para frente
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
      statusLED(254);
    }
  }
}

void connectToWifi() {
  // Conectando à Internet:
  statusLED(2);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.println("Conectando à Internet...");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Falha ao conectar.");
    statusLED(254);
    delay(200);
  }
  Serial.println("\nConectado à Internet!");
}

void connectToMQTT() {
  // Comunicação com broker:
  statusLED(2);
  mqtt.setServer(BROKER_URL, BROKER_PORT);
  mqtt.setCallback(callback);

  String clientID = "TREM-";
  clientID += String(random(0xffff), HEX);
  Serial.println("Conectando ao Broker...");

  while (!mqtt.connected()) {
    if (mqtt.connect(clientID.c_str())) {
      Serial.println("\nConectado ao Broker!");
      mqtt.subscribe(TOPIC_SPEED);
    } else {
      Serial.print("Falha ao conectar.");
      statusLED(254);
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  wifiClient.setInsecure();

  pinMode(dir1, OUTPUT);
  pinMode(dir2, OUTPUT);
  ledcAttach(vermelhinho, 5000, 8);
  ledcAttach(verdinho, 5000, 8);
  ledcAttach(azulzinho, 5000, 8);
}

void loop() {
  mqtt.loop();
}
