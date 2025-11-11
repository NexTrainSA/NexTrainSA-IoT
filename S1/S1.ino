#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "env.h"
#include <DHT.h>

#define pinLDR 34       //ldr
#define pinDHT 13       // Pino digital do ESP32 para o DHT
#define DHTTYPE DHT11   // Tipo de sensor: DHT11
#define pinPIR 25      // Pino digital para o Sensor PIR

const float LIMITAR_TEMPERATURA = 28.0; // acima de 28.0°C é considerado "Quente"
const float LIMITAR_UMIDADE = 60.0;     // acima de 60.0% é considerado "Úmido"

DHT dht(pinDHT, DHTTYPE);

WiFiClientSecure wifi_client;
PubSubClient mqtt(wifi_client);

const String brokerUser = ""; //na SA vai ter, já que esse é publico e a SA não
const String brokerPass = "";

void setup() {
  Serial.begin(115200);

  wifi_client.setInsecure();

    pinMode(2, OUTPUT);
    dht.begin();
    pinMode(pinPIR, INPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.println("Conectando no WiFi");
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(200);
  }

  Serial.println("Conectado com sucesso!");

  mqtt.setServer(BROKER_URL, BROKER_PORT); //isso aí cria um nome aleatório que aparece pra plaquinha, que é "S1-" + um código aleatório qualquer
  String clientID = "S1-";
  clientID += String(random(0xffff),HEX);
  Serial.println("Conectando ao broker...");
  while (mqtt.connect (clientID.c_str () , BROKER_USR_ID, BROKER_USR_PASS) == 0 ){ //aqui o cod do iago
    Serial.println(".");
    delay(200);
  }
  mqtt.subscribe(TOPIC_PRESENCE_1); //conectando a inscrição no tópico com outra placa de msm tópico
  mqtt.subscribe(TOPIC_UMID_1);
  mqtt.subscribe(TOPIC_TEMP_1);
  mqtt.subscribe(TOPIC_LUMI_1);

  mqtt.setCallback(callback);
  Serial.println("\nConectado ao broker!");
}

void loop() {
    int luz = map(analogRead(pinLDR),0,4095,0,100);
    if(luz > 50){
      mqtt.publish(TOPIC_LUMI_1,"Claro");
    }else{
      mqtt.publish(TOPIC_LUMI_1,"Escuro");
    }

    mqtt.loop();
    delay(500);

  float t = dht.readTemperature();
  if (t > LIMITAR_TEMPERATURA){
    mqtt.publish(TOPIC_TEMP_1,"Quente");
  } else {
    mqtt.publish(TOPIC_TEMP_1,"Frio");
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

  }

    mqtt.loop();
    delay(500);

    int presenca = digitalRead(pinPIR);
    if(presenca == HIGH){
      mqtt.publish(TOPIC_PRESENCE_1,"Presente");
    }else{
      mqtt.publish(TOPIC_PRESENCE_1,"Ausente");
    }

    mqtt.loop();
    delay(500);
}

void callback(char * topic, byte* payload, unsigned long length) { //callback para a conexão de inscrição

 /* String MensagemRecebida = "";

  for(int i = 0; i < length; i++){ //pega cada letra de payload e junta a mensagem, bit por bit (letra por letra)
    MensagemRecebida += (char) payload[i];
  }

  Serial.println(MensagemRecebida); //faz aqui o que quiser com a mensagem -> aqui vamos por o led, provavelmente

  if(MensagemRecebida == "Nome: 1"){
    digitalWrite(2, HIGH);
  }else if(MensagemRecebida == "Nome: 0"){
    digitalWrite(2, LOW);
  } */

}