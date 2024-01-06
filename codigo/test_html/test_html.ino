#include <WiFi.h>
#include "index.h"
const char *ssid = "Invernadero"; // Nombre del punto de acceso
const char *password = "123456789"; // Contraseña del punto de acceso
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
  WiFiClient client = server.available(); //ver conexión de clientes
  if (client) { //si se conecta un cliente
    Serial.println("Cliente conectado"); //enviar mensaje por serial

    // Si el cliente hizo una petición, leer la string y mostrarla por serial
    if (client.available()) {
      String request = client.readStringUntil('\r');
      //String request = client.readString();
      Serial.println(request);
    }

    // El server envía un código de respuesta, el tipo de contenido
    // que recibirá el cliente, una línea vacía y después el contenido
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type: text/html");
    client.println("Connection: close");
    client.println();
    client.println(html); //enviar página web
    //client.stop();
  }
}
