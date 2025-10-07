#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient wifi_client;
PubSubClient mqtt(wifi_client);

const String SSID = "FIESC_IOT_EDU"; //vamos fazer de um jeito nao ideal 
const String PASS = "8120gv08";

const String brokerURL = "test.mosquitto.org";
const int brokerPort = 1883;

const String brokerUser = "";
const String brokePass = "";

void setup() { 
  Serial.begin(115200);
  WiFi.begin(SSID, PASS);
  Serial.println("Conectando no WiFi");
  while(WiFi.status()!= WL_CONNECTED){ //Comando da biblioteca do wifi, se for diferente de concetado, ele não esta conectado
  Serial.println(".");
  delay(200);
  }
  Serial.println("Conectado com sucesso!");

  mqtt.setServer(brokerURL.c_str(), brokerPort);
  String clientID = "S1";
  clientID += String(random(0xffff), HEX); //o programa vai gerar um número diferente, a chance de ter dois iguais é muito baixa
  Serial.println("Conectando ao Broker");
  while(mqtt.connect(clientID.c_str()) == 0){
    Serial.print(".");
    delay(200);
  }
  Serial.println("\nConectado ao broker!");
}
void loop() {
  // put your main code here, to run repeatedly:
}