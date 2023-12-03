#include "SPI.h"

#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#define TFT_DC 21
#define TFT_CS 22
// Use hardware SPI
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

#include "XPT2046_Touchscreen.h"
#define CS_PIN  5
#define TIRQ_PIN  17
//XPT2046_Touchscreen ts(CS_PIN);  // Param 2 - NULL - No interrupts
//XPT2046_Touchscreen ts(CS_PIN, 255);  // Param 2 - 255 - No interrupts
XPT2046_Touchscreen ts(CS_PIN, TIRQ_PIN);  // Param 2 - Touch IRQ Pin - interrupt enabled polling

void setup() {
  Serial.begin(115200);
  
  tft.begin();                     
  tft.setRotation(3);
  
  ts.begin();
  ts.setRotation(1);

  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_RED);  
  tft.setTextSize(2);               
  tft.setCursor(85,5);              
  tft.print("Touch Demo"); 
  //while (!Serial && (millis() <= 1000));
}

void loop() {
  int radius = 4; 
  int x, y;
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    Serial.print("Pressure = ");
    Serial.print(p.z);
    Serial.print(", x = ");
    x = map(p.x, 430, 3920, 0, 320);
    Serial.print(x);
    //Serial.print(p.x);
    Serial.print(", y = ");
    y = map(p.y, 3850, 200, 0, 240);
    Serial.print(y);
    //Serial.print(p.y);
    
    tft.fillCircle(x, y, radius, ILI9341_YELLOW);
    delay(30);
    Serial.println();
  }
}
