#include <WiFi.h>
#include "index.h" //contiene código html como raw string
const char *ssid = "Invernadero"; // Nombre del punto de acceso
const char *password = "123456789"; // Contraseña del punto de acceso
WiFiServer server(80); // objeto server

//array para enviar los valores de los sliders
//int sliderValues[5] = {0, 0, 0, 0, 0};
float sliderValues[8]; //= {0, 0, 0, 0, 0};

//------------------------------------------------------------------------------------------
// Respuesta que se envía al cliente cuando este hace una petición
void sendResponse(WiFiClient &client, const String &response) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type: text/html");
  client.println("Connection: close");
  client.println();
  client.println(response);
  Serial.println(response);
}

//------------------------------------------------------------------------------------------
// Responder a peitición de datos por parte del cliente
// Se envía una string con valores de los sensores y los actuadores
void handleDataRequest(WiFiClient &client) {
  String response = "{";
  response += "\"temperatura\":" + String(temperatura, 0) + ",";
  response += "\"humedad\":" + String(humedad, 0) + ",";
  response += "\"humedadSuelo\":" + String(humedad_suelo) + ",";
  response += "\"bomba\":" + String(bomba ? "true" : "false") + ",";
  response += "\"spray\":" + String(spray ? "true" : "false") + ",";
  response += "\"sliderValues\":[" + String(sliderValues[0], 0);
  for (int i = 1; i < 8; i++) {
    response += "," + String(sliderValues[i]);
  }
  response += "]}";

  sendResponse(client, response);
} //fin handleDataRequest

//------------------------------------------------------------------------------------------
// Responder a solicitud de control por parte del cliente
// Cambiar las salidas de acuerdo a los valores del web server
void handleControlRequest(WiFiClient &client, String device, String state) {
  if (device == "bomba") {
    bomba = (state == "true");
    bomba_auto = false; //bomba manual
    digitalWrite(PIN_BOMBA, bomba ? LOW : HIGH); //se activa en nivel bajo
  }

  else if (device == "spray") {
    bool nuevo_estado = (state == "true");
    //Serial.println("clic spray");
    spray_auto = false; //spray/nebulizador manual
    //spray = not spray; //invertir estado
    if (nuevo_estado != spray) {//si se quiere cambiar el estado
      digitalWrite(PIN_SPRAY, LOW); //recuerda, el relevador se activa en bajo
      delay(T_PULSO_SPRAY); //tiempo que tarda el pulso
      digitalWrite(PIN_SPRAY, HIGH); //desactivar relevador
      //tft.drawXBitmap(bix0[5], biy0[5], bi0[5], 32, 32, spray ? TFT_CYAN : 0); //cambiar color del icono
      //Serial.println(spray ? "Spray ON (manual)" : "Spray OFF (manual)");
      spray = nuevo_estado;
    }
  }

  else if (device == "vent1") {
    sliderValues[0] = state.toInt();
    vent1 = sliderValues[0];
    //Serial.println(sliderValues[0]);
    analogWrite(PIN_VENT1, map(sliderValues[0], 0, 10, 0, 255));
    vent_auto = false;
  }

  else if (device == "vent2") {
    sliderValues[1] = state.toInt();
    vent2 = sliderValues[1];
    analogWrite(PIN_VENT2, map(sliderValues[1], 0, 10, 0, 255));
    vent_auto = false;
  }

  else if (device == "R") {
    sliderValues[2] = state.toInt();
    val_r = sliderValues[2];
    analogWrite(PIN_R, mapfloat(sliderValues[2], 0, 10, 0, 255));
  } else if (device == "G") {
    sliderValues[3] = state.toInt();
    val_g = sliderValues[3];
    analogWrite(PIN_G, mapfloat(sliderValues[3], 0, 10, 0, 255));
  } else if (device == "B") {
    sliderValues[4] = state.toInt();
    val_b = sliderValues[4];
    analogWrite(PIN_B, mapfloat(sliderValues[4], 0, 10, 0, 255));
  }

  else if (device == "limtemp") {//limite de temperatura
    sliderValues[5] = state.toInt();
    lim_temperatura = sliderValues[5];
    //analogWrite(PIN_VENT2, map(sliderValues[1], 0, 10, 0, 255));
    vent_auto = true; //cambiar a modo automatico
  }

  else if (device == "limhum") {//limite de humedad
    sliderValues[6] = state.toInt();
    lim_humedad = sliderValues[6];
    //analogWrite(PIN_VENT2, map(sliderValues[1], 0, 10, 0, 255));
    spray_auto = true; //cambiar a modo automatico
  }

  else if (device == "limhums") {//limite de humedad del suelo
    sliderValues[7] = state.toInt();
    lim_humedad_suelo = sliderValues[7];
    //analogWrite(PIN_VENT2, map(sliderValues[1], 0, 10, 0, 255));
    bomba_auto = true; //cambiar a modo automatico
  }

  sendResponse(client, "OK");
} //fin handleControlRequest

//------------------------------------------------------------------------------------------
// Inicializar server
void serverSetup() {
  WiFi.softAP(ssid, password);
  IPAddress ip = WiFi.softAPIP();
  Serial.print("Punto de acceso IP: ");
  Serial.println(ip);
  server.begin();
} //fin serverSetup

//------------------------------------------------------------------------------------------
// Código que se repite cuando un cliente se conecta (o hace una petición)
void serverLoop() {
  WiFiClient client = server.available(); //ver conexión de clientes
  if (client) { //si se conecta un cliente
    Serial.println("Cliente conectado"); //enviar mensaje por serial

    // Manejar la solicitud de datos
    if (client.available()) {
      String request = client.readStringUntil('\r'); //obtener petición del cliente
      //String request = client.readString();
      Serial.println(request); //imprimir la petición
      if (request.indexOf("/datos") >= 0) { //si la petición tiene "datos"
        sliderValues[0] = vent1; //actualizar array de slider values
        sliderValues[1] = vent2;
        sliderValues[2] = val_r;
        sliderValues[3] = val_g;
        sliderValues[4] = val_b;
        sliderValues[5] = lim_temperatura;
        sliderValues[6] = lim_humedad;
        sliderValues[7] = lim_humedad_suelo;
        handleDataRequest(client); //se envían los datos
        client.stop(); //cerrar la conexión
        return;
      } else if (request.indexOf("/control/") >= 0) { //si la petición tiene "control"
        // Manejar solicitud de control
        int start = request.indexOf("/control/") + 9;
        int end = request.indexOf("/", start);
        String device = request.substring(start, end); //extraer string con el nombre del dispositivo
        start = end + 1;
        end = request.indexOf(" ", start);
        String state = request.substring(start, end); //después extraer string con el estado del dispositivo
        handleControlRequest(client, device, state); //actualizar salidas con los valores
        client.stop(); //cerrar la conexión
        return;
      }
    }

    // El server envía un código de respuesta, el tipo de contenido
    // que recibirá el cliente, una línea vacía y después el contenido
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type: text/html");
    client.println("Connection: close");
    client.println();
    client.println(html); //enviar página web
    client.stop();
  } //fin if client
} //fin serverLoop
