#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

const String SSID = "FIESC_IOT_EDU";
const String PASS = "8120gv08";

const String brokerURL = "test.mosquitto.org";
const int brokerPort = 1883;

const String topic = "topicoTrem"; // Criação de tópico próprio

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
  Serial.println("Conectado à Internet!");

  // Comunicação com broker:
  mqtt.setServer(brokerURL.c_str(), brokerPort);  // Definindo servidor
  String clientID = "TREM-";
  clientID += String(random(0xffff), HEX);  // Forma texto aleatório para acompanhar o clientID
  Serial.println("Conectando ao Broker...");

  // Começando comunicação:
  while (mqtt.connect(clientID.c_str()) == 0) { // Enquanto não conectar:
    Serial.println(".");
      delay(2000);
  }
  // Inscrição:
  mqtt.subscribe(topic.c_str());
  mqtt.setCallback(callback);
  // Fim da inscrição

  Serial.println("\nConectado ao Broker!");
  // Fim do código da conexão com Broker
}

void loop() {
  // Publicar:
  String mensagem = ""; // Variável a ser zerada/recriada toda vez (só dentro do loop)
  if (Serial.available() >  0) {
    mensagem = Serial.readStringUntil('\n');
    mensagem = "Fernanda: " + mensagem;
    // Mandar msg para o broker:
    mqtt.publish("topico_S2", mensagem.c_str()); // Nome do broker.publish("Tópico", mensagem.c_str());
  }
  mqtt.loop();
  // Fim da parte de Publicar
}

void callback (char* topic, byte* payload, unsigned long length) { // Argumentos padrões: de onde veio, a mensagem em si e o tamanho em bytes
  String mensagemRecebida = "";
  for (int i = 0; i < length; i++) {
    mensagemRecebida += (char) payload[i]; // Juntas as letras do payload na mensagem
  }
  Serial.println(mensagemRecebida);
}
