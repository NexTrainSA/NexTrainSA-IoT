#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "env.h"

WiFiClientSecure wifiClient;
PubSubClient mqtt(wifiClient);

const byte dir1 = 4;
const byte dir2 = 5;

const byte verdinho = 14;
const byte vermelhinho = 12;
const byte azulzinho = 13;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida em ");
  Serial.println(topic);

  // Acho que primeiro converte de byte para char... 
  String msg = "";
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  // ... e de char para int
  int valor = msg.toInt();
  Serial.print("Velocidade recebida: ");
  Serial.println(valor);

  if (String(topic) == "NexTrain/TREM/Atuadores/SPEED") {
    if (valor > 0) {
      digitalWrite(dir1, HIGH);
      digitalWrite(dir2, LOW);
      digitalWrite(verdinho, HIGH);
      digitalWrite(vermelhinho, LOW);
      digitalWrite(azulzinho, LOW);
    } else if (valor < 0) {
      digitalWrite(dir1, LOW);
      digitalWrite(dir2, HIGH);
      digitalWrite(azulzinho, HIGH);
      digitalWrite(verdinho, LOW);
      digitalWrite(vermelhinho, LOW);
    } else {
      digitalWrite(dir1, LOW);
      digitalWrite(dir2, LOW);
      digitalWrite(vermelhinho, HIGH);
      digitalWrite(verdinho, LOW);
      digitalWrite(azulzinho, LOW);
    }
  }
}

void setup() {
  Serial.begin(115200);
  wifiClient.setInsecure();

  pinMode(verdinho, OUTPUT);
  pinMode(vermelhinho, OUTPUT);
  pinMode(azulzinho, OUTPUT);
  pinMode(dir1, OUTPUT);
  pinMode(dir2, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.println("Conectando à Internet...");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }

  Serial.println("\nConectado à Internet!");

  // Comunicação com broker:
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
      Serial.print(".");
      delay(2000);
    }
  }
}

void loop() {
  // Publicar:
  if (Serial.available() > 0) {
    String mensagem = Serial.readStringUntil('\n');
    mqtt.publish(TOPIC_RGB_4, mensagem.c_str());
  }

  mqtt.loop();
}
