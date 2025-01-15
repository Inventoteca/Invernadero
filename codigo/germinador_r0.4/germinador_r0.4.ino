/*
  GERMINADOR
  Maqueta de invernadero automatizado.
  El código inicial está basado en
  - ejemplo "Keypad_240x320" de la librería TFT_eSPI
  - ejemplo dht22

  En este archivo se encuentra la lógica principal del programa

  Versiones
  0.3 agrega varias pantallas con controles deslizantes
  0.4 cambiar pines y agregar medidor de PH PH
*/

//===================================================================
// Definir pines
#define PIN_SUELOD 35  //sensor de humedad del suelo (lectura digital)
#define PIN_SUELOA 34  //sensor de humedad del suelo (lectura analogica)
#define PIN_PHD 39     //sensor de PH (lectura digital)
#define PIN_PHA 36     //sensor de PH (lectura analogica)
#define PIN_VENT1 32   //ventilador 1 (PWM)
#define PIN_VENT2 33   //ventilador 2 (PMW)
#define PIN_R 12       //LED R (PMW)
#define PIN_G 13       //LED G (PMW)
#define PIN_B 14       //LED B (PMW)
#define PIN_BOMBA 26   //relevador para bomba de agua
#define PIN_SPRAY 25   //relevador para bomba de atomizador/spray
#define PIN_BEEP 17    //buzzer piezoeléctrico

// Los pines para la pantalla y el panel touch se definen en "Setup21_ILI9488.h"

// nuevos pines agregados
//#define PIN_BSPRAY 13  //bomba para el spray
//#define PIN_SSPRAY 39  //36//sensor de agua para el spray
// cada vez que se presiona un botón suena el beep

// sensor de temperatura y humedad
#define PIN_DHT 27
#include "DHT_Async.h"
DHT_Async dht(PIN_DHT, DHT_TYPE_22);
//#include <dhtESP32-rmt.h>
float humedad, temperatura;  //variables para guardar lecturas del sensor
// las lecturas se realizan cada 2 segundos
#define DHT_PERIOD 3000  //4000//periodo de lectura en milisegundos
unsigned long dht_now;   //marca de tiempo en la que se hizo una lectura

// Variables para guardar el estado de los sensores y actuadores
bool suelod;  //guarda un estado alto o bajo
//es alto cuando la resistencia es alta?
int sueloa;  //guarda la lectura analógia (12 bits --> 0 a 4095)
//es el valor de la resistencia de la tierra
//cuando la resistencia es infinita la lectura es 4095
//cuando la resistencia es cero la lectura es 0
//el nivel de humedad como porcentaje se puede calcular así:
//int humedad_suelo = ((4095 - sueloa) * 100) / 4095;
int humedad_suelo = (4095 - sueloa) / 41;  // forma más simple
//se debe cambiar el tipo de variable?
//cuando la humedad baja de un nivel determinado, se enciende la bomba
int lim_humedad_suelo = 50;  //limite de humedad del suelo
//si la humedad baja de ese nivel, se activa la bomba, si se pasa de ese nivel, apaga la bomba
//la lectura de este sensor se realiza con un poco de retardo
#define SH_PERIOD 1000  //sensor humedad periodo
unsigned long sh_now;   //sensor humedad marca temporal

//el estado de los ventiladores se guarda en las siguientes variables
//se usará pwm con resolución de 8 bits (0 a 255)
//pero se usan otras variables que guardan un valor de 0 a 10
//uint8_t vent1 = 0;
//uint8_t vent2 = 0;
float vent1 = 0;
float vent2 = 0;
uint8_t pwm_vent1 = 0;
uint8_t pwm_vent2 = 0;
//el ciclo de trabajo asigna con analogWrite,
//sin embargo la frecuencia del pwm se escucha como zumbido
//para cambiar a una frecuencia no audible se puede usar la librería ledc
//(https://lastminuteengineers.com/esp32-pwm-tutorial/)
//(https://randomnerdtutorials.com/esp32-pwm-arduino-ide/)
//los ventiladores se activan cuando la temperatura ambiente sobrepasa un valor
//uint8_t lim_temperatura = 20; //en grados celcius

int lim_temperatura = 20;  //en grados celcius
//la lectura del sensor es float, se debe usar el mismo tipo de variable?

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
//
float val_r = 0;
float val_g = 0;
float val_b = 0;

//la bomba de agua se activa con un relevador. Tiene lógica invertida:
//cuando el pin está en HIGH, la bomba se apaga
//cuando el pin está en LOW, la bomba se enciende
bool bomba = false;  //estado de la boma, el pin se escribirá invertido

//el nebulizador/atomizador/spray también se activa con un relevador
bool spray = false;  //estado del spray
//se necesitaba un pulso, pero ya no
//pero necesita un pulso (flanco descendente) para activarse y otro pulso para desactivarse
//qué duración debe tener ese pulso?
//#define T_PULSO_SPRAY 30  //milisegundos
//el relevador se activa un momento y luego se desactiva

