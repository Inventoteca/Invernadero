/*
  User interface 1
  Página para establecer el limite de temperatura
*/

// Esta pantalla solo tiene 2 botones
TFT_eSPI_Button b1[4]; //objetos botón
const int bx1[4] = { 20,  20,  20, 130}; //x top-left corner
const int by1[4] = { 50, 100, 150, 200}; //y top-left corner
const int bw1[4] = {280, 280, 280,  60}; //wdith
const int bh1[4] = { 40,  40,  40,  40}; //height
char* bl1[4] = {"", "", "", "OK"}; //labels
const uint16_t bc1[4] {PANEL, PANEL, PANEL, PANEL}; //colors

//objetos slider
//Slider s10(&tft, 40,  65, 240, 10, 10, 0.0, 10.0, TFT_WHITE, TFT_CYAN);
//Slider s11(&tft, 40, 115, 240, 10, 10, 0.0, 10.0, TFT_WHITE, TFT_CYAN);
//Slider s12(&tft, 40, 165, 240, 10, 10, 0.0, 10.0, TFT_WHITE, TFT_CYAN);
//Slider s10(&tft, 40,  65, 240, 10, 10, 0.0, 10.0, TFT_CYAN, TFT_WHITE);
//Slider s11(&tft, 40, 115, 240, 10, 10, 0.0, 10.0, TFT_CYAN, TFT_WHITE);
//Slider s12(&tft, 40, 165, 240, 10, 10, 0.0, 10.0, TFT_CYAN, TFT_WHITE);
//Slider s10(&tft, 40,  65, 240, 10, 10, 0.0, 10.0, TFT_WHITE, TFT_RED);
//Slider s11(&tft, 40, 115, 240, 10, 10, 0.0, 10.0, TFT_WHITE, TFT_GREEN);
//Slider s12(&tft, 40, 165, 240, 10, 10, 0.0, 10.0, TFT_WHITE, TFT_BLUE);
Slider s10(&tft, 40,  65, 240, 10, 10, 0.0, 10.0, TFT_RED, TFT_WHITE);
Slider s11(&tft, 40, 115, 240, 10, 10, 0.0, 10.0, TFT_GREEN, TFT_WHITE);
Slider s12(&tft, 40, 165, 240, 10, 10, 0.0, 10.0, TFT_BLUE, TFT_WHITE);

//no hay iconos ni texto aparte
/*const uint8_t* bi0[8] = { //icons (todos tienen icono excepto el último)
  th32_bits, drop32_bits, grass32_bits,
  fan32_bits, lamp32_bits, sprink32_bits, pump32_bits, 0
  };
  const int bix0[8] = { //icon x top-left corner
  45 - 16, 45 - 16, 45 - 16, 210 - 16, 280 - 16, 210 - 16, 280 - 16, 0
  };
  const int biy0[8] = { //icon y top-left corner
  50 - 16, 120 - 16, 190 - 16, 50 - 16, 50 - 16, 120 - 16, 120 - 16, 0
  };*/
//posición del texto de los primeros 3 botones (temperatura, humedad y humedad del suelo)
//const int btx0[3] = {70, 70, 70};
//const int bty0[3] = {40, 110, 180};

//--------------------------------------------------------------------------------------
void setupUI1() {
  // Inicializar botones con los valores declarados arriba
  for (uint8_t i = 0; i < 4; i++) {
    b1[i].initButtonUL(&tft, bx1[i], by1[i], bw1[i], bh1[i], bc1[i], bc1[i], TFT_WHITE, bl1[i], 1);
  }
}

//--------------------------------------------------------------------------------------
void loopUI1() {
  // Pressed will be set true is there is a valid touch on the screen
  uint16_t tx, ty, tz; // To store the touch coordinates
  //bool pressed = tft.getTouch(&tx, &ty, 100); //using threshold
  tft.getTouchRaw(&tx, &ty);
  tft.convertRawXY(&tx, &ty);
  tz = tft.getTouchRawZ();
  bool pressed2 = tz > 600;
  //Serial.printf("x: %i     ", tx);
  //Serial.printf("y: %i     ", ty);
  //Serial.printf("z: %i \n", tz);

  for (uint8_t i = 0; i < 8; i++) { //revisar los botones
    if ((/*pressed or */pressed2) and b0[i].contains(tx, ty)) { //si el botón es tocado
      b0[i].press(true); //marcar como presionado
    } else {
      b0[i].press(false); //en caso contrario, marcar como no presionado
    }

    // Si el botón se acaba de presionar, dibujarlo presionado
    if (b0[i].justPressed()) {
      tft.setTextFont(2);
      b0[i].drawButton(true); //inverted/pressed
    }

    // Si el botón se acaba de soltar, realizar una acción
    if (b0[i].justReleased()) {
      //acción
      tft.setTextFont(2);
      b0[i].drawButton(false); //dibujar normal
      if (i != 7) { //dibujar el icono correspondiente encima del botón
        tft.drawXBitmap(bix0[i], biy0[i], bi0[i], 32, 32, ICON);
      }

      /*
        Cambiar a otra pantalla
        primer se dibuja otra UI y se crean nuevos botones
        (ya podrían estar creados desde el principio)
        Ahora la detección de clics se hace con otro conjunto de botones
        Cómo ordenar los botones?

        En los sliders se dibuja el boton como fondo
        encima se dibuja un rectángulo alargado y un círculo
      */
      switch (i) {
        case 0: //temperatura
          Serial.println("clic temperatura");
          break;
        case 1: //humedad del aire
          Serial.println("clic humedad");
          break;
        case 2: //humedad del suelo
          Serial.println("clic humedad suelo");
          break;
        case 3: //ventilador
          Serial.println("clic ventilador");
          break;
        case 4: //LEDs
          Serial.println("clic leds");
          break;
        case 5: //spray/atomizador
          Serial.println("clic spray");
          break;
        case 6: //bomba de agua
          Serial.println("clic bomba");
          break;
        case 7: //conectar
          Serial.println("clic conectar");
          break;
      }
    }
  }//fin for
}//fin loopUI0

//--------------------------------------------------------------------------------------
void drawUI1() {
  tft.fillScreen(FONDO);

  tft.setTextFont(4);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(TC_DATUM); //top-centre

  tft.drawString("Color R:10 G:10 B:10", 160, 10);
  //tft.drawString("R:10 G:10 B:10", 160, 20);

  for (uint8_t i = 0; i < 3; i++) {
    b1[i].drawButton(); //normal
  }
  tft.setTextFont(2);
  b1[3].drawButton();

  s10.drawSlider();
  s11.drawSlider();
  s12.drawSlider();
}
