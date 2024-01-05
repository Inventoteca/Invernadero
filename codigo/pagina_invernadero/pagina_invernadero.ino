#include <WiFi.h>
const char *ssid = "Invernadero";    // Nombre del punto de acceso
const char *password = "123456789";  // Contraseña del punto de acceso
WiFiServer server(80); // objeto server

#include <DHT.h>
#define DHTPIN 15  // El pin al que está conectado el pin de datos del DHT22
#define DHTTYPE DHT22  // Tipo de sensor DHT, podría ser DHT11 también
DHT dht(DHTPIN, DHTTYPE); //objeto para el sensor DHT

#define BOMBA_PIN 14  // Pin para la bomba
#define HUMIDIFICADOR_PIN 13  // Pin para el humidificador
#define VENTILADOR_PIN_1 32  // Pin para el ventilador 1
#define VENTILADOR_PIN_2 33   // Pin para el ventilador 2
#define PIN_R 26  // Pin para el componente R de la tira RGB
#define PIN_G 25 // Pin para el componente G de la tira RGB
#define PIN_B 27 // Pin para el componente B de la tira RGB
#define PIN_SUELOD 34 //lectura digital
#define PIN_SUELOA 35 //lectura analogica

float temperatura = 0.0;
int humedad = 0.0;
int humedadSuelo = 30;
bool bombaEncendida = false;
bool humidificadorEncendido = false;
int sliderValues[5] = {0, 0, 0, 0, 0};

// Respuesta que se envía al cliente cuando este hace una petición
void sendResponse(WiFiClient &client, const String &response) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type: text/html");
  client.println("Connection: close");
  client.println();
  client.println(response);
}

// String con valores de los sensores y los actuadores
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

// Cambiar las salidas de acuerdo a los valores del web server
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
    analogWrite(PIN_R, map(sliderValues[2], 0, 10, 0, 255));
  } else if (device == "G") {
    sliderValues[3] = state.toInt();
    analogWrite(PIN_G, map(sliderValues[3], 0, 10, 0, 255));
  } else if (device == "B") {
    sliderValues[4] = state.toInt();
    analogWrite(PIN_B, map(sliderValues[4], 0, 10, 0, 255));
  }

  sendResponse(client, "OK");
}

void setup() {
  // Entradas y salidas
  pinMode(PIN_SUELOD, INPUT);
  pinMode(PIN_SUELOA, INPUT);
  pinMode(BOMBA_PIN, OUTPUT);
  pinMode(HUMIDIFICADOR_PIN, OUTPUT);
  pinMode(VENTILADOR_PIN_1, OUTPUT);
  pinMode(VENTILADOR_PIN_2, OUTPUT);
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  //
  analogRead(PIN_SUELOA); //descartar primera lectura
  analogWrite(VENTILADOR_PIN_1, 0);
  analogWrite(VENTILADOR_PIN_2, 0);
  analogWrite(PIN_R, 0); //si el pin es DAC no saca PWM?
  analogWrite(PIN_G, 0);
  analogWrite(PIN_B, 0);
  digitalWrite(BOMBA_PIN, HIGH); //relevador apagado en HIGH
  digitalWrite(HUMIDIFICADOR_PIN, HIGH); //relevador apagado en HIGH
  //
  dht.begin(); //iniciar sensor
  Serial.begin(115200); //iniciar serial

  // Inicializar el punto de acceso
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("Punto de acceso IP: ");
  Serial.println(IP);

  server.begin();
}

void loop() {
  WiFiClient client = server.available(); //ver la conexión de clientes
  if (client) { //si se conecta un nuevo cliente
    Serial.println("Nuevo cliente");
    
    String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<style>.slider { width: 50%; }</style></head><body>";

    // Actualizar datos del DHT22
    temperatura = dht.readTemperature();
    humedad = dht.readHumidity();

    // Manejar solicitud de datos JSON
    if (client.available()) {
      String request = client.readStringUntil('\r'); //obtener petición del cliente
      if (request.indexOf("/datos") >= 0) { //si la petición tiene "datos"
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
  //
} //Fin loop()
