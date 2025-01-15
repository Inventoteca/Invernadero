/*
  User interface 4
  Página para controlar ventiladores manualmente
*/

// Esta pantalla tiene 3 botones y 2 sliders
TFT_eSPI_Button b4[3]; //objetos botón
//los sliders establecen una velocidad de 0 a 10
Slider s41(&tft, 40,  85, 240, 10, 10, 0.0, 10.0, TFT_CYAN, TFT_WHITE);
Slider s42(&tft, 40, 135, 240, 10, 10, 0.0, 10.0, TFT_CYAN, TFT_WHITE);

//--------------------------------------------------------------------------------------
void drawUI4() {
  tft.fillScreen(FONDO);

  tft.setTextFont(4);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(TL_DATUM); //TC_DATUM top-centre
  //int padding = tft.textWidth("Quick brown", font); // get the width of the widest text in pixels
  //tft.setTextPadding(padding);
  Serial.println("DRAW UI 4 =================");
  Serial.println(tft.drawString("Ventilador_1: ", 60, 10));
  Serial.println(tft.drawString(String(vent1, 0), 230, 10)); //vent1
  tft.drawString("Ventilador_2: ", 60, 40);
  tft.drawString(String(vent2, 0), 230, 40);

  b4[0].drawButton(); //normal
  s41.setV(vent1); //asignar valor del slider
  s41.drawSlider();
  b4[1].drawButton(); //normal
  s42.setV(vent2); //asignar valor del slider
  s42.drawSlider();

  tft.setTextFont(2);
  b4[2].drawButton();
}

//--------------------------------------------------------------------------------------
void setupUI4() {
  // Inicializar botones
  b4[0].initButtonUL(&tft,  20,  70, 280, 40, PANEL, PANEL, TFT_WHITE, "", 1);
  b4[1].initButtonUL(&tft,  20, 120, 280, 40, PANEL, PANEL, TFT_WHITE, "", 1);
  b4[2].initButtonUL(&tft, 130, 170,  60, 60, PANEL, PANEL, TFT_WHITE, "OK", 1);
  s41.setV(vent1); //asignar valor del slider
  s42.setV(vent2); //asignar valor del slider
}

//--------------------------------------------------------------------------------------
void loopUI4() {
  if (pantalla_inicia) {
    pantalla_inicia = false;
    digitalWrite(PIN_BEEP, LOW);
    drawUI4();
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

    for (uint8_t i = 0; i < 3; i++) { //revisar los botones
      if (pressed and b4[i].contains(tx, ty)) { //si el botón es tocado
        b4[i].press(true); //marcar como presionado
      } else {
        b4[i].press(false); //en caso contrario, marcar como no presionado
      }

      // Si el botón del slider41 se está presionando (vent1)
      // actualizar slider y escribir valor en el pin de salida
      if (b4[i].isPressed() && i == 0) {
        s41.drawKnob(PANEL); //dibujar knob para borrar el anterior
        s41.touch(tx); //actualizar valor
        s41.drawSlider(); //dibujar slider

        //dibujar el nuevo valor en la pantalla
        tft.setTextFont(4);
        tft.setTextSize(1);
        tft.setTextColor(TFT_WHITE, FONDO);
        //tft.setTextDatum(TC_DATUM); //top-centre
        vent1 = round(s41.getV()); //asignar valor redondeado
        String sven = String(vent1, 0) + "  ";
        tft.drawString(sven, 230, 10);
        Serial.print("vent1 = ");
        Serial.println(sven);
        //vent_auto = false;
        analogWrite(PIN_VENT1, round(vent1 * 25.5)); //velocidad vent1
        //analogWrite(PIN_VENT2, round(vent2 * 25.5));
      }

      // Si el botón del slider42 se está presionando (vent2)
      // actualizar slider y escribir valor en el pin de salida
      if (b4[i].isPressed() && i == 1) {
        s42.drawKnob(PANEL); //dibujar knob para borrar el anterior
        s42.touch(tx); //actualizar valor
        s42.drawSlider(); //dibujar slider

        //dibujar el nuevo valor en la pantalla
        tft.setTextFont(4);
        tft.setTextSize(1);
        tft.setTextColor(TFT_WHITE, FONDO);
        //tft.setTextDatum(TC_DATUM); //top-centre
        vent2 = round(s42.getV()); //asignar valor redondeado
        String sven = String(vent2, 0) + "  ";
        tft.drawString(sven, 230, 40);
        Serial.print("vent2 = ");
        Serial.println(sven);
        //vent_auto = false;
        //analogWrite(PIN_VENT1, round(vent1 * 25.5));
        analogWrite(PIN_VENT2, round(vent2 * 25.5));
      }

      // Si el botón OK se acaba de presionar, dibujarlo presionado
      if (b4[i].justPressed() && i == 2) {
        tft.setTextFont(2);
        b4[i].drawButton(true); //inverted/pressed
      }

      // Si el botón OK se acaba de soltar, regresar a la pantalla principal
      if (b4[i].justReleased() && i == 2) {
        //
        digitalWrite(PIN_BEEP, HIGH); //suena un beep
        //
        tft.setTextFont(2);
        b4[i].drawButton(false); //dibujar normal
        Serial.println("clic OK");
        pantalla = 0;
        pantalla_inicia = true;
        //bomba_auto = true;
      }

    }//fin for
  }//fin else (pantalla_inicia)
}//fin loopUI0