#define T_PULSO_BEEP 30  //milisegundos, duracion minima del beep

int lim_humedad = 60;  //si baja de este valor, se activa el nebulizador
//float lim_humedad = 60; //usar float?

//puede existir un control para seleccionar funcionamiento manual o automático
//por ahora se cambiará a manual si se cambian los valores de los actuadores
//y regresará a automático si se cambian los valores de los límites
bool vent_auto = true;   //ventilador automático (se activa con la temperatura)
bool spray_auto = true;  //nebulizador/spray automático (se activa con la humedad)
bool bomba_auto = true;  //bamba automática (se activa con la humedad del suelo)
/*
  ¿Qué pasa si el usuario utiliza el control manual?
  Lo más fácil es sobre-escribir el valor al instante
  y cuando vuelve a leer el sensor retoma el valor por default.
  La segunda opción es que cuando el usuario usa el control manual
  se activa un temporizador que mantiene los valores por un minuto
  y luego regresa a su valor por default
  También se puede reactivar el control automático si se reasigna el valor límite
*/

// Código para el display
#include "display.h"

// Código para el access point y página web
#include "server.h"

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
  pinMode(PIN_SPRAY, OUTPUT);  //controlar spray
  //pinMode(PIN_BSPRAY, OUTPUT);  //bomba spray
  //pinMode(PIN_SSPRAY, INPUT);   //sensor spray
  pinMode(PIN_BEEP, OUTPUT);  //buzzer piezoeléctrico
  //
  analogRead(PIN_SUELOA);  //descartar primera lectura
  analogRead(PIN_PHA);     //descartar primera lectura
  analogWrite(PIN_VENT1, 0);
  analogWrite(PIN_VENT2, 0);
  analogWrite(PIN_R, 0);  //si el pin es DAC no saca PWM?
  analogWrite(PIN_G, 0);
  analogWrite(PIN_B, 0);
  digitalWrite(PIN_BOMBA, HIGH);  //relevador apagado en HIGH
  //digitalWrite(PIN_BSPRAY, HIGH);  //relevador apagado en HIGH
  digitalWrite(PIN_SPRAY, HIGH);  //relevador apagado en HIGH
  digitalWrite(PIN_BEEP, HIGH);
  digitalWrite(PIN_BEEP, LOW);
  //
  //dht.begin();
  //uint8_t dht_error = read_dht(temperatura, humedad, PIN_DHT, DHT22); //descartar primera lectura
  dht.measure(&temperatura, &humedad);  //descartar primera lectura
  dht.measure(&temperatura, &humedad);
  dht_now = millis();  //guardar marca temporal
  digitalWrite(PIN_BEEP, LOW);

  // Use serial port
  Serial.begin(115200);
  Serial.print("\nGERMINADOR\n");
  digitalWrite(PIN_BEEP, LOW);

  // Initialise the TFT screen
  tft.init();
  tft.setRotation(1);  //3 // Set the rotation before we calibrate
  touch_calibrate();   // Calibrate and retrieve the scaling factors
  //tft.fillScreen(TFT_BLACK); // Clear the screen
  digitalWrite(PIN_BEEP, LOW);

  // Mensaje de bienvenida
  tft.fillScreen(FONDO);
  tft.setTextFont(4);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(TC_DATUM);  //top-centre
  tft.drawString("GERMINADOR PARA ZONAS URBANAS", 160, 80);
  tft.drawString("CONTROLADO MEDIANTE TECNOLOGIA IoT", 160, 110);

  // Iniciar server
  serverSetup();
  delay(2000);  //esperar para que se vea el mensaje de bienvenida

  //Inicializar controles de todas las pantallas/páginas
  setupUI0();  //principal
  setupUI1();  //temperatura
  setupUI2();  //humedad
  setupUI3();  //humedad suelo
  setupUI4();  //ventiladores
  setupUI5();  //color
  setupUI6();  //conectar
  drawUI0();   //Dibujar interfaz principal
  //drawUI1();
  //while (true) {}
}

