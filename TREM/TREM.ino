// Bibliotecas:
#include <ESP32Servo.h>


// Definição de variáveis:

// LEDs:
const byte ledVerde = 2;
const byte ledVermelho = 4;

// Para o sensor:
const byte echoPin = 35;
const byte triggPin = 27;
float distancia = 0;
unsigned long tempo = 0;

// Para o botão:
const byte botao = 18;

void setup() {
  // Setup para o sensor:
  pinMode(echoPin, INPUT);
  pinMode(triggPin, OUTPUT);

  // LEDs:
  pinMode (ledVerde, OUTPUT);
  pinMode (ledVermelho, OUTPUT);

  // Botão:
  pinMode(botao, INPUT_PULLUP);

  Serial.begin(9600);
}


void loop() {
  // Ler botão:
  bool botaoSimNao = lerBotao();
  controlarBotao(botaoSimNao);  
}

bool lerBotao() {
  return digitalRead(botao) == LOW; // LOW para botão pressionado
}

void controlarBotao(bool estadoBotao) {
  if (estadoBotao) {
    // Serial.println(estadoBotao);
    digitalWrite(ledVerde, HIGH);
     // Medir distância:
    // Envia som:
    digitalWrite(triggPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggPin, LOW);

    // Volta som:
    tempo = pulseIn(echoPin, HIGH);
    distancia = (tempo * 0.0343) / 2;
    Serial.println(distancia);

    if (distancia < 50) {
      digitalWrite(ledVerde, LOW);
      digitalWrite(ledVermelho, HIGH);
      delay(200);
      digitalWrite(ledVermelho, LOW);
      delay(200);
    } 
  } else {
    digitalWrite(ledVerde, LOW);
  }
}
