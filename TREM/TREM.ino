#include <WiFi.h>
#include <PubSubClient.h>
#include <WifiClientSecure.h>
#include <"env.h">
#include <stdio.h> // Por causa da função sscanf

WifiClientSecure wifiClient;
PubSubClient mqtt(wifiClient);

const byte verdinho = ;
const byte vermelhinho = ;
const byte azulzinho = ;

void setup() {
  Serial.begin(115200);
  wifiClient.setInsecure();

  pinMode(pino, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.println("Conectando à Internet...");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("...");
    delay(200);
  }

  Serial.println("Conectado à Internet!");

  // Comunicação com broker:
  mqtt.setServer(BROKER_URL.c_str(), BROKER_PORT);  
  String clientID = "TREM-";
  clientID += String(random(0xffff), HEX); 
  Serial.println("Conectando ao Broker...");

  // Começando comunicação:
  while (mqtt.connect(clientID.c_str()) == 0) {
    Serial.println(".");
      delay(2000);
  }

  // Inscrição:
  mqtt.subscribe(TOPIC_SPEED);
  mqtt.setCallback(callback);
  // Fim da inscrição

    Serial.println("...");
      delay(200);

  Serial.println("\nConectado ao Broker!");
  // Fim do código da conexão com Broker
}

void loop() {

  // Publicar:
  String mensagem = ""; 
  if (Serial.available() >  0) {
    mensagem = Serial.readStringUntil('\n');
    mensagem = "Fernanda: " + mensagem;
    // Mandar msg para o broker:
    mqtt.publish(TOPIC_RGB_4, mensagem.c_str()); 
  }
  mqtt.loop();
  // Fim da parte de Publicar
}

void callback (char* topic, char* payload, unsigned long length) { 
  // Pesquisar como converter o payload para int, pois a velocidade é um número inteiro
  // Mudar essa parte de baixo aqui ⬇️⬇️⬇️

  char* payload = "";
  int valor;
  if (sscanf(payload, "%d", &valor) == 1) {
        // Aí deu certo
  } else {
        Serial.println("Acho que deu ruim.");
    }

  String mensagemRecebida = "";
  for (int i = 0; i < length; i++) {
    msg += (byte) payload[i]; 
  }
  Serial.println(msg);

  if(topic == "NexTrain/TREM/Atuadores/SPEED"){
    if(msg > 0) {
      digitalWrite(dir1, HIGH);
      digitalWrite(dir2, LOW);
      digitalWrite(verdinho, HIGH);
      digitalWrite(vermelhinho, LOW);
      digitalWrite(azulzinho, LOW);
    } else if(msg < 0) {
        digitalWrite(dir1, LOW);
        digitalWrite(dir2, HIGH);
        digitalWrite(azulzinho, HIGH);
        digitalWrite(verdinho, LOW);
        digitalWrite(vermelhinho, LOW);
    } else {
      digitalWrite(dir1, LOW);
      digitalWrite(dir2, LOW);
      digitalWrite(vermelhinho, HIGH);
      digitalWrite(verdinho, LOW);
      digitalWrite(azulzinho, LOW);
    }
  }
}
