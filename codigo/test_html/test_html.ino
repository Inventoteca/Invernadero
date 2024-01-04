#include <WiFi.h>
#include "index.h"
const char *ssid = "Invernadero"; // Nombre del punto de acceso
const char *password = "12345678"; // Contraseña del punto de acceso
WiFiServer server(80); // objeto server

void setup() {
  Serial.begin(115200);

  WiFi.softAP(ssid, password);
  IPAddress ip = WiFi.softAPIP();
  Serial.print("Punto de acceso IP: ");
  Serial.println(ip);
  server.begin();
}

void loop() {
  
}
