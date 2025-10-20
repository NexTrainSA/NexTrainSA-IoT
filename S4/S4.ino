#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient wifi_client;
PubSubClient mqtt(wifi_client);

const String SSID = "FIESC_IOT_EDU";
const String PASS = "8120gv08";

const String brokerURL = "test.mosquitto.org";
const int brokerPort = 1883;
const String topic = "mari";

const String brokerUser = "";
const String brokerPass = "";

void setup() {
  Serial.begin(115200);
  WiFi.begin(SSID, PASS);
  Serial.println("Concetando no Wifi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
  Serial.println("Conectado com sucesso!");
  mqtt.setServer(brokerURL.c_str(), brokerPort);
  String clientID = "S4-";
  clientID += String(random(0xffff), HEX);
  while (mqtt.connect(clientID.c_str()) == 0) {
    Serial.print(".");
    delay(200);
  }
  mqtt.subscribe(topic.c_str());
  mqtt.setCallback(callback);
  Serial.println("\nConectado ao broker!");
}

void loop() {
  String mensagem = "";
  if(Serial.available() > 0){
    mensagem = Serial.readStringUntil('\n');
    mensagem = "mari: " + mensagem;
    mqtt.publish("Yasmin",mensagem.c_str());
  }
  mqtt.loop();
}

void callback(char* topic, byte* payload, unsigned long length){
    String MensagemRecebida = "";
    for(int i = 0; i < length; i++){
      MensagemRecebida += (char) payload[i];
    }
    Serial.println(MensagemRecebida);
}