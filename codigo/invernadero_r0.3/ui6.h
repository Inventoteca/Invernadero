/*
  User interface 6 - Conectar
  Página que muestra códigos QR
*/

// Esta pantalla tiene 1 botón y muestra 2 códigos QR
TFT_eSPI_Button b6; //objetos botón

//--------------------------------------------------------------------------------------
void drawBitmapScale(TFT_eSPI *tf, int x, int y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t fg, uint8_t sc) {
  int32_t i, j, byteWidth = (w + 7) / 8;

  for (j = 0; j < h; j++) {
    for (i = 0; i < w; i++ ) {
      //if (pgm_read_byte(bitmap + j * byteWidth + i / 8) & (128 >> (i & 7))) {
      if (pgm_read_byte(bitmap + j * byteWidth + i / 8) & (1 << (i & 7))) {
        //drawPixel(x + i, y + j, color);
        tf->fillRect(x + (i * sc), y + (j * sc), sc, sc, fg);
      }
    }
  }
}

//--------------------------------------------------------------------------------------
void drawUI6() {
  tft.fillScreen(TFT_WHITE);

  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextColor(FONDO); //TFT_WHITE
  tft.setTextDatum(TC_DATUM); //top-centre
  tft.drawString("Nombre de red: Invernadero", 160, 10);
  tft.drawString("URL: 192.168.4.1", 160, 25);

  //drawBitmapScale(&tft, 0, 47, qr_wifi_29, 29, 29, TFT_CYAN, 5);
  //drawBitmapScale(&tft, 170, 45, qr_url_25, 25, 25, TFT_CYAN, 6);
  drawBitmapScale(&tft, 10, 62, qr_wifi_29, 29, 29, FONDO, 4);
  drawBitmapScale(&tft, 185, 57, qr_url_25, 25, 25, FONDO, 5);

  //tft.setTextFont(2);
  b6.drawButton();
}

//--------------------------------------------------------------------------------------
void setupUI6() {
  // Inicializar botones
  b6.initButtonUL(&tft, 130, 200, 60, 40, PANEL, PANEL, TFT_WHITE, "OK", 1);
}

//--------------------------------------------------------------------------------------
void loopUI6() {
  if (pantalla_inicia) {
    pantalla_inicia = false;
    drawUI6();
  }
  else {
    // Pressed will be set true is there is a valid touch on the screen
    uint16_t tx, ty, tz; // To store the touch coordinates
    bool pressed = tft.getTouch(&tx, &ty, 100); //using threshold
    //tft.getTouchRaw(&tx, &ty);
    //tft.convertRawXY(&tx, &ty);
    //tz = tft.getTouchRawZ();
    //bool pressed = tz > 600;
    //Serial.printf("x: %i     ", tx);
    //Serial.printf("y: %i     ", ty);
    //Serial.printf("z: %i \n", tz);

    //for (uint8_t i = 0; i < 2; i++) { //revisar los botones
    if (pressed and b6.contains(tx, ty)) { //si el botón es tocado
      b6.press(true); //marcar como presionado
    } else {
      b6.press(false); //en caso contrario, marcar como no presionado
    }

    // Si el botón OK se acaba de presionar, dibujarlo presionado
    if (b6.justPressed()) {
      tft.setTextFont(2);
      b6.drawButton(true); //inverted/pressed
    }

    // Si el botón OK se acaba de soltar, regresar a la pantalla principal
    if (b6.justReleased()) {
      tft.setTextFont(2);
      b6.drawButton(false); //dibujar normal
      Serial.println("clic OK");
      pantalla = 0;
      pantalla_inicia = true;
    }

    //}//fin for
  }//fin else (pantalla_inicia)
}//fin loopUI6
