/*
  MICRO INVERNADERO EXPERIMENTAL PORTATIL
  Maqueta de invernadero automatizado.
  El código inicial está basado en
  - ejemplo "Keypad_240x320" de la librería TFT_eSPI
  - ejemplo dht22

  En este archivo se encuentra la lógica principal del programa
*/

// Código para el display
#include "display.h"

//===================================================================
// Definir pines
#define PIN_SUELOD 34 //sensor de humedad del suelo (lectura digital)
#define PIN_SUELOA 35 //sensor de humedad del suelo (lectura analogica)
#define PIN_VENT1 32 //ventilador 1 (usa PWM)
#define PIN_VENT2 33 //ventilador 2 (usa PMW)
#define PIN_R 26 //LED R (usa PMW)
#define PIN_G 25 //LED G (usa PMW)
#define PIN_B 27 //LED B (usa PMW)
#define PIN_BOMBA 14 //bomba de agua
#define PIN_SPRAY 13 //atomizador/spray
// Los pines para la pantalla y el panel touch se definen en "Setup42_ILI9341_ESP32.h"

// sensor de temperatura y humedad
#define PIN_DHT 15
//
//#include "DHT.h"
//DHT dht(PIN_DHT, DHT22); //objeto para leer el sensor DHT22
//
//otra librería
#include "DHT_Async.h"
DHT_Async dht(PIN_DHT, DHT_TYPE_22);
//
//variables para guardar lecturas del sensor
float humedad, temperatura;
//las lecturas se realizan después cada varios segundos porque es un sensor lento
#define DHT_PERIOD 4000 //periodo de lectura en milisegundos
unsigned long dht_now; //marca de tiempo en la que se hizo una lectura

// Variables para guardar el estado de los sensores y actuadores
bool suelod; //guarda un estado alto o bajo
//es alto cuando la resistencia es alta?
int sueloa; //guarda la lectura analógia (12 bits --> 0 a 4095)
//es el valor de la resistencia de la tierra
//cuando la resistencia es infinita la lectura es 4095
//cuando la resistencia es cero la lectura es 0
//el nivel de humedad como porcentaje se puede calcular así:
//int humedad_suelo = ((4095 - sueloa) * 100) / 4095;
int humedad_suelo = (4095 - sueloa) / 41; // forma más simple
//se debe cambiar el tipo de variable?
//cuando la humedad baja de un nivel determinado, se enciende la bomba
int lim_humedad_suelo = 50; //limite de humedad del suelo
//si la humedad baja de ese nivel, se activa la bomba, si se pasa de ese nivel, apaga la bomba
//la lectura de este sensor se realiza con un poco de retardo
#define SH_PERIOD 1000
unsigned long sh_now; //sensor humedad marca temporal

//el estado de los ventiladores se guarda en las siguientes variables
//se usará pwm con resolución de 8 bits (0 a 255)
uint8_t vent1 = 0;
uint8_t vent2 = 0;
//el ciclo de trabajo asigna con analogWrite,
//sin embargo la frecuencia del pwm se escucha como zumbico
//para cambiar a una frecuencia no audible se puede usar la librería ledc
//(https://lastminuteengineers.com/esp32-pwm-tutorial/)
//(https://randomnerdtutorials.com/esp32-pwm-arduino-ide/)
//los ventiladores se activan cuando la temperatura ambiente sobrepasa un valor
//uint8_t lim_temperatura = 20; //en grados celcius
float lim_temperatura = 20; //en grados celcius
//para asignar este limite se puede usar una barra deslizadora de 10 a 50 grados
//cuál es el rango ideal?
//los ventiladores pueden encender con una velocidad proporcional a la temperatura
//o pueden encender a una velodiad predeterminada (se encenderán a la máxima velocidad, es más fácil)
//con el control manual se pueden activar los ventiladores de manera independiente,
//pero con el control automático los dos ventiladores se encienden o apagan al mismo tiempo

//los leds usan pwm como los ventiladores (analogWrite)
//es necesario cambiar la resolución o frecuencia de pwm para los leds?
uint8_t pwm_r = 0;
uint8_t pwm_g = 0;
uint8_t pwm_b = 0;
//qué condición hace que los leds enciendan?
//puede ser un temporizador que active la luz por periodos
//por ahora solo controlarán de forma manual
//estos son los valores por defatul al encender cuando el control sea atuomático
uint8_t def_r = 255;
uint8_t def_g = 255;
uint8_t def_b = 255;

