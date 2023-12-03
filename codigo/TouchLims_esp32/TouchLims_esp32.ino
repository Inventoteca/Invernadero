// Programa para encontrar el valor minimo y maximo de las coordenadas

#include "XPT2046_Touchscreen.h"
#include "SPI.h"

#define CS_PIN  15
// MOSI=11, MISO=12, SCK=13

//XPT2046_Touchscreen ts(CS_PIN);
#define TIRQ_PIN  2
//XPT2046_Touchscreen ts(CS_PIN);  // Param 2 - NULL - No interrupts
//XPT2046_Touchscreen ts(CS_PIN, 255);  // Param 2 - 255 - No interrupts
XPT2046_Touchscreen ts(CS_PIN, TIRQ_PIN);  // Param 2 - Touch IRQ Pin - interrupt enabled polling

int xmin = 500, ymin = 500;
int xmax = 500, ymax = 500;

void setup() {
  Serial.begin(115200);
  ts.begin();
  ts.setRotation(0);
  //while (!Serial && (millis() <= 1000));
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (p.x < xmin) {
      xmin = p.x;
      Serial.print("xmin = ");
      Serial.println(p.x);
    }
    if (p.y < ymin) {
      ymin = p.y;
      Serial.print("ymin = ");
      Serial.println(p.y);
    }
    if (p.x > xmax) {
      xmax = p.x;
      Serial.print("xmax = ");
      Serial.println(p.x);
    }
    if (p.y > ymax) {
      ymax = p.y;
      Serial.print("ymax = ");
      Serial.println(p.y);
    }
    /*
      Serial.print("Pressure = ");
      Serial.print(p.z);
      Serial.print(", x = ");
      Serial.print(p.x);
      Serial.print(", y = ");
      Serial.print(p.y);
    */
    delay(50);
  }
}
