#include <WiFi.h>
#include <DHT.h>

const char *ssid = "ESP32-AP";    // Nombre del punto de acceso
const char *password = "123456789";  // Contraseña del punto de acceso

WiFiServer server(80);

#define DHTPIN 4  // El pin al que está conectado el pin de datos del DHT22
#define DHTTYPE DHT22  // Tipo de sensor DHT, podría ser DHT11 también

#define BOMBA_PIN 12  // Pin para la bomba
#define HUMIDIFICADOR_PIN 13  // Pin para el humidificador
#define VENTILADOR_PIN_1 15  // Pin para el ventilador 1
#define VENTILADOR_PIN_2 14   // Pin para el ventilador 2
#define RGB_PIN_R 5  // Pin para el componente R de la tira RGB
#define RGB_PIN_G 18 // Pin para el componente G de la tira RGB
#define RGB_PIN_B 19 // Pin para el componente B de la tira RGB

DHT dht(DHTPIN, DHTTYPE);

float temperatura = 0.0;
int humedad = 0.0;
int humedadSuelo = 30;
bool bombaEncendida = false;
bool humidificadorEncendido = false;
int sliderValues[5] = {0, 0, 0, 0, 0};

void sendResponse(WiFiClient &client, const String &response) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type: text/html");
  client.println("Connection: close");
  client.println();
  client.println(response);
}

void handleDataRequest(WiFiClient &client) {
  String response = "{";
  response += "\"temperatura\":" + String(temperatura) + ",";
  response += "\"humedad\":" + String(humedad) + ",";
  response += "\"humedadSuelo\":" + String(humedadSuelo) + ",";
  response += "\"bombaEncendida\":" + String(bombaEncendida ? "true" : "false") + ",";
  response += "\"humidificadorEncendido\":" + String(humidificadorEncendido ? "true" : "false") + ",";
  response += "\"sliderValues\":[" + String(sliderValues[0]);
  for (int i = 1; i < 5; i++) {
    response += "," + String(sliderValues[i]);
  }
  response += "]}";

  sendResponse(client, response);
}

void handleControlRequest(WiFiClient &client, String device, String state) {
  if (device == "bomba") {
    bombaEncendida = (state == "true");
    digitalWrite(BOMBA_PIN, bombaEncendida ? HIGH : LOW);
  } else if (device == "humidificador") {
    humidificadorEncendido = (state == "true");
    digitalWrite(HUMIDIFICADOR_PIN, humidificadorEncendido ? HIGH : LOW);
  } else if (device == "Ventilador_1") {
    sliderValues[0] = state.toInt();
    Serial.println(sliderValues[0]);
    analogWrite(VENTILADOR_PIN_1, map(sliderValues[0], 0, 10, 0, 255));
  } else if (device == "Ventilador_2") {
    sliderValues[1] = state.toInt();
    analogWrite(VENTILADOR_PIN_2, map(sliderValues[1], 0, 10, 0, 255));
  } else if (device == "R") {
    sliderValues[2] = state.toInt();
    analogWrite(RGB_PIN_R, map(sliderValues[2], 0, 10, 0, 255));
  } else if (device == "G") {
    sliderValues[3] = state.toInt();
    analogWrite(RGB_PIN_G, map(sliderValues[3], 0, 10, 0, 255));
  } else if (device == "B") {
    sliderValues[4] = state.toInt();
    analogWrite(RGB_PIN_B, map(sliderValues[4], 0, 10, 0, 255));
  }

  sendResponse(client, "OK");
}

