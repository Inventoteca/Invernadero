/*
  User interface 2
  Página para establecer el limite de humedad del aire
*/

// Esta pantalla tiene 2 botones y 1 slider
TFT_eSPI_Button b2[2]; //objetos botón
//el slider establece un limite de humedad de 0% a 100%
Slider s2(&tft, 40, 150, 400, 10, 10, 0.0, 100.0, ICON, TFT_WHITE);

//--------------------------------------------------------------------------------------
void drawUI2() {
  tft.fillScreen(FONDO);

  tft.setTextFont(4);
  tft.setTextSize(1);
  tft.setTextColor(TFT_BLACK); //WHITE
  tft.setTextDatum(TC_DATUM); //top-centre
  tft.drawString("Limite de humedad del aire", 240, 20);
  tft.drawString("  " + String(lim_humedad) + "%  ", 240, 50);

  s2.setV(lim_humedad); //asignar valor del slider
  b2[0].drawButton(); //normal
  s2.drawSlider();

  tft.setTextFont(2);
  b2[1].drawButton();
}

//--------------------------------------------------------------------------------------
void setupUI2() {
  // Inicializar botones
  // el primero rodea al slider
  b2[0].initButtonUL(&tft,  20, 130, 440, 50, PANEL, PANEL, TFT_BLACK, "", 1);
  // el segundo es para aceptar y regresar a la pantalla principal
  b2[1].initButtonUL(&tft, 210, 240,  60, 60, PANEL, PANEL, TFT_BLACK, "OK", 1);
  s2.setV(lim_humedad); //asignar valor del slider
}

//--------------------------------------------------------------------------------------
void loopUI2() {
  if (pantalla_inicia) {
    pantalla_inicia = false;
    digitalWrite(PIN_BEEP, LOW);
    drawUI2();
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

    for (uint8_t i = 0; i < 2; i++) { //revisar los botones
      if (pressed and b2[i].contains(tx, ty)) { //si el botón es tocado
        b2[i].press(true); //marcar como presionado
      } else {
        b2[i].press(false); //en caso contrario, marcar como no presionado
      }

      // Si el botón del slider se está presionando, actualizar slider
      if (b2[i].isPressed() && i == 0) {
        s2.drawKnob(PANEL); //dibujar knob para borrar el anterior
        s2.touch(tx); //actualizar valor
        s2.drawSlider(); //dibujar slider

        //dibujar el nuevo valor en la pantalla
        tft.setTextFont(4);
        tft.setTextSize(1);
        tft.setTextColor(TFT_BLACK, FONDO); //WHITE
        tft.setTextDatum(TC_DATUM); //top-centre
        lim_humedad = round(s2.getV()); //asignar valor redondeado
        String hum = "  " + String(lim_humedad) + "%  ";
        tft.drawString(hum, 240, 50);
        Serial.print("lim_humedad = ");
        Serial.println(hum);

        //spray_auto = true;
      }

      // Si el botón OK se acaba de presionar, dibujarlo presionado
      if (b2[i].justPressed() && i == 1) {
        tft.setTextFont(2);
        b2[i].drawButton(true); //inverted/pressed
      }

      // Si el botón OK se acaba de soltar, regresar a la pantalla principal
      if (b2[i].justReleased() && i == 1) {
        //
        digitalWrite(PIN_BEEP, HIGH); //suena un beep
        //
        tft.setTextFont(2);
        b2[i].drawButton(false); //dibujar normal
        Serial.println("clic OK");
        pantalla = 0;
        pantalla_inicia = true;
        //spray_auto = true;
      }

    }//fin for
  }//fin else (pantalla_inicia)
}//fin loopUI0
