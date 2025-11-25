#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h> // Essencial para comunicação segura (TLS)
#include "env.h"
#include <DHT.h>

// --- Definições de Pinos (Estação S1) ---
#define pinLDR 34       // LDR (Luminosidade) - Leitura Analógica (ADC1)
#define pinDHT 4        // Pino digital para o sensor DHT11
#define DHTTYPE DHT11   // Tipo de sensor: DHT11

#define ledPin 19       // LED de Iluminação
#define TRIGGER_PIN 22  // Pino Trigger do Ultrassônico
#define ECHO_PIN 23     // Pino Echo do Ultrassônico

#define LED_RGB_R 14    // LED RGB - Red
#define LED_RGB_G 26    // LED RGB - Green
#define LED_RGB_B 25    // LED RGB - Blue



String currentColor = "NONE"; 

// Variáveis de Limites (Usadas para definir a cor de status do LED RGB)
const float LIMITAR_TEMPERATURA = 28.0; // Acima de 28.0°C é "Quente"
const float LIMITAR_UMIDADE = 60.0;     // Acima de 60.0% é "Úmido"


DHT dht(pinDHT, DHTTYPE);
WiFiClientSecure wifi_client;

PubSubClient mqtt(wifi_client);


void callback(char* topic, byte* payload, unsigned int length);
void setRgbColor(int r, int g, int b);
long lerDistancia();

// Função para ler a distância (Ultrassônico)
long lerDistancia() {
  
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);


  long duracao = pulseIn(ECHO_PIN, HIGH);
  
  long distancia = duracao * 0.03432 / 2; 

  return distancia;
}

// --- Função para controlar o LED RGB 
void setRgbColor(int r, int g, int b) {
  
  digitalWrite(LED_RGB_R, !r); 
  digitalWrite(LED_RGB_G, !g);
  digitalWrite(LED_RGB_B, !b);
  
  
  if (r == 1 && g == 0 && b == 0) currentColor = "RED";
  else if (r == 0 && g == 1 && b == 0) currentColor = "GREEN";
  else if (r == 0 && g == 0 && b == 1) currentColor = "BLUE";
  else currentColor = "NONE";
}


void callback(char* topic, byte* payload, unsigned int length) { 
  String MensagemRecebida = "";

  for (int i = 0; i < length; i++) {  
    MensagemRecebida += (char)payload[i];
  }
  Serial.print("Mensagem recebida no tópico [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(MensagemRecebida);  

  
  if (strcmp(topic, TOPIC_LUMI_1) == 0 && MensagemRecebida == "LIGAR_LUZ_EXTERNA") {
    digitalWrite(ledPin, HIGH);
  } else if (strcmp(topic, TOPIC_LUMI_1) == 0 && MensagemRecebida == "DESLIGAR_LUZ_EXTERNA") {
    digitalWrite(ledPin, LOW);
  }
}

void setup() {
  Serial.begin(115200);

  
  wifi_client.setInsecure();

  // --- Configuração dos Pinos ---
  dht.begin(); // Inicializa o sensor DHT
  pinMode(ledPin, OUTPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_RGB_R, OUTPUT);
  pinMode(LED_RGB_G, OUTPUT);
  pinMode(LED_RGB_B, OUTPUT);
  
  // Desliga os LEDs inicialmente (Anodo Comum = HIGH)
  digitalWrite(ledPin, LOW); 
  setRgbColor(0, 0, 0); // Desliga o RGB

  // --- Conexão Wi-Fi ---
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.println("Conectando no WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
  Serial.println("\nConectado com sucesso!");

  // --- Conexão MQTT ---
  mqtt.setServer(BROKER_URL, BROKER_PORT);
  String clientID = "S1-";
  clientID += String(random(0xffff), HEX);
  Serial.println("Conectando ao broker...");

  while (!mqtt.connect(clientID.c_str(), BROKER_USR_ID, BROKER_USR_PASS)) {
    Serial.print(".");
    delay(200);
  }
  Serial.println("\nConectado ao broker!");

  
  mqtt.subscribe(TOPIC_PRESENCE_1); 
  .
  
  mqtt.setCallback(callback);
}

void loop() {
  
  if (!mqtt.connected()) {
    
    Serial.println("Tentando reconectar ao MQTT...");
    String clientID = "S1-";
    clientID += String(random(0xffff), HEX);
    if (mqtt.connect(clientID.c_str(), BROKER_USR_ID, BROKER_USR_PASS)) {
      Serial.println("Reconectado!");
      // Resubscribe nos tópicos
      mqtt.subscribe(TOPIC_PRESENCE_1);
    } else {
      delay(5000); 
    }
  }

  mqtt.loop();
  
 

  // 1. Luminosidade (LDR) e Controle do LED de Iluminação
  int luz_analog = analogRead(pinLDR);
  int luz_percent = map(luz_analog, 0, 4095, 0, 100); 
  
  if (luz_percent > 50) {
    mqtt.publish(TOPIC_LUMI_1, "Claro");
    digitalWrite(ledPin, LOW); // Desliga a luz
  } else {
    mqtt.publish(TOPIC_LUMI_1, "Escuro");
    digitalWrite(ledPin, HIGH); // Liga a luz
  }

  delay(500);
  mqtt.loop();

  
  float t = dht.readTemperature();
  if (!isnan(t)) {
    String tempString = String(t, 1);
    mqtt.publish(TOPIC_TEMP_1, tempString.c_str());

    // Lógica para o LED RGB baseada na TEMPERATURA
    if (t > LIMITAR_TEMPERATURA) {
      setRgbColor(1, 0, 0); // Vermelho: Quente
    } else if (t < 20.0) {
      setRgbColor(0, 0, 1); // Azul: Frio (Abaixo de 20°C)
    } else {
      setRgbColor(0, 1, 0); // Verde: Normal
    }
  }

  delay(500);
  mqtt.loop();

  // 3. Umidade (DHT)
  float h = dht.readHumidity();
  if (!isnan(h)) {
    String umidString = String(h, 1);
    mqtt.publish(TOPIC_UMID_1, umidString.c_str());
  }

  delay(500);
  mqtt.loop();

  // 4. Presença (Ultrassônico)
  long distancia = lerDistancia();

  if (distancia > 0 && distancia < 50) { 
    mqtt.publish(TOPIC_PRESENCE_1, "Presente");
   
  } else {
    mqtt.publish(TOPIC_PRESENCE_1, "Ausente");
  }

  delay(500); 
}