void setup() {
  Serial.begin(115200);
  pinMode(A0, INPUT); // Pin analógico A0 para el sensor de humedad del suelo
  pinMode(BOMBA_PIN, OUTPUT);
  pinMode(HUMIDIFICADOR_PIN, OUTPUT);
  pinMode(VENTILADOR_PIN_1, OUTPUT);
  pinMode(VENTILADOR_PIN_2, OUTPUT);
  pinMode(RGB_PIN_R, OUTPUT);
  pinMode(RGB_PIN_G, OUTPUT);
  pinMode(RGB_PIN_B, OUTPUT);

  dht.begin();

  // Inicializar el punto de acceso
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("Punto de acceso IP: ");
  Serial.println(IP);

  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("Nuevo cliente");
    String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<style>.slider { width: 50%; }</style></head><body>";

    // Actualizar datos del DHT22
    temperatura = dht.readTemperature();
    humedad = dht.readHumidity();

    // Manejar solicitud de datos JSON
    if (client.available()) {
      String request = client.readStringUntil('\r');
      if (request.indexOf("/datos") != -1) {
        handleDataRequest(client);
        client.stop();
        return;
      } else if (request.indexOf("/control/") != -1) {
        // Manejar solicitud de control
        int start = request.indexOf("/control/") + 9;
        int end = request.indexOf("/", start);
        String device = request.substring(start, end);
        start = end + 1;
        end = request.indexOf(" ", start);
        String state = request.substring(start, end);
        handleControlRequest(client, device, state);
        client.stop();
        return;
      }
    }

    html += "<div style='float: left; width: 100%; text-align: center;'><h2>Humedad</h2><p id='humedad'>";
    html += String(humedad);
    html += "%</p></div>";

    // Segundo recuadro (Temperatura)
    html += "<div style='float: left; width: 100%; text-align: center;'><h2>Temperatura</h2><p id='temperatura'>";
    html += String(temperatura);
    html += "C</p></div>";

    // Tercer recuadro (Humedad del suelo)
    html += "<div style='float: left; width: 100%; text-align: center;'><h2>Humedad del Suelo</h2><p id='humedadSuelo'>";
    html += String(humedadSuelo);
    html += "%</p></div>";

    // Cuarto recuadro (Toggle para la bomba)
    html += "<div style='float: left; width: 100%; text-align: center;'><h2>Bomba</h2>";
    html += "<label class='switch'><input type='checkbox' id='toggleBomba' ";
    html += bombaEncendida ? "checked" : "";
    html += " onchange='toggleBomba()'><span class='slider round'></span></label></div>";

    // Quinto recuadro (Toggle para el humidificador)
    html += "<div style='float: left; width: 100%; text-align: center;'><h2>Humidificador</h2>";
    html += "<label class='switch'><input type='checkbox' id='toggleHumidificador' ";
    html += humidificadorEncendido ? "checked" : "";
    html += " onchange='toggleHumidificador()'><span class='slider round'></span></label></div>";

    // Sliders
    const char *sliderNames[5] = {"Ventilador_1", "Ventilador_2", "R", "G", "B"};
    for (int i = 0; i < 5; i++) {
      html += "<div style='float: left; width: 100%; text-align: center;'>";
      html += "<h2>" + String(sliderNames[i]) + ": <span id='value" + String(sliderNames[i]) + "'>" + String(sliderValues[i]) + "</span></h2>";
      html += "<input type='range' id='" + String(sliderNames[i]) + "' class='slider' min='0' max='10' value='" + String(sliderValues[i]) + "' oninput='updateSliderValue(\"" + String(sliderNames[i]) + "\", \"value" + String(sliderNames[i]) + "\")'></div>";
    }

    html += "<script>";
    html += "function toggleBomba() { var estado = document.getElementById('toggleBomba').checked; enviarDatos('bomba', estado); }";
    html += "function toggleHumidificador() { var estado = document.getElementById('toggleHumidificador').checked; enviarDatos('humidificador', estado); }";
    html += "function updateSliderValue(sliderId, valueId) { var valor = document.getElementById(sliderId).value; document.getElementById(valueId).innerHTML = valor; enviarDatos(sliderId, valor); }";
    html += "function updateVariable(variable, valor) { document.getElementById('input' + variable).value = valor; enviarDatos(variable, valor); }";
    html += "function enviarDatos(dispositivo, estado) { var xhttp = new XMLHttpRequest(); xhttp.open('GET', '/control/' + dispositivo + '/' + estado, true); xhttp.send(); }";
    html += "function actualizarDatos() { var xhttp = new XMLHttpRequest(); xhttp.open('GET', '/datos', true); xhttp.onload = function() { var datos = JSON.parse(this.responseText); document.getElementById('temperatura').innerHTML = datos.temperatura; document.getElementById('humedad').innerHTML = datos.humedad; document.getElementById('humedadSuelo').innerHTML = datos.humedadSuelo; document.getElementById('toggleBomba').checked = datos.bombaEncendida; document.getElementById('toggleHumidificador').checked = datos.humidificadorEncendido; for (var i = 0; i < 5; i++) { document.getElementById(sliderNames[i]).value = datos.sliderValues[i]; document.getElementById('value' + sliderNames[i]).innerHTML = datos.sliderValues[i]; } }; xhttp.send(); }";
    html += "setInterval(function() { actualizarDatos(); }, 2000);";  // Actualizar datos cada 2 segundos
    html += "</script></body></html>";

    sendResponse(client, html);
  }

  // Tu código principal aquí
}
