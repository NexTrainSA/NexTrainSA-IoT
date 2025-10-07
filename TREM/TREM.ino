#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

const String SSID = "FIESC_IOT_EDU";
const String PASS = "8120gv08";

const String brokerURL = "test.mosquitto.org";
const int brokerPort = 1883;

const String brokerUser = "";
const String brokerPass = "";

void setup() {
  Serial.begin(115200);
  WiFi.begin(SSID, PASS);  // Tenta conectar na Internet
  Serial.println("Conectando à Internet...");

  while (WiFi.status() != WL_CONNECTED) {  // Ou seja, não está conectado
    Serial.print("...");
    delay(200);
  }

  // Comunicação com broker:
  mqtt.setServer(brokerURL.c_str(), brokerPort);  // Definindo servidor
  String clientID = "TREM-";
  clientID += String(random(0xffff), HEX);  // Forma texto aleatório para acompanhar o clientID
  Serial.println("Conectando ao Broker...");

  // Começando comunicação:
  while (mqtt.connect(clientID.c_str()) == 0) { // Enquanto não conectar:
    Serial.println("...");
      delay(200);
  }
  Serial.println("\nConectado ao Broker!");
  // Fim do código da conexão com Broker
}

void loop() {
  // put your main code here, to run repeatedly:
}
