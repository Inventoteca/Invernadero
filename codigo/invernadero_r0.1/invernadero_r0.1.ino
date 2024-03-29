/*
  INVERNADERO
  Maqueta de invernadero automatizado.
  El código inicial está basado en el ejemplo "Keypad_240x320" de la librería TFT_eSPI.
  Se utiliza la clase "button".
*/

// The SPIFFS (FLASH filing system) is used
// to hold touch screen calibration data
#include "FS.h"

// Iconos
#include "icons/th32.h" //thermometer
#include "icons/drop32.h" //water drop
#include "icons/grass32.h" //grass
#include "icons/fan32.h" //fan
#include "icons/lamp32.h" //lamp
#include "icons/sprink32.h" //sprinkler
#include "icons/pump32.h" //water pump

// Colores
#define FONDO 0x1084
#define PANEL 0x526d
#define GRIS70 0x4a69
#define ICON TFT_CYAN

#include <SPI.h>
#include <TFT_eSPI.h>      // Hardware-specific library
TFT_eSPI tft = TFT_eSPI(); // objeto para manejar la pantalla

// This is the file name used to store the calibration data
// You can change this to create new calibration files.
// The SPIFFS file name must start with "/".
#define CALIBRATION_FILE "/TouchCalData1"

// Set REPEAT_CAL to true instead of false to run calibration
// again, otherwise it will only be done once.
// Repeat calibration if you change the screen rotation.
#define REPEAT_CAL true
// en lugar de usar este define se usará un botón para iniciar la calibración

// Botones de la pantalla principal
TFT_eSPI_Button b[8]; //objetos botón
const int bx[8] = { 20,  20,  20, 180, 250, 180, 250, 180}; //x top-left corner
const int by[8] = { 20,  90, 160,  20,  20,  90,  90, 160}; //y top-left corner
const int bw[8] = {140, 140, 140,  60,  60,  60,  60, 130}; //wdith
const int bh[8] = { 60,  60,  60,  60,  60,  60,  60,  60}; //height
char* bl[8] = {"", "", "", "", "", "", "", "CONECTAR"}; //labels
const uint16_t bc[8] {0, 0, 0, PANEL, PANEL, PANEL, PANEL, PANEL}; //colors
const uint8_t* bi[8] = { //icons
  th32_bits, drop32_bits, grass32_bits,
  fan32_bits, lamp32_bits, sprink32_bits, pump32_bits, 0
};
const int bix[8] = { //icon x top-left corner
  40 - 16, 40 - 16, 40 - 16, 210 - 16, 280 - 16, 210 - 16, 280 - 16, 0
};
const int biy[8] = { //icon y top-left corner
  50 - 16, 120 - 16, 190 - 16, 50 - 16, 50 - 16, 120 - 16, 120 - 16, 0
};

//===================================================================
// Definir pines
// Los pines para la pantalla y el panel touch se definen en "Setup42_ILI9341_ESP32.h"
#define PIN_SUELOD 34 //sensor de humedad del suelo (lectura digital)
#define PIN_SUELOA 35 //sensor de humedad del suelo (lectura analogica)
#define PIN_VENT1 32 //ventilador 1 (usa PWM)
#define PIN_VENT2 33 //ventilador 2 (usa PMW)
#define PIN_R 26 //LED R (usa PMW)
#define PIN_G 25 //LED G (usa PMW)
#define PIN_B 27 //LED B (usa PMW)
#define PIN_BOMBA 14 //bomba de agua
#define PIN_SPRAY 13 //atomizador/spray

// sensor de temperatura y humedad
#include "DHT.h"
#define PIN_DHT 15
DHT dht(PIN_DHT, DHT22); //objeto para leer el sensor DHT22

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
  dht.begin();

  // Use serial port
  Serial.begin(115200);

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
  /*
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    Serial.print(h);
    Serial.println(t);
  */
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
      switch (i) {
        case 0: //temperatura
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
  //}//fin if pressed

  /*
    // Update the number display field
    tft.setTextDatum(TL_DATUM);        // Use top left corner as text coord datum
    tft.setFreeFont(&FreeSans18pt7b);  // Choose a nicefont that fits box
    tft.setTextColor(DISP_TCOLOR);     // Set the font colour

    // Draw the string, the value returned is the width in pixels
    int xwidth = tft.drawString(numberBuffer, DISP_X + 4, DISP_Y + 12);

    // Now cover up the rest of the line up by drawing a black rectangle.  No flicker this way
    // but it will not work with italic or oblique fonts due to character overlap.
    tft.fillRect(DISP_X + 4 + xwidth, DISP_Y + 1, DISP_W - xwidth - 5, DISP_H - 2, TFT_BLACK);
  */

  delay(50); // UI debouncing
} //fin loop()

