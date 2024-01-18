/*
  User interface 5
  Página para establecer el color de la tira de LEDs
*/

// Esta pantalla tiene 4 botones y 3 sliders
TFT_eSPI_Button b5[4]; //objetos botón
const int bx5[4] = { 20,  20,  20, 130}; //x top-left corner
const int by5[4] = { 50, 100, 150, 200}; //y top-left corner
const int bw5[4] = {280, 280, 280,  60}; //wdith
const int bh5[4] = { 40,  40,  40,  40}; //height
char* bl5[4] = {"", "", "", "OK"}; //labels
const uint16_t bc5[4] {PANEL, PANEL, PANEL, PANEL}; //colors

//objetos slider
//Slider s50(&tft, 40,  65, 240, 10, 10, 0.0, 10.0, TFT_WHITE, TFT_CYAN);
//Slider s51(&tft, 40, 115, 240, 10, 10, 0.0, 10.0, TFT_WHITE, TFT_CYAN);
//Slider s52(&tft, 40, 165, 240, 10, 10, 0.0, 10.0, TFT_WHITE, TFT_CYAN);
//Slider s50(&tft, 40,  65, 240, 10, 10, 0.0, 10.0, TFT_CYAN, TFT_WHITE);
//Slider s51(&tft, 40, 115, 240, 10, 10, 0.0, 10.0, TFT_CYAN, TFT_WHITE);
//Slider s52(&tft, 40, 165, 240, 10, 10, 0.0, 10.0, TFT_CYAN, TFT_WHITE);
//Slider s50(&tft, 40,  65, 240, 10, 10, 0.0, 10.0, TFT_WHITE, TFT_RED);
//Slider s51(&tft, 40, 115, 240, 10, 10, 0.0, 10.0, TFT_WHITE, TFT_GREEN);
//Slider s52(&tft, 40, 165, 240, 10, 10, 0.0, 10.0, TFT_WHITE, TFT_BLUE);
Slider s50(&tft, 40,  65, 240, 10, 10, 0.0, 10.0, TFT_RED, TFT_WHITE);
Slider s51(&tft, 40, 115, 240, 10, 10, 0.0, 10.0, TFT_GREEN, TFT_WHITE);
Slider s52(&tft, 40, 165, 240, 10, 10, 0.0, 10.0, TFT_BLUE, TFT_WHITE);


//--------------------------------------------------------------------------------------
void drawUI5() {
  tft.fillScreen(FONDO);

  tft.setTextFont(4);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(TL_DATUM); //top-left

  tft.drawString("Color ", 20, 10);
  tft.drawString("R:" + String(val_r, 0) + " ", 100, 10);
  tft.drawString("G:" + String(val_g, 0) + " ", 170, 10);
  tft.drawString("B:" + String(val_b, 0) + " ", 240, 10);

  for (uint8_t i = 0; i < 3; i++) {
    b5[i].drawButton(); //normal
  }
  s50.setV(val_r);
  s51.setV(val_g);
  s52.setV(val_b);
  s50.drawSlider();
  s51.drawSlider();
  s52.drawSlider();

  tft.setTextFont(2);
  b5[3].drawButton();
}

//--------------------------------------------------------------------------------------
void setupUI5() {
  // Inicializar botones con los valores declarados arriba
  for (uint8_t i = 0; i < 4; i++) {
    b5[i].initButtonUL(&tft, bx5[i], by5[i], bw5[i], bh5[i], bc5[i], bc5[i], TFT_WHITE, bl5[i], 1);
  }
  s50.setV(val_r); //asignar valor del slider
  s51.setV(val_g); //asignar valor del slider
  s52.setV(val_b); //asignar valor del slider
}

//--------------------------------------------------------------------------------------
void loopUI5() {
  if (pantalla_inicia) {
    pantalla_inicia = false;
    drawUI5();
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
    String val;

    for (uint8_t i = 0; i < 4; i++) { //revisar los botones
      if (pressed and b5[i].contains(tx, ty)) { //si el botón es tocado
        b5[i].press(true); //marcar como presionado
      } else {
        b5[i].press(false); //en caso contrario, marcar como no presionado
      }

      // Mover sliders y cambiar salida (PWM)
      if (b5[i].isPressed()) {
        switch (i)
        {
          case 0: //R
            s50.drawKnob(PANEL); //dibujar knob para borrar el anterior
            s50.touch(tx); //actualizar valor
            s50.drawSlider(); //dibujar slider

            //dibujar el nuevo valor en la pantalla
            tft.setTextFont(4);
            tft.setTextSize(1);
            tft.setTextColor(TFT_WHITE, FONDO);
            //tft.setTextDatum(TL_DATUM);

            val_r = round(s50.getV());
            val = "R:" + String(val_r, 0) + "  ";
            tft.drawString(val, 100, 10);
            Serial.println(val);

            //salida PWM
            analogWrite(PIN_R, round(val_r * 25.5));
            break;

          case 1: //G
            s51.drawKnob(PANEL); //dibujar knob para borrar el anterior
            s51.touch(tx); //actualizar valor
            s51.drawSlider(); //dibujar slider

            //dibujar el nuevo valor en la pantalla
            tft.setTextFont(4);
            tft.setTextSize(1);
            tft.setTextColor(TFT_WHITE, FONDO);
            //tft.setTextDatum(TL_DATUM);

            val_g = round(s51.getV());
            val = "G:" + String(val_g, 0) + "  ";
            tft.drawString(val, 170, 10);
            Serial.println(val);

            //salida PWM
            analogWrite(PIN_G, round(val_g * 25.5));
            break;

          case 2: //B
            s52.drawKnob(PANEL); //dibujar knob para borrar el anterior
            s52.touch(tx); //actualizar valor
            s52.drawSlider(); //dibujar slider

            //dibujar el nuevo valor en la pantalla
            tft.setTextFont(4);
            tft.setTextSize(1);
            tft.setTextColor(TFT_WHITE, FONDO);
            //tft.setTextDatum(TL_DATUM);

            val_b = round(s52.getV());
            val = "B:" + String(val_b, 0) + "  ";
            tft.drawString(val, 240, 10);
            Serial.println(val);

            //salida PWM
            analogWrite(PIN_B, round(val_b * 25.5));
            break;
        }
      }

      // Si el botón OK se acaba de presionar, dibujarlo presionado
      if (b5[i].justPressed() && i == 3) {
        tft.setTextFont(2);
        b5[i].drawButton(true); //inverted/pressed
      }

      // Si el botón se acaba de soltar, regresar a la pantalla principal
      if (b5[i].justReleased() && i == 3) {
        tft.setTextFont(2);
        b5[i].drawButton(false); //dibujar normal
        Serial.println("clic OK");
        pantalla = 0;
        pantalla_inicia = true;
      }

    }//fin for
  }//fin else (pantalla_inicia)
}//fin loopUI0
