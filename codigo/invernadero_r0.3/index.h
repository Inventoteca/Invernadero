const char *html = R"rawString(
<!DOCTYPE html>
<html>

<head>
  <meta charset="UTF-8">
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <title>Micro-Invernadero</title>
  <style>
    body {
      font-family: sans-serif;
      color: lightgray;
      background-color: #101021;
      text-align: center;
    }

    div {
      margin-left: 5%;
      margin-right: 5%;
      background-color: #524d6b;
      padding: 10px;
      border-radius: 10px;
    }

    .slider {
      width: 70%;
    }

    .negro {
      margin-left: 25%;
      margin-right: 25%;
      background-color: black;
      padding: 10px;
      border-radius: 10px;
    }
  </style>
</head>

<body>
  <h1>Micro-Invernadero</h1>

  <div>
    <h3>Temperatura</h3>
    <p id='temperatura' class="negro">20 °C</p>
    <h3>Humedad</h3>
    <p id='humedad' class="negro">50%</p>
    <h3>Humedad del Suelo</h3>
    <p id='humedadSuelo' class="negro">60%</p>
  </div>

  <h3>Temperatura requerida: <span id="vlimtemp">20</span></h3>
  <input type="range" id="limtemp" class="slider" min="10" max="30" value="20"
    oninput="updateSliderValue('limtemp', 'vlimtemp')">

  <h3>Humedad requerida: <span id="vlimhum"></span></h3>
  <input type="range" id="limhum" class="slider" min="0" max="100" value="50"
    oninput="updateSliderValue('limhum', 'vlimhum')">

  <h3>Humedad del suelo requerida: <span id="vlimhums"></span></h3>
  <input type="range" id="limhums" class="slider" min="0" max="100" value="50"
    oninput="updateSliderValue('limhums', 'vlimhums')">

  <h3>Bomba</h3>
  <input type='checkbox' id='bomba' onchange='toggleBomba()'>

  <h3>Humidificador</h3>
  <input type='checkbox' id='spray' onchange='toggleHumidificador()'>

  <h3>Ventilador_1: <span id='vvent1'>5</span></h3>
  <input type='range' id='vent1' class='slider' min='0' max='10' value='5'
    oninput='updateSliderValue("vent1", "vvent1")'>

  <h3>Ventilador_2: <span id='vvent2'>5</span></h3>
  <input type='range' id='vent2' class='slider' min='0' max='10' value='5'
    oninput='updateSliderValue("vent2", "vvent2")'>

  <h3>R: <span id='vR'>5</span></h3>
  <input type='range' id='R' class='slider' min='0' max='10' value='5' oninput='updateSliderValue("R", "vR")'>

  <h3>G: <span id='vG'>5</span></h3>
  <input type='range' id='G' class='slider' min='0' max='10' value='5' oninput='updateSliderValue("G", "vG")'>

  <h3>B: <span id='vB'>5</span></h3>
  <input type='range' id='B' class='slider' min='0' max='10' value='5' oninput='updateSliderValue("B", "vB")'>

  <p class="negro"></p>

  <script>
    function toggleBomba() {
      var estado = document.getElementById('bomba').checked;
      enviarDatos('bomba', estado);
    }
    function toggleHumidificador() {
      var estado = document.getElementById('spray').checked;
      enviarDatos('spray', estado);
    }
    function updateSliderValue(sliderId, valueId) {
      var valor = document.getElementById(sliderId).value;
      document.getElementById(valueId).innerText = valor; //innerText innerHTML
      enviarDatos(sliderId, valor);
    }
    function enviarDatos(dispositivo, estado) {
      var xhttp = new XMLHttpRequest();
      xhttp.open('GET', '/control/' + dispositivo + '/' + estado, true);
      xhttp.send();
    }
    function actualizarDatos() {
      var sliderNames = ["vent1", "vent2", "R", "G", "B", "limtemp", "limhum", "limhums"];
      var xhttp = new XMLHttpRequest();
      xhttp.open('GET', '/datos', true);
      xhttp.onload = function () {
        var datos = JSON.parse(this.responseText);
        document.getElementById('temperatura').innerHTML = datos.temperatura;
        document.getElementById('humedad').innerHTML = datos.humedad;
        document.getElementById('humedadSuelo').innerHTML = datos.humedadSuelo;
        document.getElementById('bomba').checked = datos.bombaEncendida;
        document.getElementById('spray').checked = datos.humidificadorEncendido;
        for (var i = 0; i < 8; i++) {
          document.getElementById(sliderNames[i]).value = datos.sliderValues[i];
          document.getElementById('v' + sliderNames[i]).innerHTML = datos.sliderValues[i];
        }
      };
      xhttp.send();
    }
    setInterval(function () { actualizarDatos(); }, 500);
  </script>
</body>

</html>)rawString";
