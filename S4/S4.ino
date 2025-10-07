#include <Wifi.h>
WiFiClient cliente;
const String SSID = "nome da rede";
const String PASS= "senha da rede";

void setup() {
  Serial.begin(115200);
  client.begin(nome,senha);
  Serial.println("Concetando no Wifi");
  while(cliente.status()!= WL_CONNECTED){
    Serial.print(".");
    delay(200);
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
