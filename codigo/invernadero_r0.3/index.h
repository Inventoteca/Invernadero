const char *html = R"(
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
      padding:10px;
      border-radius: 10px;
    }

    .slider {width: 70%;}
    
    .negro {margin-left: 25%; margin-right: 25%; background-color: black; padding:10px; border-radius:10px;}
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

  <h3>Bomba</h3>
  <label class='switch'>
    <input type='checkbox' id='toggleBomba' onchange='toggleBomba()'>
    <span class='slider round'></span>
  </label>

  <h3>Humidificador</h3>
  <label class='switch'>
    <input type='checkbox' id='toggleHumidificador' onchange='toggleHumidificador()'>
    <span class='slider round'></span>
  </label>

  <h3>Ventilador_1: <span id='valueVentilador_1'>5</span></h3>
  <input type='range' id='Ventilador_1' class='slider' min='0' max='10' value='5'
    oninput='updateSliderValue("Ventilador_1", "valueVentilador_1")'>

  <h3>Ventilador_2: <span id='valueVentilador_2'>5</span></h3>
  <input type='range' id='Ventilador_2' class='slider' min='0' max='10' value='5'
    oninput='updateSliderValue("Ventilador_2", "valueVentilador_2")'>

  <h3>R: <span id='valueR'>5</span></h3>
  <input type='range' id='R' class='slider' min='0' max='10' value='5' oninput='updateSliderValue("R", "valueR")'>

  <h3>G: <span id='valueG'>5</span></h3>
  <input type='range' id='G' class='slider' min='0' max='10' value='5' oninput='updateSliderValue("G", "valueG")'>

  <h3>B: <span id='valueB'>5</span></h3>
  <input type='range' id='B' class='slider' min='0' max='10' value='5' oninput='updateSliderValue("B", "valueB")'>

  <p class="negro"></p>

  <script>
    function toggleBomba() {
      var estado = document.getElementById('toggleBomba').checked;
      enviarDatos('bomba', estado);
    }
    function toggleHumidificador() {
      var estado = document.getElementById('toggleHumidificador').checked;
      enviarDatos('humidificador', estado);
    }
    function updateSliderValue(sliderId, valueId) {
      var valor = document.getElementById(sliderId).value;
      document.getElementById(valueId).innerText = valor; //innerText innerHTML
      enviarDatos(sliderId, valor);
    }
    function updateVariable(variable, valor) { //parece que esta función no se usa
      document.getElementById('input' + variable).value = valor;
      enviarDatos(variable, valor);
    }
    function enviarDatos(dispositivo, estado) {
      var xhttp = new XMLHttpRequest();
      xhttp.open('GET', '/control/' + dispositivo + '/' + estado, true);
      xhttp.send();
    }
    function actualizarDatos() {
      var sliderNames = ["Ventilador_1", "Ventilador_2", "R", "G", "B"];
      var xhttp = new XMLHttpRequest();
      xhttp.open('GET', '/datos', true);
      xhttp.onload = function () {
        var datos = JSON.parse(this.responseText);
        document.getElementById('temperatura').innerHTML = datos.temperatura;
        document.getElementById('humedad').innerHTML = datos.humedad;
        document.getElementById('humedadSuelo').innerHTML = datos.humedadSuelo;
        document.getElementById('toggleBomba').checked = datos.bombaEncendida;
        document.getElementById('toggleHumidificador').checked = datos.humidificadorEncendido;
        for (var i = 0; i < 5; i++) {
          document.getElementById(sliderNames[i]).value = datos.sliderValues[i];
          document.getElementById('value' + sliderNames[i]).innerHTML = datos.sliderValues[i];
        }
      };
      xhttp.send();
    }
    setInterval(function () { actualizarDatos(); }, 500);
  </script>
</body>

</html>)";
