/*
  Este archivo tiene el código relacionado con la pantalla
  - Librería tft_eSPI
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
#define REPEAT_CAL false
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
  45 - 16, 45 - 16, 45 - 16, 210 - 16, 280 - 16, 210 - 16, 280 - 16, 0
};
const int biy[8] = { //icon y top-left corner
  50 - 16, 120 - 16, 190 - 16, 50 - 16, 50 - 16, 120 - 16, 120 - 16, 0
};

// La detección de clics en la pantalla se realiza por periodos
#define TOUCH_PERIOD 50 //periodo
unsigned long touch_now; //marca temporal

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