//la bomba de agua se activa con un relevador. Tiene lógica invertida:
//cuando el pin está en HIGH, la bomba se apaga
//cuando el pin está en LOW, la bomba se enciende
bool bomba = false; //estado de la boma, el pin se escribirá invertido

//el nebulizador/atomizador/spray también se activa con un relevador
bool spray = false; //estado del spray
//pero necesita un pulso (flanco descendente) para activarse y otro pulso para desactivarse
//qué duración debe tener ese pulso?
#define T_PULSO_SPRAY 100 //milisegundos
//el relevador se activa un momento y luego se desactiva
float lim_humedad = 60; //si baja de este valor, se activa el nebulizador

//puede existir un control para seleccionar funcionamiento manual o automático
//por ahora se cambiará a manual si se cambian los valores de los actuadores
//y regresará a automático si se cambian los valores de los límites
bool vent_auto = true; //ventilador automático (se activa con la temperatura)
bool spray_auto = true; //nebulizador/spray automático (se activa con la humedad)
bool bomba_auto = true; //bamba automática (se activa con la humedad del suelo)
/*
  ¿Qué pasa si el usuario utiliza el control manual?
  Lo más fácil es sobre-escribir el valor al instante
  y cuando vuelve a leer el sensor retoma el valor por default.
  La segunda opción es que cuando el usuario usa el control manual
  se activa un temporizador que mantiene los valores por un minuto
  y luego regresa a su valor por default
  También se puede reactivar el control automático si se reasigna el valor límite
*/

//------------------------------------------------------------------------------------------
void setup() {
  // configurar entradas y salidas
  pinMode(PIN_SUELOD, INPUT);
  pinMode(PIN_SUELOA, INPUT);
  pinMode(PIN_VENT1, OUTPUT);
  pinMode(PIN_VENT2, OUTPUT);
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_BOMBA, OUTPUT);
  pinMode(PIN_SPRAY, OUTPUT);
  //
  analogRead(PIN_SUELOA); //descartar primera lectura
  analogWrite(PIN_VENT1, 0);
  analogWrite(PIN_VENT2, 0);
  analogWrite(PIN_R, 0); //si el pin es DAC no saca PWM?
  analogWrite(PIN_G, 0);
  analogWrite(PIN_B, 0);
  digitalWrite(PIN_BOMBA, HIGH); //relevador apagado en HIGH
  digitalWrite(PIN_SPRAY, HIGH); //relevador apagado en HIGH
  //
  //dht.begin();
  dht.measure(&temperatura, &humedad); //descartar primera lectura
  dht.measure(&temperatura, &humedad);
  dht_now = millis(); //guardar marca temporal

  // Use serial port
  Serial.begin(115200);
  Serial.print("\nMICRO INVERNADERO\n");

  // Initialise the TFT screen
  tft.init();
  tft.setRotation(3);//3 // Set the rotation before we calibrate
  touch_calibrate(); // Calibrate and retrieve the scaling factors
  //tft.fillScreen(TFT_BLACK); // Clear the screen

  // Inicializar botones
  for (uint8_t i = 0; i < 8; i++) {
    b[i].initButtonUL(&tft, bx[i], by[i], bw[i], bh[i], bc[i], bc[i], TFT_WHITE, bl[i], 1);
  }
  drawUI(); //Dibujar interfaz
}

