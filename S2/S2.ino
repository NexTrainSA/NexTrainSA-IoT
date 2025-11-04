#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "env.h"


WiFiClientSecure wifiClient;
PubSubClient mqtt(wifiClient);

// Para fazer: Acender e apagar led, recebe mensagem; Sensor dentro do loop; Servo não precisa; Manda distância para o tópico; 

// Pinos LED RGB:
const int redPin = 2;
const int greenPin = 4;
const int bluePin = 6;


// Sensor ultrassônico:
const int trig1 = 4;   // sensor 1
const int echo1 = 5;   // sensor 1
const int trig2 = 18;  // sensor 2
const int echo2 = 19;  // sensor 2


const String brokerUser = "";


void setup() {
  Serial.begin(115200);
  wifiClient.setInsecure();


  pinMode(trig1, OUTPUT);
  pinMode(echo1, INPUT);
  pinMode(trig2, OUTPUT);
  pinMode(echo2, INPUT);


  ledcAttach(redPin, 5000, 8);
  ledcAttach(greenPin, 5000, 8);
  ledcAttach(bluePin, 5000, 8);


  WiFi.begin(WIFI_SSID, WIFI_PASS);  //tenta conectar na rede


  Serial.println("Conectando no WiFi");
  WiFi.begin(WIFI_SSID, WII_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi conectado com sucesso!");
  Serial.print("Endereço IP: ");
    Serial.println(WiFi.localIP();


    delay(200);
}
Serial.println("Conectado com sucesso!");
mqtt.setServer(BROKER_URL.c_str(), BROKER_PORT);
String clientID = "S2*";
clientID += String(random(0xffff), HEX);
Serial.println("Conectado ao broker");
while (mqtt.connect(clientID.c_str()) == 0) {
  Serial.println(".");
  delay(2000);
}
mqtt.subscribe(topic.c_str());
mqtt.setCallback(callback);
Serial.println("\nConectado ao broker!");
}



float medirDistancia(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(10);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);


  long duracao = pulseIn(echo, HIGH);
  float distancia = duracao * 0.034 / 2;  // Convertendo em cm
  return distancia;
}


void callback(char* topic, byte* payload, unsigned long length) {
  String MensagemRecebida = "";
  for (int i = 0; i < length; i++) {
    //Pega cada letra de payload e junta na mensagem
    MensagemRecebida += (char)payload[i];
  }
  Serial.println(MensagemRecebida);

  // Lendo as distâncias:

  distancia1 = medirDistancia(trigPin1, echoPin1);
  distancia2 = medirDistancia(trigPin2, echoPin2);


  Serial.printf("Distância 1: %.2f cm | Distância 2: %.2f cm\n", distancia1, distancia2);  // Manda em cm

  // Servo:
  if (String(topic) == "NexTrain/S2/Atuadores/Servo_3") {
    int angulo = msg.toInt();  // Msg vira ângulo
    if (angulo >= 0 && angulo <= 180) {
      servo3.write(angulo);
      Serial.print("Servo 3 ajustado para ");
      Serial.print(angulo);
      Serial.println(" graus");
    }
  }
}

void loop() {
  mqtt.publish("NexTrain/S2/Sensores/Presence_2", String(distancia1).c_str());
  mqtt.publish("NexTrain/S2/Sensores/Presence_4", String(distancia2).c_str());
  mqtt.loop();
}
