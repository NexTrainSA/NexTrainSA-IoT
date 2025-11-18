#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "env.h"
#include <DHT.h>

// Definições de Pinos
#define pinLDR 34       // LDR (Luminosidade)
#define pinDHT 4        // Pino digital para o sensor DHT11
#define DHTTYPE DHT11   // Tipo de sensor: DHT11

#define led 19          // LED de Iluminação
#define TRIGGER_PIN 22  // Pino Trigger do Ultrassônico
#define ECHO_PIN 23     // Pino Echo do Ultrassônico

#define LED_RGB_R 14    // LED RGB - Red
#define LED_RGB_G 26    // LED RGB - Green
#define LED_RGB_B 25    // LED RGB - Blue

// Variáveis de Limites (se não usadas, podem ser removidas)
const float LIMITAR_TEMPERATURA = 28.0; // Acima de 28.0°C é considerado "Quente"
const float LIMITAR_UMIDADE = 60.0;     // Acima de 60.0% é considerado "Úmido"

// Objetos
DHT dht(pinDHT, DHTTYPE);
WiFiClientSecure wifi_client;
PubSubClient mqtt(wifi_client);



// Função para ler a distância (Ultrassônico)
long lerDistancia() {
  // Envia pulso de trigger
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  // Lê a duração do pulso de echo
  long duracao = pulseIn(ECHO_PIN, HIGH);
  // Converte a duração para distância em cm (usando 343.2 m/s, convertido para cm/µs)
  long distancia = duracao * 0.03432 / 2; // 0.03432 cm/µs é a velocidade do som

  return distancia;
}

void setup() {
  Serial.begin(115200);

  // Inicialização do WiFiClientSecure (para broker com TLS)
  wifi_client.setInsecure();

  // Configuração dos Pinos
  dht.begin();
  pinMode(led, OUTPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Configuração dos Pinos do LED RGB
  pinMode(LED_RGB_R, OUTPUT);
  pinMode(LED_RGB_G, OUTPUT);
  pinMode(LED_RGB_B, OUTPUT);
  
  // Desliga o LED RGB inicialmente
  digitalWrite(LED_RGB_R, HIGH); // Assumindo LED RGB é Anodo Comum (HIGH=OFF)
  digitalWrite(LED_RGB_G, HIGH);
  digitalWrite(LED_RGB_B, HIGH);

  // Conexão WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.println("Conectando no WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
  Serial.println("\nConectado com sucesso!");

  // Conexão MQTT
  mqtt.setServer(BROKER_URL, BROKER_PORT);
  String clientID = "S1-";
  clientID += String(random(0xffff), HEX);
  Serial.println("Conectando ao broker...");

  while (!mqtt.connect(clientID.c_str(), BROKER_USR_ID, BROKER_USR_PASS)) {
    Serial.print(".");
    delay(200);
  }
  Serial.println("\nConectado ao broker!");

  // Subscrição a Tópicos (Ajustar conforme o que S1 deve receber)
  // Assumi que S1 não precisa subscrever a seus próprios tópicos de dados de sensor
  // Se S1 recebe comandos, eles devem ser subscritos aqui.
  // mqtt.subscribe(TOPIC_COMANDO_S1);
}

// Função para controlar o LED RGB (Anodo Comum)
void setRgbColor(int r, int g, int b) {
  digitalWrite(LED_RGB_R, !r); // !r, !g, !b para Anodo Comum
  digitalWrite(LED_RGB_G, !g);
  digitalWrite(LED_RGB_B, !b);
}


void loop() {
  mqtt.loop();

  // --- Leitura e Publicação de Luminosidade (LDR) ---
  int luz_analog = analogRead(pinLDR);
  int luz_percent = map(luz_analog, 0, 4095, 0, 100); // 4095 é o máximo para o ADC do ESP32 (12-bit)
  
  if (luz_percent > 50) {
    mqtt.publish(TOPIC_LUMI_1, "Claro");
    digitalWrite(led, LOW); // Desliga o LED de iluminação se estiver claro (LOW se for Active-LOW/Anodo Comum)
    setRgbColor(0, 1, 0); // Verde (indicador de "Claro")
  } else {
    mqtt.publish(TOPIC_LUMI_1, "Escuro");
    digitalWrite(led, HIGH); // Liga o LED de iluminação se estiver escuro (HIGH se for Active-HIGH/Catodo Comum)
    setRgbColor(0, 0, 1); // Azul (indicador de "Escuro")
  }

  delay(500);
  mqtt.loop();

  // --- Leitura e Publicação de Temperatura (DHT) ---
  float t = dht.readTemperature();
  if (!isnan(t)) {
    String tempString = String(t, 1);
    mqtt.publish(TOPIC_TEMP_1, tempString.c_str());
  }

  delay(500);
  mqtt.loop();

  // --- Leitura e Publicação de Umidade (DHT) ---
  float h = dht.readHumidity();
  if (!isnan(h)) {
    String umidString = String(h, 1);
    mqtt.publish(TOPIC_UMID_1, umidString.c_str());
  }

  delay(500);
  mqtt.loop();

  // --- Leitura e Publicação de Presença (Ultrassônico) ---
  long distancia = lerDistancia();

  if (distancia > 0 && distancia < 50) { // Considera presença se a distância for menor que 50 cm
    mqtt.publish(TOPIC_PRESENCE_1, "Presente");
    // Você pode adicionar uma cor de LED RGB para presença aqui, por exemplo, Vermelho
    // setRgbColor(1, 0, 0); 
  } else {
    mqtt.publish(TOPIC_PRESENCE_1, "Ausente");
  }

  delay(500); // Espera 500ms entre as leituras/publicações para o ciclo completo
}