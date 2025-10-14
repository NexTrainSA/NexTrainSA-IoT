#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient wifi_client;
PubSubClient mqtt(wifi_client);

const String SSID = "FIESC_IOT_EDU"; //nome da rede wifi
const String PASS = "8120gv08"; //Senha da rede

const String brokerURL = "test.mosquitto.org";
const int brokerPort = 1883;

const int pino = 2;

const String topic = "topico_S2";

const String brokerUser = "";
const String brokerPass = "";

void setup() {
  Serial.begin(115200);
  pinMode(pino, OUTPUT);
  WiFi.begin(SSID, PASS); //tenta conectar na rede
  Serial.println("Conectando no WiFi");
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(200);
    }
    Serial.println("Conectado com sucesso!");
    mqtt.setServer(brokerURL.c_str(),brokerPort);
    String clientID = "S2*";
    clientID += String(random(0xffff),HEX);
    Serial.println("Conectado ao broker");
    while(mqtt.connect(clientID.c_str()) == 0) {
      Serial.println(".");
      delay(2000);
    }
    mqtt.subscribe(topic.c_str());
    mqtt.setCallback(callback);
    Serial.println("\nConectado ao broker!");
  }
  
void loop() {
  String mensagem = "";
  if(Serial.available() > 0) {
    mensagem = Serial.readStringUntil('\n');
    mensagem = "Gabriela:" + mensagem;
    Serial.println("A mensagem foi: ");
    Serial.println (mensagem);
    mqtt.publish("topicoTrem",mensagem.c_str());
  }
  mqtt.loop();
}

void callback(char* topic, byte* payload, unsigned long length) {
  String MensagemRecebida = "";
    for(int i = 0; i < length; i++){
      //Pega cada letra de payload e junta na mensagem
      MensagemRecebida += (char) payload[i];
    }
    Serial.println(MensagemRecebida);
    if (MensagemRecebida == "Acender" || "ACENDER") {
      digitalWrite (pino, HIGH);
    }
  }