//------------------------------------------------------------------------------------------
void loop(void) {
  // Leer sensor dht de manera periódica
  if ((unsigned long)(millis() - dht_now) > DHT_PERIOD) {
    dht_now = millis();
    dht.measure(&temperatura, &humedad);

    //imprimir en consola
    Serial.print("T: ");
    Serial.print(temperatura, 1);
    Serial.print("  H: ");
    Serial.print(humedad, 1);
    Serial.println("%");

    //actualizar pantalla
    tft.setTextFont(4);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    String t = String(temperatura, 0) + " `C ";
    String h = String(humedad, 0) + "% ";
    tft.drawString(t, 70, 40); //30 + 8
    tft.drawString(h, 70, 110); //100 + 8

    //si está activado el control automático del ventilador
    if (vent_auto) {
      //si la temperatura sobrepasa el límite
      if (temperatura > lim_temperatura) {
        vent1 = 255; //encender a la máxima velocidad
        vent2 = 255;
        analogWrite(PIN_VENT1, vent1);
        analogWrite(PIN_VENT2, vent2);
        Serial.println("Vent ON (auto)");
      }
      //si la temperatura está debajo del límite
      else if (temperatura < lim_temperatura) {
        vent1 = 0; //apagar ventiladores
        vent2 = 0;
        analogWrite(PIN_VENT1, vent1);
        analogWrite(PIN_VENT2, vent2);
        Serial.println("Vent OFF (auto)");
      }
    }//fin if vent_auto

    //si está activado el control automático del nebulizador/spray
    if (spray_auto) {
      //si la humedad es más baja que el límite
      if (humedad < lim_humedad) {
        //encender el spray solo si está apagado
        if (spray == false) {
          digitalWrite(PIN_SPRAY, LOW); //recuerda, el relevador se activa en bajo
          delay(T_PULSO_SPRAY); //tiempo que tarda el pulso
          digitalWrite(PIN_SPRAY, HIGH); //desactivar relevador
          spray = true; //guardar estado actual
          Serial.println("Spray ON (auto)");
        }
      }
      // si la humedad está arriba del límite
      else if (humedad > lim_humedad) {
        //apagar el nebulizador spray, solo si está encendido
        if (spray == true) {
          digitalWrite(PIN_SPRAY, LOW); //recuerda, el relevador se activa en bajo
          delay(T_PULSO_SPRAY); //tiempo que tarda el pulso
          digitalWrite(PIN_SPRAY, HIGH); //desactivar relevador
          spray = false; //guardar estado actual
          Serial.println("Spray OFF (auto)");
        }
      }
    } //fin if spray_auto
  }//fin DHT_PERIOD

  // El sensor de humedad de suelo se lee más seguido
  if ((unsigned long)(millis() - sh_now) > SH_PERIOD) {
    sh_now = millis();
    suelod = digitalRead(PIN_SUELOD); //lectura digital
    sueloa = analogRead(PIN_SUELOA); //lectura analógica
    humedad_suelo = (4095 - sueloa) / 41; //calcular porcentaje de humedad
    //
    Serial.print("HS: ");
    Serial.print(humedad_suelo);
    Serial.println("%");
    //
    tft.setTextFont(4);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    String hs = String(humedad_suelo) + "% ";
    tft.drawString(hs, 70, 180); //170 + 8

    //si el control automático de la bomba está activado
    if (bomba_auto) {
      //si la humedad del suelo está debajo del límite
      if (humedad_suelo < lim_humedad_suelo) {
        digitalWrite(PIN_BOMBA, LOW); //encendido en nivel bajo
        bomba = true; //guardar estado actual
        Serial.println("Bomba ON (auto)");
      }
      //en cambio, si la humedad del suelo está arriba del límite
      if (humedad_suelo > lim_humedad_suelo) {
        digitalWrite(PIN_BOMBA, HIGH); //apagado en nivel alto
        bomba = false; //guardar estado actual
        Serial.println("Bomba OFF (auto)");
      }
    }
  }

  //Detectar clics en la pantalla
  if ((unsigned long)(millis() - touch_now) > TOUCH_PERIOD) {
    // Pressed will be set true is there is a valid touch on the screen
    uint16_t tx = 0, ty = 0; // To store the touch coordinates
    bool pressed = tft.getTouch(&tx, &ty, 0); //using threshold

    //if (pressed) { //si la pantalla es tocada
    for (uint8_t i = 0; i < 8; i++) { //revisar los botones
      if (pressed and b[i].contains(tx, ty)) { //si el botón es tocado
        b[i].press(true); //marcar como presionado
      } else {
        b[i].press(false); //en caso contrario, marcar como no presionado
      }

      // Si el botón se acaba de presionar, dibujarlo presionado
      if (b[i].justPressed()) {
        b[i].drawButton(true); //inverted/pressed
      }

      // Si el botón se acaba de soltar, realizar una acción
      if (b[i].justReleased()) {
        //acción
        //drawUI();
        b[i].drawButton(false); //dibujar normal
        if (i != 7) {
          tft.drawXBitmap(bix[i], biy[i], bi[i], 32, 32, ICON);
        }

        /*
          Cambiar a otra pantalla
          primer se dibuja otra UI y se crean nuevos botones
          (ya podrían estar creados desde el principio)
          Ahora la detección de clics se hace con otro conjunto de botones
          Cómo ordenar los botones?

          En los sliders se dibuja el boton como fondo
          encima se dibuja un rectángulo alargado y un círculo
        */
        switch (i) {
          case 0: //temperatura
            Serial.println("clic temperatura");
            break;
          case 1: //humedad del aire
            break;
          case 2: //humedad del suelo
            break;
          case 3: //ventilador
            break;
          case 4: //LEDs
            break;
          case 5: //spray/atomizador
            break;
          case 6: //bomba de agua
            break;
          case 7: //conectar
            break;
        }
      }
    }//fin for
  }//fin if pressed
}//fin loop()
