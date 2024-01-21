/*
  User interface 1
  Página para establecer el limite de temperatura
*/

// Esta pantalla tiene 2 botones y 1 slider
TFT_eSPI_Button b1[2]; //objetos botón
Slider s11(&tft, 40, 115, 240, 10, 10, 10.0, 30.0, TFT_CYAN, TFT_WHITE);

//--------------------------------------------------------------------------------------
void drawUI1() {
  tft.fillScreen(FONDO);

  tft.setTextFont(4);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(TC_DATUM); //top-centre
  tft.drawString("Limite de temperatura", 160, 10);
  tft.drawString(String(lim_temperatura) + " `C", 160, 40);

  s11.setV(lim_temperatura); //asignar valor del slider
  b1[0].drawButton(); //normal
  s11.drawSlider();

  tft.setTextFont(2);
  b1[1].drawButton();
}

//--------------------------------------------------------------------------------------
void setupUI1() {
  // Inicializar botones
  b1[0].initButtonUL(&tft,  20, 100, 280, 40, PANEL, PANEL, TFT_WHITE, "", 1);
  b1[1].initButtonUL(&tft, 130, 170,  60, 60, PANEL, PANEL, TFT_WHITE, "OK", 1);
  s11.setV(lim_temperatura); //asignar valor del slider
}

//--------------------------------------------------------------------------------------
void loopUI1() {
  if (pantalla_inicia) {
    pantalla_inicia = false;
    digitalWrite(PIN_BEEP, LOW);
    drawUI1();
  }
  else {
    // Pressed will be set true is there is a valid touch on the screen
    uint16_t tx, ty, tz; // To store the touch coordinates
    //bool pressed = tft.getTouch(&tx, &ty, 100); //using threshold
    tft.getTouchRaw(&tx, &ty);
    tft.convertRawXY(&tx, &ty);
    tz = tft.getTouchRawZ();
    bool pressed = tz > 600;
    //Serial.printf("x: %i     ", tx);
    //Serial.printf("y: %i     ", ty);
    //Serial.printf("z: %i \n", tz);

    for (uint8_t i = 0; i < 2; i++) { //revisar los botones
      if (pressed and b1[i].contains(tx, ty)) { //si el botón es tocado
        b1[i].press(true); //marcar como presionado
      } else {
        b1[i].press(false); //en caso contrario, marcar como no presionado
      }

      // Si el botón del slider se está presionando, actualizar slider
      // El ventilador pueder cambiar a modo automático
      if (b1[i].isPressed() && i == 0) {
        s11.drawKnob(PANEL); //dibujar knob para borrar el anterior
        s11.touch(tx); //actualizar valor
        s11.drawSlider(); //dibujar slider

        //dibujar el nuevo valor en la pantalla
        tft.setTextFont(4);
        tft.setTextSize(1);
        tft.setTextColor(TFT_WHITE, FONDO);
        tft.setTextDatum(TC_DATUM); //top-centre
        lim_temperatura = round(s11.getV()); //asignar valor redondeado
        String temp = String(lim_temperatura) + " `C";
        tft.drawString(temp, 160, 40);
        Serial.print("lim_temperatura = ");
        Serial.println(temp);

        //vent_auto = true;
      }

      // Si el botón OK se acaba de presionar, dibujarlo presionado
      if (b1[i].justPressed() && i == 1) {
        tft.setTextFont(2);
        b1[i].drawButton(true); //inverted/pressed
      }

      // Si el botón OK se acaba de soltar, regresar a la pantalla principal
      // también se podría cambiar al modo automático del ventilador
      // como se realiza al pulsar el slider
      if (b1[i].justReleased() && i == 1) {
        //
        digitalWrite(PIN_BEEP, HIGH); //suena un beep
        //
        tft.setTextFont(2);
        b1[i].drawButton(false); //dibujar normal
        Serial.println("clic OK");
        pantalla = 0;
        pantalla_inicia = true;
        //vent_auto = true;
      }

    }//fin for
  }//fin else (pantalla_inicia)
}//fin loopUI0
