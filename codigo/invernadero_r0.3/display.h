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

// La detección de clics en la pantalla se realiza por periodos
#define TOUCH_PERIOD 100 //66 //periodo
unsigned long touch_now; //marca temporal

// Pantallas
int pantalla = 0; //seleccionar pantalla 0 (principal)
bool pantalla_inicia = false;
#include "ui0.h"
#include "ui1.h"

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
