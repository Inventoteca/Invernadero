/*
  User interface 3
  Página para establecer el limite de humedad del suelo
*/

// Esta pantalla tiene 2 botones y 1 slider
TFT_eSPI_Button b3[2]; //objetos botón
//el slider establece un limite de humedad del suelo d 0% a 100%
Slider s3(&tft, 40, 115, 240, 10, 10, 0.0, 100.0, TFT_CYAN, TFT_WHITE);

//--------------------------------------------------------------------------------------
void drawUI3() {
  tft.fillScreen(FONDO);

  tft.setTextFont(4);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(TC_DATUM); //top-centre
  tft.drawString("Lim. humedad del suelo", 160, 10);
  tft.drawString(String(lim_humedad_suelo) + "%", 160, 40);

  s3.setV(lim_humedad_suelo); //asignar valor del slider
  b3[0].drawButton(); //normal
  s3.drawSlider();

  tft.setTextFont(2);
  b3[1].drawButton();
}

//--------------------------------------------------------------------------------------
void setupUI3() {
  // Inicializar botones
  b3[0].initButtonUL(&tft,  20, 100, 280, 40, PANEL, PANEL, TFT_WHITE, "", 1);
  b3[1].initButtonUL(&tft, 130, 170,  60, 60, PANEL, PANEL, TFT_WHITE, "OK", 1);
  s3.setV(lim_humedad_suelo); //asignar valor del slider
}

//--------------------------------------------------------------------------------------
void loopUI3() {
  if (pantalla_inicia) {
    pantalla_inicia = false;
    drawUI3();
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
      if (pressed and b3[i].contains(tx, ty)) { //si el botón es tocado
        b3[i].press(true); //marcar como presionado
      } else {
        b3[i].press(false); //en caso contrario, marcar como no presionado
      }

      // Si el botón del slider se está presionando, actualizar slider
      if (b3[i].isPressed() && i == 0) {
        s3.drawKnob(PANEL); //dibujar knob para borrar el anterior
        s3.touch(tx); //actualizar valor
        s3.drawSlider(); //dibujar slider

        //dibujar el nuevo valor en la pantalla
        tft.setTextFont(4);
        tft.setTextSize(1);
        tft.setTextColor(TFT_WHITE, FONDO);
        tft.setTextDatum(TC_DATUM); //top-centre
        lim_humedad_suelo = round(s3.getV()); //asignar valor redondeado
        String hums = "  " + String(lim_humedad_suelo) + "%  ";
        tft.drawString(hums, 160, 40);
        Serial.print("lim_humedad_suelo = ");
        Serial.println(hums);

        //bomba_auto = true;
      }

      // Si el botón OK se acaba de presionar, dibujarlo presionado
      if (b3[i].justPressed() && i == 1) {
        tft.setTextFont(2);
        b3[i].drawButton(true); //inverted/pressed
      }

      // Si el botón OK se acaba de soltar, regresar a la pantalla principal
      if (b3[i].justReleased() && i == 1) {
        tft.setTextFont(2);
        b3[i].drawButton(false); //dibujar normal
        Serial.println("clic OK");
        pantalla = 0;
        pantalla_inicia = true;
        //bomba_auto = true;
      }

    }//fin for
  }//fin else (pantalla_inicia)
}//fin loopUI0
