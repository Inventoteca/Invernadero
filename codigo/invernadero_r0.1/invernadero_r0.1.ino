/*
  INVERNADERO
  Maqueta de invernadero automatizado.
  El código inicial está basado en el ejemplo "Keypad_240x320" de la librería TFT_eSPI.
  Se utiliza la clase "button".
*/

// The SPIFFS (FLASH filing system) is used
// to hold touch screen calibration data
#include "FS.h"

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
#define REPEAT_CAL false
// en lugar de usar este define se usará un botón para iniciar la calibración

// Keypad start position, key sizes and spacing
#define KEY_X 40 // Centre of key
#define KEY_Y 96
#define KEY_W 64 // Width and height
#define KEY_H 35
#define KEY_SPACING_X 10 // X and Y gap
#define KEY_SPACING_Y 10
#define KEY_TEXTSIZE 1   // Font size multiplier

// Using two fonts since numbers are nice when bold
#define LABEL1_FONT &FreeSansOblique12pt7b // Key label font 1
#define LABEL2_FONT &FreeSansBold12pt7b    // Key label font 2
// tal vez sea neceario vel el ejemplo de fonts para elegir otras

// Numeric display box size and location
#define DISP_X 1
#define DISP_Y 10
#define DISP_W 238
#define DISP_H 50
#define DISP_TSIZE 3
#define DISP_TCOLOR TFT_CYAN

// Number length, buffer for storing it and character index
#define NUM_LEN 12
char numberBuffer[NUM_LEN + 1] = "";
uint8_t numberIndex = 0;

// We have a status line for messages
#define STATUS_X 120 // Centred on this
#define STATUS_Y 65

// Create 15 keys for the keypad
char keyLabel[15][5] = {"New", "Del", "Send", "1", "2", "3", "4", "5", "6", "7", "8", "9", ".", "0", "#" };
uint16_t keyColor[15] = {TFT_RED, TFT_DARKGREY, TFT_DARKGREEN,
                         TFT_BLUE, TFT_BLUE, TFT_BLUE,
                         TFT_BLUE, TFT_BLUE, TFT_BLUE,
                         TFT_BLUE, TFT_BLUE, TFT_BLUE,
                         TFT_BLUE, TFT_BLUE, TFT_BLUE
                        };

// Invoke the TFT_eSPI button class and create all the button objects
TFT_eSPI_Button key[15];

//===================================================================
// Definir pines
// Los pines para la pantalla, incluyendo los del panel touch,
// se definen en "Setup42_ILI9341_ESP32.h" de la librería TFT_eSPI

// sensor de humedad de suelo
#define PIN_SUELO 36 //lectura analogica

// ventiladores - 2 salidas PWM
#define PIN_VENT1 32
#define PIN_VENT2 33

// LEDs RGB - 3 salidas PWM
#define PIN_R 25
#define PIN_G 26
#define PIN_B 27

// bomba de agua
#define PIN_BOMBA 12

// atomizador/spray
#define PIN_SPRAY 13

// sensor de temperatura y humedad
#include "DHT.h"
#define PIN_DHT 14
DHT dht(PIN_DHT, DHT22); //objeto para leer el sensor DHT22

//------------------------------------------------------------------------------------------

