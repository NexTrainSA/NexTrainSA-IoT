#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient wifi_client;
PubSubClient mqtt(wifi_client);

const String SSID = "FIESC_IOT_EDU"; //vamos fazer de um jeito nao ideal 
const String PASS = "8120gv08";

const String brokerURL = "test.mosquitto.org";
const int brokerPort = 1883;
const String topic = "Yasmin";

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
  mqtt.subscribe(topic.c_str());
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



