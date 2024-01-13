/*
  User interface 0
  Página principal
*/

// Botones de la pantalla principal
TFT_eSPI_Button b0[8]; //objetos botón
const int bx0[8] = { 20,  20,  20, 180, 250, 180, 250, 180}; //x top-left corner
const int by0[8] = { 20,  90, 160,  20,  20,  90,  90, 160}; //y top-left corner
const int bw0[8] = {140, 140, 140,  60,  60,  60,  60, 130}; //wdith
const int bh0[8] = { 60,  60,  60,  60,  60,  60,  60,  60}; //height
char* bl0[8] = {"", "", "", "", "", "", "", "CONECTAR"}; //labels
const uint16_t bc0[8] {0, 0, 0, PANEL, PANEL, PANEL, PANEL, PANEL}; //colors
const uint8_t* bi0[8] = { //icons (todos tienen icono excepto el último)
  th32_bits, drop32_bits, grass32_bits,
  fan32_bits, lamp32_bits, sprink32_bits, pump32_bits, 0
};
const int bix0[8] = { //icon x top-left corner
  45 - 16, 45 - 16, 45 - 16, 210 - 16, 280 - 16, 210 - 16, 280 - 16, 0
};
const int biy0[8] = { //icon y top-left corner
  50 - 16, 120 - 16, 190 - 16, 50 - 16, 50 - 16, 120 - 16, 120 - 16, 0
};
//posición del texto de los primeros 3 botones (temperatura, humedad y humedad del suelo)
const int btx0[3] = {70, 70, 70};
const int bty0[3] = {40, 110, 180};

//--------------------------------------------------------------------------------------
void setupUI0() {
  // Inicializar botones con los valores declarados arriba
  for (uint8_t i = 0; i < 8; i++) {
    b0[i].initButtonUL(&tft, bx0[i], by0[i], bw0[i], bh0[i], bc0[i], bc0[i], TFT_WHITE, bl0[i], 1);
  }
}

//--------------------------------------------------------------------------------------
void loopUI0() {
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
          pantalla = 1;
          pantalla_inicia = true;
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
void drawUI0() {
  tft.fillScreen(FONDO);
  // Fuentes
  // En la fuente 1 el signo de grados es \xF7 (hexadecimal) o \367 (octal)
  // En las fuentes 2 y 4 el signo de grados es el acento grave
  // Tamaños que se ven bien
  // - Fuente 1 x3 o x4
  // - Fuente 2 x2 o x3
  // - Fuente 4 x1 porque el pixelado se ve feo

  // Grupo 1
  tft.setTextFont(4);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(TL_DATUM); //top-centre
  tft.fillRoundRect(10, 10, 160, 220, 4, PANEL);
  for (uint8_t i = 0; i < 3; i++) {
    b0[i].drawButton(); //normal
    tft.drawXBitmap(bix0[i], biy0[i], bi0[i], 32, 32, TFT_CYAN);
  }
  //tft.drawString("26 `C", btx0[0], bty0[0]);
  //tft.drawString("50%", btx0[1], bty0[1]);
  //tft.drawString("10%", btx0[2], bty0[2]);

  // Grupo 2
  tft.setTextFont(2);
  tft.setTextSize(1);
  for (uint8_t i = 3; i < 7; i++) {
    b0[i].drawButton(); //normal
    tft.drawXBitmap(bix0[i], biy0[i], bi0[i], 32, 32, TFT_CYAN);
  }
  b0[7].drawButton(); //el último botón no lleva icono
}
