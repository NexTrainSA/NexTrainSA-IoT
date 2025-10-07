#include <Wifi.h>

WifiClient client;
const String SSID = "Nome da rede";
const String PASS = "Senha da rede";

void setup() {
  Serial.begin(115200);
  client.begin(SSID, PASS); 
  Serial.println("Conectando...");

  while(client.status() != WL_CONNECTED) { // Ou seja, não está conectado
    Serial.print("...");
    delay(200);
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