void setup() {
  // configurar entradas y salidas
  pinMode(PIN_SUELO, INPUT);
  pinMode(PIN_VENT1, OUTPUT);
  pinMode(PIN_VENT2, OUTPUT);
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_BOMBA, OUTPUT);
  pinMode(PIN_SPRAY, OUTPUT);
  //
  analogRead(PIN_SUELO); //descartar primera lectura
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
  tft.setRotation(3); // Set the rotation before we calibrate
  touch_calibrate(); // Calibrate and retrieve the scaling factors
  tft.fillScreen(TFT_BLACK); // Clear the screen
  //tft.fillRect(0, 0, 240, 320, TFT_DARKGREY); // Draw keypad background

  // Draw number display area and frame
  //tft.fillRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_BLACK);
  //tft.drawRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_WHITE);

  // Draw keypad
  //drawKeypad();

  // Dibujar interfaz
  drawUI();
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

  uint16_t t_x = 0, t_y = 0; // To store the touch coordinates

  // Pressed will be set true is there is a valid touch on the screen
  bool pressed = tft.getTouch(&t_x, &t_y);

  // / Check if any key coordinate boxes contain the touch coordinates
  for (uint8_t b = 0; b < 15; b++) {
    if (pressed && key[b].contains(t_x, t_y)) {
      key[b].press(true);  // tell the button it is pressed
    } else {
      key[b].press(false);  // tell the button it is NOT pressed
    }
  }

  // Check if any key has changed state
  for (uint8_t b = 0; b < 15; b++) {

    if (b < 3) tft.setFreeFont(LABEL1_FONT);
    else tft.setFreeFont(LABEL2_FONT);

    if (key[b].justReleased()) key[b].drawButton();     // draw normal

    if (key[b].justPressed()) {
      key[b].drawButton(true);  // draw invert

      // if a numberpad button, append the relevant # to the numberBuffer
      if (b >= 3) {
        if (numberIndex < NUM_LEN) {
          numberBuffer[numberIndex] = keyLabel[b][0];
          numberIndex++;
          numberBuffer[numberIndex] = 0; // zero terminate
        }
        status(""); // Clear the old status
      }

      // Del button, so delete last char
      if (b == 1) {
        numberBuffer[numberIndex] = 0;
        if (numberIndex > 0) {
          numberIndex--;
          numberBuffer[numberIndex] = 0;//' ';
        }
        status(""); // Clear the old status
      }

      if (b == 2) {
        status("Sent value to serial port");
        Serial.println(numberBuffer);
      }
      // we dont really check that the text field makes sense
      // just try to call
      if (b == 0) {
        status("Value cleared");
        numberIndex = 0; // Reset index to 0
        numberBuffer[numberIndex] = 0; // Place null in buffer
      }

      // Update the number display field
      tft.setTextDatum(TL_DATUM);        // Use top left corner as text coord datum
      tft.setFreeFont(&FreeSans18pt7b);  // Choose a nicefont that fits box
      tft.setTextColor(DISP_TCOLOR);     // Set the font colour

      // Draw the string, the value returned is the width in pixels
      int xwidth = tft.drawString(numberBuffer, DISP_X + 4, DISP_Y + 12);

      // Now cover up the rest of the line up by drawing a black rectangle.  No flicker this way
      // but it will not work with italic or oblique fonts due to character overlap.
      tft.fillRect(DISP_X + 4 + xwidth, DISP_Y + 1, DISP_W - xwidth - 5, DISP_H - 2, TFT_BLACK);

      delay(10); // UI debouncing
    }
  }
 */ 
}

//------------------------------------------------------------------------------------------
void drawUI() {
  //tft.fillScreen(TFT_BLACK);
  //tft.drawRoundRect(10, 10, 150, 220, 10, tft.color565(19, 19, 19));
  tft.fillScreen(6339);
  tft.fillRoundRect(10, 10, 150, 220, 6, 19049); //tft.color565(19, 19, 19)
  tft.fillRoundRect(20, 20, 130, 60, 10, TFT_BLACK);
  tft.fillRoundRect(20, 90, 130, 60, 20, TFT_BLACK);
  tft.fillRoundRect(20, 160, 130, 60, 30, TFT_BLACK);
  tft.setFreeFont(&FreeMono24pt7b);  // Choose a nicefont that fits box
  tft.setTextColor(TFT_WHITE);     // Set the font colour
  tft.drawString("50%", 60, 30);
  tft.drawString("10%", 60, 100);
  tft.drawString("26ºC", 60, 170);
  //tft.fillRect(10, 10, 150, 220, TFT_DARKGREY);
}

//------------------------------------------------------------------------------------------
void drawKeypad() {
  // Draw the keys
  for (uint8_t row = 0; row < 5; row++) {
    for (uint8_t col = 0; col < 3; col++) {
      uint8_t b = col + row * 3;

      if (b < 3) tft.setFreeFont(LABEL1_FONT);
      else tft.setFreeFont(LABEL2_FONT);

      key[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
                        KEY_Y + row * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
                        KEY_W, KEY_H, TFT_WHITE, keyColor[b], TFT_WHITE,
                        keyLabel[b], KEY_TEXTSIZE);
      key[b].drawButton();
    }
  }
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

//------------------------------------------------------------------------------------------

// Print something in the mini status bar
void status(const char *msg) {
  tft.setTextPadding(240);
  //tft.setCursor(STATUS_X, STATUS_Y);
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft.setTextFont(0);
  tft.setTextDatum(TC_DATUM);
  tft.setTextSize(1);
  tft.drawString(msg, STATUS_X, STATUS_Y);
}

//------------------------------------------------------------------------------------------