// Mi función para dibujar botones
void drawButton() {
  // por ahora no se usa
}

//------------------------------------------------------------------------------------------
void drawUI() {
  tft.fillScreen(FONDO);

  // Fuentes
  tft.setTextFont(4);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  // En la fuente 1 el signo de grados es \xF7 (hexadecimal) o \367 (octal)
  // En las fuentes 2 y 4 el signo de grados es el acento grave
  // Tamaños que se ven bien
  // - Fuente 1 x3 o x4
  // - Fuente 2 x2 o x3
  // - Fuente 4 x1 porque el pixelado se ve feo

  // Grupo 1
  tft.fillRoundRect(10, 10, 160, 220, 4, PANEL); //tft.color565(19, 19, 19)
  for (uint8_t i = 0; i < 3; i++) {
    b[i].drawButton(); //normal
  }
  //tft.fillRoundRect(20, 20, 140, 60, 20, TFT_BLACK);
  //tft.fillRoundRect(20, 90, 140, 60, 20, TFT_BLACK);
  //tft.fillRoundRect(20, 160, 140, 60, 20, TFT_BLACK);
  //
  tft.drawXBitmap(40 - 16, 50 - 16, th32_bits, 32, 32, TFT_CYAN);
  tft.drawXBitmap(40 - 16, 120 - 16, drop32_bits, 32, 32, TFT_CYAN);
  tft.drawXBitmap(40 - 16, 190 - 16, grass32_bits, 32, 32, TFT_CYAN);
  //
  tft.drawString("26 `C", 70, 30 + 8);
  tft.drawString("50%", 70, 100 + 8);
  tft.drawString("10%", 70, 170 + 8);

  // Grupo 2
  tft.setTextFont(2);
  tft.setTextSize(1);
  //
  for (uint8_t i = 3; i < 8; i++) {
    b[i].drawButton(); //normal
  }
  //tft.fillRoundRect(180, 20, 60, 60, 4, PANEL);
  //tft.fillRoundRect(250, 20, 60, 60, 4, PANEL);
  //tft.fillRoundRect(180, 90, 60, 60, 4, PANEL);
  //tft.fillRoundRect(250, 90, 60, 60, 4, PANEL);
  //tft.fillRoundRect(180, 160, 130, 60, 4, PANEL);
  //
  tft.drawXBitmap(210 - 16, 50 - 16, fan32_bits, 32, 32, TFT_CYAN);
  tft.drawXBitmap(280 - 16, 50 - 16, lamp32_bits, 32, 32, TFT_CYAN);
  tft.drawXBitmap(210 - 16, 120 - 16, sprink32_bits, 32, 32, TFT_CYAN);
  tft.drawXBitmap(280 - 16, 120 - 16, pump32_bits, 32, 32, TFT_CYAN);
}

//------------------------------------------------------------------------------------------
void touch_calibrate()
{
  uint16_t calData[5]; //array de 5 enteros
  uint8_t calDataOK = 0; //flag

  // check file system exists
  // si no existe se formatea
  if (!SPIFFS.begin()) {
    Serial.println("Formating file system");
    SPIFFS.format();
    SPIFFS.begin();
  }

  // una vez que el sistema de archivos está listo
  // check if calibration file exists and size is correct
  if (SPIFFS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CAL) //si se debe repetir la calibración
    {
      // Delete if we want to re-calibrate
      SPIFFS.remove(CALIBRATION_FILE);
    }
    else //si no se debe repetir
    {
      // abrir archivo en modo de lectura
      File f = SPIFFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {//si archivo ok y repetir es falso
    // calibration data valid
    tft.setTouch(calData); //usar datos de calibración guardados
  } else {//si archivo no ok o repetir cal
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK); //texto blanco, fondo negro

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    /*if (REPEAT_CAL) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
      }*/

    // esta es la función principal para calibrar
    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    // abrir archivo en modo escritura
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}
