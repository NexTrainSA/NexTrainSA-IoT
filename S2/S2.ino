#include "env.h"

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Definição dos pinos
#define LED_PIN 2

const int PINO_TRIG = 4; // Pino D4 conectado ao TRIG do HC-SR04 
const int PINO_ECHO = 2; // Pino D2 conectado ao ECHO do HC-SR04 

const int PINO_TRIG2 = 16; // Pino D16 conectado ao TRIG do HC-SR04 
const int PINO_ECHO2 = 17; // Pino D17 conectado ao ECHO do HC-SR04 

const int PINO_R = 18;
const int PINO_G = 19; 
const int PINO_B = 21; 

//Variáveis
String currentColor = "NONE"; //Guarda o valor hexadecimal da cor atual do LED RGB

// WiFi and MQTT clients
WiFiClient espClient;
PubSubClient client(espClient);

//Distância medida pelo sensor
float presence2 = 0; 
float presence4 = 0; 

bool ledState = false; //Guarda se o led comum está ligado ou desligado
String ledRgbState = "Desligado";
unsigned long lastMsg = 0; //armazena o último envio MQTT
const long interval = 2000; 

String rgbToHex(int r, int g, int b) //Transforma valores RGB em uma cor HEX
{  
  long colorValue = ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF);
  char hexString[7];
  snprintf(hexString, sizeof(hexString), "%06lX", colorValue); 
  return String(hexString);
}

void setColor(int redValue, int greenValue,  int blueValue) { //Usada para controlar cada canal do LED;
  currentColor = rgbToHex(redValue, greenValue, blueValue); 

  analogWrite(PINO_R, redValue); //Escreve um valor no pino vermelho
  analogWrite(PINO_G,  greenValue);  
  analogWrite(PINO_B, blueValue);
}

void setColor(String hex) { //Converte HEX para RGB e escreve os valores nos pinos;
  if (hex.startsWith("#")) {
    hex.remove(0, 1); 
  }
  long colorValue = strtol(hex.c_str(), NULL, 16); //Converte a string HEX para um valor long

  int r = (colorValue >> 16) & 0xFF; //Extrai o byte vermelho
  int g = (colorValue >> 8) & 0xFF; 
  int b = colorValue & 0xFF; 

  analogWrite(PINO_R, r);
  analogWrite(PINO_G,  g);
  analogWrite(PINO_B, b);
}

void setup_wifi() { //Conecta ao WiFi usando SSID e senha do env.h
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);
  WiFi.begin(SSID, password); //Inicia a conexão Wi-Fi
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); //Exibe o endereço IP do ESP32
}

void callback(char* topic, byte* payload, unsigned int length) { //Recebe mensagens do MQTT enviadas ao ESP32
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i]; //Converte um array de bytes em String
  }

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(message);
  if(message == "ping") { //comando de teste de conectividade 
    client.publish(mqtt_topic_out, "pong"); //responde "pong" no tópico de saída
  }

  if (message == "TOGGLE_LED") { //Comando para alterar o estado do LED comum
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState ? HIGH : LOW); //Liga e desliga o LED
  }

  //Muda a cor do LED RGB
  if (message == "GREEN") {
    setColor(0,255,0);
  }

  if (message == "BLUE") {
    setColor(0,0,255);
  }

  if (message == "RED") {
    setColor(255,0,0);
  }

  if(message.startsWith("color/")) {
    String color = message.substring(7);
    Serial.println("Color: " + color);
    setColor(color);
  }
}

void reconnect() { //Se o ESP32 perda a conexão com o MQTT, ele tenta reconectar infinitamente
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32_S2_";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str(), "nextrain", "nextrain")) {
      Serial.println("connected");
      client.subscribe(mqtt_topic_in);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void readSensors() {    //Envia o sinal no TRIG, espera a resposta ECHO, Mede tempo de retorno e converte para distância em cm;
  digitalWrite(PINO_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PINO_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PINO_TRIG, LOW);
  long duracao1 = pulseIn(PINO_ECHO, HIGH);
  presence2 = (duracao1 * 0.0343) / 2;
  delay(20); 
  digitalWrite(PINO_TRIG2, LOW);
  delayMicroseconds(2);
  digitalWrite(PINO_TRIG2, HIGH);
  delayMicroseconds(10);
  digitalWrite(PINO_TRIG2, LOW);
  long duracao2 = pulseIn(PINO_ECHO2, HIGH);
  presence4 = (duracao2 * 0.0343) / 2;
}

void publishData() {  //Envio de dados via MQTT
  StaticJsonDocument<300> doc;

  doc["PRESENCE2"] = String(presence2); //Distância do sensor 1;
  doc["PRESENCE4"] = String(presence4); 
  doc["LED_STATE"] = ledState ? "Ligado" : "Desligado"; 
  doc["LED_RGB"] = currentColor; //Cor atual (HEX)

  char jsonBuffer[300];
  serializeJson(doc, jsonBuffer); //Converte Json para texto
  client.publish(mqtt_topic_out, jsonBuffer); //envia o texto pelo MQTT
  Serial.println("Published data:");
  Serial.println(jsonBuffer);
}

void setup() {
  Serial.begin(115200);

  // Inicializa os pinos
  pinMode(LED_PIN, OUTPUT);
  pinMode(PINO_TRIG, OUTPUT);
  pinMode(PINO_ECHO, INPUT);
  pinMode(PINO_TRIG2, OUTPUT);
  pinMode(PINO_ECHO2, INPUT);
  pinMode(PINO_R,  OUTPUT);              
  pinMode(PINO_G, OUTPUT);
  pinMode(PINO_B, OUTPUT);

  // Desliga todos os leds 
  digitalWrite(LED_PIN, LOW);
  digitalWrite(PINO_R, LOW);
  digitalWrite(PINO_G, LOW);
  digitalWrite(PINO_B, LOW);

  // Connect to WiFi
  setup_wifi();

  // Configure MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback); //Define qual a função que será chamada quando uma mensagem chegar de um tópico
  Serial.println("S2 Station initialized!");
}

void loop() {
  if (!client.connected()) { //Verifica se o ESP32 está conectado ao broker MQTT
    reconnect(); //Caso não esteja conectado, a função reconnect é chamada
  }

  client.loop();
  unsigned long now = millis(); //Retorna o tempo em milissegundos 
  if (now - lastMsg > interval) {
    lastMsg = now;//lastMsg momento da última mensagem enviada
    readSensors(); //Lê os sensores de temperatura e umidade.
    publishData(); //Função enviada para os dados lidos para o MQTT
  }
}
