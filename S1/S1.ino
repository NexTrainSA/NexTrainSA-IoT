#include <WiFi.h>
#include <PubSubClient.h>
#include "env.h"

WiFiClient wifi_client;
PubSubClient mqtt(wifi_client);

const String brokerUser = "";
const String brokePass = "";

void setup() { 
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.println("Conectando no WiFi");
  while(WiFi.status()!= WL_CONNECTED){ //Comando da biblioteca do wifi, se for diferente de concetado, ele não esta conectado
  Serial.println(".");
  delay(200);
  }
  Serial.println("Conectado com sucesso!");

  mqtt.setServer(BROKER_URL, BROKER_PORT);
  String clientID = "S1";
  clientID += String(random(0xffff), HEX); //o programa vai gerar um número diferente, a chance de ter dois iguais é muito baixa
  Serial.println("Conectando ao Broker");
  while(mqtt.connect(clientID.c_str()) == 0){
    Serial.print(".");
    delay(200);
  }
  mqtt.subscribe(TOPIC_PRESENCE1);
  mqtt.setCallback(callback);
  Serial.println("\nConectado ao broker!");
}
void loop() {
  String mensagem = "";
  if(Serial.available()>0){
    mensagem = Serial.readStringUntil('\n');
    mensagem = "Yasmin:" + mensagem;
    mqtt.publish("mari",mensagem.c_str());
  }
  mqtt.loop();
}

void callback(char* topic, byte* payload, unsigned long length){
  String MensagemRecebida = "";
  for (int i= 0; i < length; i++){
    MensagemRecebida += (char) payload[i];
  }
  Serial.println(MensagemRecebida);

}