//------------------------------------------------------------------------------------------
void loop(void) {
  // Leer sensor dht de manera periódica =================================
  if ((unsigned long)(millis() - dht_now) > DHT_PERIOD) {
    dht_now = millis();
    dht.measure(&temperatura, &humedad);

    //imprimir en consola
    /*
    Serial.print("T: ");
    Serial.print(temperatura, 1);
    Serial.print("  H: ");
    Serial.print(humedad, 1);
    Serial.println("%");
    */

    //actualizar pantalla principal
    if (pantalla == 0) {
      tft.setTextFont(4);
      tft.setTextSize(1);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      String t = String(temperatura, 0) + " `C  ";
      String h = String(humedad, 0) + "%   ";
      tft.drawString(t, btx0[0], bty0[0]);
      tft.drawString(h, btx0[1], bty0[1]);
    }

    //si está activado el control automático del ventilador
    if (vent_auto) {
      //si la temperatura sobrepasa el límite
      if (temperatura > lim_temperatura) {
        vent1 = 10;  //encender a la máxima velocidad
        vent2 = 10;
        analogWrite(PIN_VENT1, vent1 * 25.5);
        analogWrite(PIN_VENT2, vent2 * 25.5);
        Serial.println("Vent ON (auto)");
      }
      //si la temperatura está debajo del límite
      else if (temperatura < lim_temperatura) {
        vent1 = 0;  //apagar ventiladores
        vent2 = 0;
        analogWrite(PIN_VENT1, vent1 * 25.5);
        analogWrite(PIN_VENT2, vent2 * 25.5);
        Serial.println("Vent OFF (auto)");
      }
    }  //fin if vent_auto

    //si está activado el control automático del nebulizador/spray
    if (spray_auto) {
      //si la humedad es más baja que el límite
      if (humedad < lim_humedad) {
        //encender el spray solo si está apagado
        if (spray == false) {
          digitalWrite(PIN_SPRAY, LOW);  //recuerda, el relevador se activa en bajo
          //delay(T_PULSO_SPRAY);           //tiempo que tarda el pulso
          //digitalWrite(PIN_SPRAY, LOW);   //desactivar relevador
          spray = true;                   //guardar estado actual
          Serial.println("Spray ON (auto)");
          if (pantalla == 0) tft.drawXBitmap(bix0[5], biy0[5], bi0[5], 32, 32, TFT_CYAN);
        }
      }
      // si la humedad está arriba del límite
      else if (humedad > lim_humedad) {
        //apagar el nebulizador spray, solo si está encendido
        if (spray == true) {
          digitalWrite(PIN_SPRAY, HIGH);  //recuerda, el relevador se activa en bajo
          //delay(T_PULSO_SPRAY);           //tiempo que tarda el pulso
          //digitalWrite(PIN_SPRAY, LOW);   //desactivar relevador
          spray = false;                  //guardar estado actual
          Serial.println("Spray OFF (auto)");
          if (pantalla == 0) tft.drawXBitmap(bix0[5], biy0[5], bi0[5], 32, 32, 0);
        }
      }
    }  //fin if spray_auto
  }    //fin DHT_PERIOD

  // El sensor de humedad de suelo se lee más seguido ===============================
  // XXXXXXXXXXXXX También se lee el sensor de agua dentro para el spray XXXXXXXXXX
  if ((unsigned long)(millis() - sh_now) > SH_PERIOD) {
    sh_now = millis();
    suelod = digitalRead(PIN_SUELOD);      //lectura digital
    sueloa = analogRead(PIN_SUELOA);       //lectura analógica
    humedad_suelo = (4095 - sueloa) / 41;  //calcular porcentaje de humedad
    //
    //sensor de agua para el spray
    // 0 significa nivel alto, 1 significa nivel bajo
    // activar o desactivar la bomba
    //digitalWrite(PIN_BSPRAY, !digitalRead(PIN_SSPRAY));
    /*
    Serial.print("HS: ");
    Serial.print(humedad_suelo);
    Serial.println("%");
    */

    // Actualizar pantalla 0 (principal)
    if (pantalla == 0) {
      tft.setTextFont(4);
      tft.setTextSize(1);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      String hs = String(humedad_suelo) + "%   ";
      tft.drawString(hs, btx0[2], bty0[2]);
    }

    //si el control automático de la bomba está activado
    if (bomba_auto) {
      //si la humedad del suelo está debajo del límite
      if (humedad_suelo < lim_humedad_suelo) {
        digitalWrite(PIN_BOMBA, LOW);  //encendido en nivel bajo
        bomba = true;                  //guardar estado actual
        Serial.println("Bomba ON (auto)");
        //dibujar icono
        if (pantalla == 0) tft.drawXBitmap(bix0[6], biy0[6], bi0[6], 32, 32, TFT_CYAN);
      }
      //en cambio, si la humedad del suelo está arriba del límite
      if (humedad_suelo > lim_humedad_suelo) {
        digitalWrite(PIN_BOMBA, HIGH);  //apagado en nivel alto
        bomba = false;                  //guardar estado actual
        Serial.println("Bomba OFF (auto)");
        //dibujar icono
        if (pantalla == 0) tft.drawXBitmap(bix0[6], biy0[6], bi0[6], 32, 32, 0);
      }
    }
  }

  //Detectar clics en la pantalla ============================================
  if ((unsigned long)(millis() - touch_now) > TOUCH_PERIOD) {
    touch_now = millis();

    //realizar acciones de la pantalla conrrespondiente
    switch (pantalla) {
      case 0:
        loopUI0();
        break;
      case 1:
        loopUI1();
        break;
      case 2:
        loopUI2();
        break;
      case 3:
        loopUI3();
        break;
      case 4:
        loopUI4();
        break;
      case 5:
        loopUI5();
        break;
      case 6:
        loopUI6();
        break;
    }
  }  //fin TOUCH_PERIOD

  // Actualizar server ===================================================
  serverLoop();

}  //fin loop()
//Fin del programa
