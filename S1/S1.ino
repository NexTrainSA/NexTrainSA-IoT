#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "env.h"
#include <DHT.h>

#define pinLDR 34       //ldr
#define pinDHT 4       // Pino digital do ESP32 para o DHT
#define DHTTYPE DHT11   // Tipo de sensor: DHT11
#define led 19
#define pinLDR 34
#define pinDHT 4      
#define DHTTYPE DHT11


#define TRIGGER_PIN 22 //ultrassonico
#define ECHO_PIN 23 //ultrassonico

#define LED_RGB_R 14 // led rgb r
#define LED_RGB_G 26 // led rgb g 
#define LED_RGB_B 25 // led rgb b 

#define Ultra_tring 22 //ultrassonic pin
#define Ultra_echo 23 //ultrassonic pin 
#define LED_RGB_B 25 // led rgb b

const float LIMITAR_TEMPERATURA = 28.0; // acima de 28.0°C é considerado "Quente"
const float LIMITAR_UMIDADE = 60.0;     // acima de 60.0% é considerado "Úmido"
const float LIMITAR_TEMPERATURA = 28.0;  
const float LIMITAR_UMIDADE = 60.0;      

DHT dht(pinDHT, DHTTYPE);

WiFiClientSecure wifi_client;
PubSubClient mqtt(wifi_client);

const String brokerUser = ""; //na SA vai ter, já que esse é publico e a SA não
const String brokerUser = "";  
const String brokerPass = "";

void setup() {
  Serial.begin(115200);

  wifi_client.setInsecure();

    pinMode(2, OUTPUT);
    dht.begin();
    pinMode(pinPIR, INPUT);
  pinMode(2, OUTPUT);
  dht.begin();

  pinMode(19, OUTPUT);

  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.println("Conectando no WiFi");
  while(WiFi.status() != WL_CONNECTED){
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }

  Serial.println("Conectado com sucesso!");

  mqtt.setServer(BROKER_URL, BROKER_PORT); //isso aí cria um nome aleatório que aparece pra plaquinha, que é "S1-" + um código aleatório qualquer
  mqtt.setServer(BROKER_URL, BROKER_PORT);  
  String clientID = "S1-";
  clientID += String(random(0xffff),HEX);
  clientID += String(random(0xffff), HEX);
  Serial.println("Conectando ao broker...");
  while (mqtt.connect (clientID.c_str () , BROKER_USR_ID, BROKER_USR_PASS) == 0 ){ //aqui o cod do iago
  while (mqtt.connect(clientID.c_str(), BROKER_USR_ID, BROKER_USR_PASS) == 0) {  //aqui o cod do iago
    Serial.println(".");
    delay(200);
  }
  mqtt.subscribe(TOPIC_PRESENCE_1); //conectando a inscrição no tópico com outra placa de msm tópico
  mqtt.subscribe(TOPIC_PRESENCE_1);  
  mqtt.subscribe(TOPIC_UMID_1);
  mqtt.subscribe(TOPIC_TEMP_1);
  mqtt.subscribe(TOPIC_LUMI_1);

  mqtt.setCallback(callback);
@@ -55,54 +70,85 @@ void setup() {
  Serial.println("\nConectado ao broker!");
}

void loop() {
    int luz = map(analogRead(pinLDR),0,4095,0,100);
    if(luz > 50){
      mqtt.publish(TOPIC_LUMI_1,"Claro");
    }else{
      mqtt.publish(TOPIC_LUMI_1,"Escuro");
    }
//função de ler distância:
long lerDistancia() {
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

    mqtt.loop();
    delay(500);
  long duracao = pulseIn(ECHO_PIN, HIGH);
  long distancia = duracao * 349.24 / 2 / 10000;

  float t = dht.readTemperature();
  if (t > LIMITAR_TEMPERATURA){
    mqtt.publish(TOPIC_TEMP_1,"Quente");
  return distancia;
}


void loop() {
  int luz = map(analogRead(pinLDR), 0, 4095, 0, 100);
  if (luz > 50) {
    mqtt.publish(TOPIC_LUMI_1, "Claro");
    digitalWrite(19, LOW);
  } else {
    mqtt.publish(TOPIC_TEMP_1,"Frio");
    mqtt.publish(TOPIC_LUMI_1, "Escuro");
    digitalWrite(19, HIGH);
  }
  //adicionar o dado com um novo tópico, para o site
  
    mqtt.loop();
    delay(500);

  float h = dht.readHumidity();
    if (h > LIMITAR_UMIDADE){
    mqtt.publish(TOPIC_UMID_1,"Úmido");
  } else {
    mqtt.publish(TOPIC_UMID_1,"Seco");
//adicionar o dado com um novo tópico, para o site
  mqtt.loop();
  delay(500);

  }
  float t = dht.readTemperature();
  String tempString = String(t, 1);
  mqtt.publish(TOPIC_TEMP_1, tempString.c_str());

  mqtt.loop();
  delay(500);

  float h = dht.readHumidity();
 
  String umidString = String(h, 1);  // 1 dígito decimal de precisão
  mqtt.publish(TOPIC_UMID_1, umidString.c_str());

    mqtt.loop();
    delay(500);
  mqtt.loop();
  delay(500);

    int presenca = digitalRead(pinPIR);
    if(presenca == HIGH){
      mqtt.publish(TOPIC_PRESENCE_1,"Presente");
    }else{
      mqtt.publish(TOPIC_PRESENCE_1,"Ausente");
    }
  long distancia = lerDistancia();

    mqtt.loop();
    delay(500);
  if (distancia < 10) {
    mqtt.publish(TOPIC_PRESENCE_1, "Presente");
  } else {
    mqtt.publish(TOPIC_PRESENCE_1, "Ausente");
  }
  delay(500);
}