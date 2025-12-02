
//Inclusão das bibliotecas necessárias
#include <WiFi.h>
#include <PubSubClient.h>//para a conexão com o MQTT 
#include <DHT.h> //para leitura do sensor de temperatura e umidade
#include <ArduinoJson.h> //para formatar os dados dos sensores em JSON facilitando a troca das informações
#include "env.h"
//Configurações de rede e mqtt 
const char* ssid = "Wokwi-GUEST"; //CREDENCIAIS DE CONEXÃO
const char* password = "";

// MQTT 
const char* mqtt_server = "179.222.224.26"; // Change to your MQTT broker
const int mqtt_port = 6883;
const char* mqtt_topic_out = "out/S1";
const char* mqtt_topic_in = "input/S1";

// Definições dos pinos necessários DHT, DHT22, Fotoresistor(LDR), Pino digital, LED e LED RGB
#define DHT_PIN 4 //temp/umid 
#define LDR_PIN 34 //luminos
#define PIR_PIN 5 //presença
#define LED_PIN 2 //LED simples
#define LED_R_PIN 25//r
#define LED_G_PIN 26//g
#define LED_B_PIN 27//b

// DHT configurações do sensor
#define DHTTYPE DHT22
DHT dht(DHT_PIN, DHTTYPE);

// WiFi and MQTT clients
WiFiClient espClient;
PubSubClient client(espClient);

// Variaveis de estado
float temperature = 0; //variavel armazenar temperatura
float humidity = 0; //umidade 
int luminosity = 0; //luminosidade
int presence = 0; //estado de presença
bool ledState = false; //estado do LED (ligado, desligado)
String ledRgbState = "Desligado"; //estado descritivo RGB

//variaveis para controlar o tempo (temporização)

unsigned long lastMsg = 0;
const long interval = 2000; // 2 segundos para envio de dados

//Função setup_wifi() tem o propósito de conectar o ESP32 à rede wifi

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  //Aguarda a conexão ser estabelecida, exibe pontos de pogresso. 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  //Conexão bem-sucedida 
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//Função callback com o propósito de processar as mensagens recebidas do broker MQTT. Lida com as mensagens recebidas. Ela é chamada quando uma mensagem chega ao tópico assinado (mqtt_topic_in)

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(message);

  //Comando é recebido para testar a conectividade (ping/pong)

    if(message == "ping") {
      client.publish(mqtt_topic_out, "pong"); //publica a resposta "pong"
    }
    //verifica o comando pra ligar/desligar 
    if (message == "TOGGLE_LED") {
      ledState = !ledState; //inverte o estado atual do LED 
      digitalWrite(LED_PIN, ledState ? HIGH : LOW);
      
      // Alterna o status do LED RGB
      if (ledState) {
        ledRgbState = "Ligado";
        //Liga o LED RGB
        digitalWrite(LED_R_PIN, HIGH);
        digitalWrite(LED_G_PIN, HIGH);
        digitalWrite(LED_B_PIN, HIGH);
      } else {
        ledRgbState = "Desligado";
        digitalWrite(LED_R_PIN, LOW);
        digitalWrite(LED_G_PIN, LOW);
        digitalWrite(LED_B_PIN, LOW);
      }
  }
}

//Função reconnect () com propósito que tentar reconectar o broker MQTT se caso a conexão cair. Manter a conexão continua ao brokerMQTT 

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32_S1_";
    // Cria um ID de cliente único
    clientId += String(random(0xffff), HEX);
    
    // Tenta conectar, usando o ID do cliente
    if (client.connect(clientId.c_str(), "nextrain", "nextrain")) {
      Serial.println("connected");
      client.subscribe(mqtt_topic_in);
    } else {
      // Loga o erro e tenta novamente em 5 segundos.
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

//Função readSensors() com o objetivo de ler o valor atual de todos os sensores. Obter os dados atuais dos sensores. 
void readSensors() {
  // Lê o sensor DHT22.
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  
  // Lê o LDR (fotoresistor).
  int ldrValue = analogRead(LDR_PIN);
  luminosity = map(ldrValue, 0, 4095, 0, 1000); // Convert to lux approximation
  
  // Lê o sensor PIR (presença). Retorna HIGH (1) ou LOW (0).
  presence = digitalRead(PIR_PIN);
  
  // Verifica se houve erro na leitura do DHT.
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    temperature = 0;
    humidity = 0;
  }
}

//Função publishData() com o propósito de formatar os dados em JSON e publica no tópico MQTT. Enviar dados ao Broker. 
void publishData() {
  //Cria um documento JSON estático com capacidade máxima de 300 bytes 
  StaticJsonDocument<300> doc;
  // Popula o documento JSON com os dados dos sensores e o estado do LED.
  doc["TEMPERATURE"] = String(temperature, 1);
  doc["HUMIDITY"] = String(humidity, 1);
  doc["LUMINOSITY"] = String(luminosity);
  doc["PRESENCE"] = String(presence);
  doc["LED_STATE"] = ledState ? "Ligado" : "Desligado";
  doc["LED_RGB"] = ledRgbState;
  
  char jsonBuffer[300];
  // converte o objeto JSON para uma string de caracteres.
  serializeJson(doc, jsonBuffer);
  // Publica a string JSON no tópico de saída.
  client.publish(mqtt_topic_out, jsonBuffer);
  
  Serial.println("Published data:");
  Serial.println(jsonBuffer);
}

//Função setup() propósito das configurações iniciais, execurta uma única vez ao ligar o ESP32
void setup() {
  Serial.begin(115200); //Inicia a comunicação serial para debug 
  
  // Inicia os pinos de OUTPUT/INPUT 
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_R_PIN, OUTPUT);
  pinMode(LED_G_PIN, OUTPUT);
  pinMode(LED_B_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  
  // Desliga todos os LEDs no início
  digitalWrite(LED_PIN, LOW);
  digitalWrite(LED_R_PIN, LOW);
  digitalWrite(LED_G_PIN, LOW);
  digitalWrite(LED_B_PIN, LOW);
  
  // Inicializa o sensor DHT
  dht.begin();
  
  // Conecta ao WiFi
  setup_wifi();
  
  // Configura o MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  
  Serial.println("S1 Station initialized!");
}

//Função loop contém uma lógica principal executada repetidamente e de forma rápida. Verifica se  MQTT esta conectado se nao tiver chama o reconnect. client loop para manter conexão MQTT. 
void loop() {
  //verifica a conexão MQTT e tenta reconectar se necessário
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  // Lógica de temporização para envio de dados
  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;// Atualiza o último momento em que a mensagem foi enviada.
    
    readSensors();// Lê os sensores.
    publishData();// Publica os dados via MQTT.
  }
}