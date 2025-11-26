#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "env.h"

WiFiClientSecure wifiClient;
PubSubClient mqtt(wifiClient);

// Ponte-H
const byte dir1 = 33;  
const byte dir2 = 25;

// Pinos LED RGB
const byte vermelhinho = 5;
const byte verdinho    = 18;
const byte azulzinho   = 19;

// Configuração dos PWMs do LED
const uint8_t CH_R = 0;
const uint8_t CH_G = 1;
const uint8_t CH_B = 2;
const uint32_t PWM_FREQ = 5000;
const uint8_t PWM_RES  = 8;

// Declaração das funções - boas práticas:
void statusLED(byte status);
void turnOffLEDs();
void setLEDColor(byte r, byte g, byte b);
void callback(char* topic, byte* payload, unsigned int length);
void connectToWifi();
void connectToMQTT();
void reconnectMQTT(); // nova função — reconecta infinitamente

void statusLED(byte status) {
  // Desliga primeiro:
  turnOffLEDs();

  switch (status) {
    case 254:  // Erro geral — deixa vermelho
      setLEDColor(255, 0, 0);
      break;

    case 1:  // Conectando ao Wi-Fi — amarelinho
      setLEDColor(150, 255, 0);
      break;

    case 2:  // Conectando ao MQTT — rosinha
      setLEDColor(150, 0, 255);
      break;

    case 3:  // Indo pra frente — verde
      setLEDColor(0, 255, 0);
      break;

    case 4:  // Indo pra trás — ciano
      setLEDColor(0, 255, 255);
      break;

    case 0:  
      break;

    default:  
      for (byte i = 0; i < 4; i++) {
        setLEDColor(0, 0, 255);
        delay(100);
        turnOffLEDs();
        delay(100);
      }
      break;
  }
}

void turnOffLEDs() {
  setLEDColor(0, 0, 0);
}

void setLEDColor(byte r, byte g, byte b) {
  // Valores PWM vão direto pros canais
  ledcWrite(CH_R, r);
  ledcWrite(CH_G, g);
  ledcWrite(CH_B, b);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida em ");
  Serial.println(topic);

  String msg((char*)payload, length);

  // Só responde se o tópico for o certo:
  if (strcmp(topic, "in/S4") == 0) {
    if (!msg.startsWith("speed")) return;
    int valor = msg.substring(6).toInt();

    Serial.print("Velocidade recebida: ");
    Serial.println(valor);

    // Lógica da direção
    if (valor > 0) {
      // Vai para frente
      digitalWrite(dir1, HIGH);
      digitalWrite(dir2, LOW);
      statusLED(3);

    } else if (valor < 0) {
      // Vai para trás
      digitalWrite(dir1, LOW);
      digitalWrite(dir2, HIGH);
      statusLED(4);

    } else {
      digitalWrite(dir1, LOW);
      digitalWrite(dir2, LOW);
      statusLED(0);
    }
  }
}

// Função para conectar Wi-Fi:
void connectToWifi() {
  statusLED(1);  // mostrando que está tentando Wi-Fi
  delay(500);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.println("Conectando à Internet...");

  // Loop infinito até conectar
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Ainda tentando conectar ao Wi-Fi...");
    statusLED(1);
    delay(500);
  }

  Serial.println("\nWi-Fi conectado!");
  statusLED(0);
}

// Função para conectar ao Wifi:
void connectToMQTT() {
  statusLED(2);  
  delay(500);

  mqtt.setServer(BROKER_URL, BROKER_PORT);
  mqtt.setCallback(callback);

  Serial.println("Conectando ao Broker MQTT...");
  
  while (!mqtt.connected()) {
    String clientID = "TREM-";
    clientID += String(random(0xffff), HEX);

    if (mqtt.connect(clientID.c_str())) {
      Serial.println("Conectado ao Broker!");
      mqtt.subscribe(TOPIC_SPEED);
      statusLED(0);
      break;

    } else {
      Serial.println("Falha ao conectar no MQTT. Tentando de novo...");
      statusLED(254); // LED vermelho indicando problema
      delay(2000);
    }
  }
}

// Função para reconectar com o broker:
void reconnectMQTT() {
  Serial.println("MQTT desconectado, tentando de novo...");
  
  while (!mqtt.connected()) {
    String clientID = "TREM-";
    clientID += String(random(0xffff), HEX);

    if (mqtt.connect(clientID.c_str())) {
      Serial.println("Reconectado ao Broker!");
      mqtt.subscribe(TOPIC_SPEED);
      statusLED(0);
      break;

    } else {
      Serial.println("Tentando novamente. Aguarde.");
      statusLED(254);
      delay(2000);
    }
  }
}

// -------------------------
void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0));
  wifiClient.setInsecure(); 
  
  pinMode(dir1, OUTPUT);
  pinMode(dir2, OUTPUT);

  // Setup dos PWMs do LED RGB
  ledcSetup(CH_R, PWM_FREQ, PWM_RES);
  ledcAttachPin(vermelhinho, CH_R);

  ledcSetup(CH_G, PWM_FREQ, PWM_RES);
  ledcAttachPin(verdinho, CH_G);

  ledcSetup(CH_B, PWM_FREQ, PWM_RES);
  ledcAttachPin(azulzinho, CH_B);

  connectToWifi();
  connectToMQTT();
}

void loop() {
  // Se falhar, reconecta
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi caiu, reconectando...");
    connectToWifi();
  }

  // Se falhar, reconecta
  if (!mqtt.connected()) {
    reconnectMQTT();
  }

  mqtt.loop();
}
