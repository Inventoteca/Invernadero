/*
  User interface 0
  Página principal
*/

// Botones de la pantalla principal
TFT_eSPI_Button b0[8]; //objetos botón
const int bx0[8] = { 30,  30,  30, 280, 380, 280, 380, 280}; //x top-left corner
const int by0[8] = { 30, 100, 170,  20,  20, 120, 120, 240}; //y top-left corner
const int bw0[8] = {210, 210, 210,  80,  80,  80,  80, 180}; //wdith
const int bh0[8] = { 50,  50,  50,  80,  80,  80,  80,  60}; //height
char* bl0[8] = {"", "", "", "", "", "", "", "CONECTAR"}; //labels
const uint16_t bc0[8] {TFT_WHITE, TFT_WHITE, TFT_WHITE, PANEL, PANEL, PANEL, PANEL, PANEL}; //colors
const uint8_t* bi0[8] = { //icons (todos tienen icono excepto el último)
  th32_bits, drop32_bits, grass32_bits,
  fan32_bits, lamp32_bits, sprink32_bits, pump32_bits, 0
};
const int bix0[8] = { //icon x top-left corner
  40, 40, 40, 320 - 16, 420 - 16, 320 - 16, 420 - 16, 0
};
const int biy0[8] = { //icon y top-left corner
  40, 110, 180, 60 - 16, 60 - 16, 160 - 16, 160 - 16, 0
};
//posición del texto de los primeros 3 botones (temperatura, humedad y humedad del suelo)
const int btx0[3] = {80, 80, 80};
const int bty0[3] = {45, 115, 185};

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
  tft.setTextColor(TFT_BLACK); //WHITE
  tft.setTextDatum(TL_DATUM); //top-centre
  tft.fillRoundRect(10, 10, 250, 300, 4, PANEL);
  for (uint8_t i = 0; i < 3; i++) {
    b0[i].drawButton(); //normal
    tft.drawXBitmap(bix0[i], biy0[i], bi0[i], 32, 32, ICON);
  }
  // nuevo rectangulo para mostrar PH
  tft.fillRoundRect(30, 240, 210, 50, 12, TFT_WHITE);
  tft.drawXBitmap(40, 250, ph32_bits, 32, 32, ICON);
  //
  tft.drawString(String(temperatura, 0) + " `C", btx0[0], bty0[0]);
  tft.drawString(String(humedad, 0) + "%  ", btx0[1], bty0[1]);
  tft.drawString(String(humedad_suelo) + "%  ", btx0[2], bty0[2]);

  // Grupo 2
  tft.setTextFont(2);
  tft.setTextSize(1);
  for (uint8_t i = 3; i < 5; i++) {
    b0[i].drawButton(); //normal
    tft.drawXBitmap(bix0[i], biy0[i], bi0[i], 32, 32, ICON); //TFT_CYAN
  }
  //icono de spray cambia de color
  b0[5].drawButton(); //normal
  tft.drawXBitmap(bix0[5], biy0[5], bi0[5], 32, 32, spray ? ICON : 0);
  //icono de bomba cambia de color
  b0[6].drawButton(); //normal
  tft.drawXBitmap(bix0[6], biy0[6], bi0[6], 32, 32, bomba ? ICON : 0);
  //el último botón no lleva icono
  b0[7].drawButton();
}

//--------------------------------------------------------------------------------------
void setupUI0() {
  // Inicializar botones con los valores declarados arriba
  for (uint8_t i = 0; i < 8; i++) {
    b0[i].initButtonUL(&tft, bx0[i], by0[i], bw0[i], bh0[i], bc0[i], bc0[i], TFT_BLACK, bl0[i], 1); //WHITE
  }
}

//--------------------------------------------------------------------------------------
void loopUI0() {
  if (pantalla_inicia) {
    pantalla_inicia = false;
    digitalWrite(PIN_BEEP, LOW); //cuando inicia la pantalla, apagar buzzer
    drawUI0();
  }
  else {
    // Pressed will be set true is there is a valid touch on the screen
    uint16_t tx, ty, tz; // To store the touch coordinates
    //bool pressed = tft.getTouch(&tx, &ty, 100); //using threshold
    tft.getTouchRaw(&tx, &ty);
    tft.convertRawXY(&tx, &ty);
    tz = tft.getTouchRawZ();
    bool pressed2 = tz > 500; //600
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
        digitalWrite(PIN_BEEP, HIGH); //suena un beep
        tft.setTextFont(2);
        b0[i].drawButton(false); //dibujar normal
        if (i != 7) { //dibujar el icono correspondiente encima del botón
          tft.drawXBitmap(bix0[i], biy0[i], bi0[i], 32, 32, ICON);
        }

        switch (i) {
          case 0: //temperatura
            Serial.println("clic temperatura");
            vent_auto = true; //ventilador automático
            pantalla = 1;
            pantalla_inicia = true;
            break;
          case 1: //humedad del aire
            Serial.println("clic humedad");
            spray_auto = true; //spray/nebulizador automático
            pantalla = 2;
            pantalla_inicia = true;
            break;
          case 2: //humedad del suelo
            Serial.println("clic humedad suelo");
            bomba_auto = true; //bomba automática
            pantalla = 3;
            pantalla_inicia = true;
            break;
          case 3: //ventilador
            Serial.println("clic ventilador");
            vent_auto = false; //ventilador manual
            pantalla = 4;
            pantalla_inicia = true;
            break;
          case 4: //LEDs
            Serial.println("clic leds");
            pantalla = 5;
            pantalla_inicia = true;
            break;
          case 5: //spray/atomizador
            Serial.println("clic spray");
            spray_auto = false; //spray/nebulizador manual
            spray = not spray; //invertir estado
            digitalWrite(PIN_SPRAY, spray ? LOW : HIGH); //encendido en nivel bajo
            //digitalWrite(PIN_SPRAY, HIGH); //recuerda, el relevador se activa en bajo
            //delay(T_PULSO_SPRAY); //tiempo que tarda el pulso
            //digitalWrite(PIN_SPRAY, LOW); //activar relevador
            tft.drawXBitmap(bix0[5], biy0[5], bi0[5], 32, 32, spray ? ICON : 0); //cambiar color del icono
            Serial.println(spray ? "Spray ON (manual)" : "Spray OFF (manual)");
            delay(T_PULSO_BEEP); //retardo para que se escuche el beep
            digitalWrite(PIN_BEEP, LOW); //apagar buzzer
            break;
          case 6: //bomba de agua
            Serial.println("clic bomba");
            bomba_auto = false; //bomba manual
            bomba = not bomba; //invertir estado
            digitalWrite(PIN_BOMBA, bomba ? LOW : HIGH); //encendido en nivel bajo
            tft.drawXBitmap(bix0[6], biy0[6], bi0[6], 32, 32, bomba ? ICON : 0); //cambiar color del icono
            Serial.println(bomba ? "Bomba ON (manual)" : "Bomba OFF (manual)");
            delay(T_PULSO_BEEP); //retardo para que se escuche el beep
            digitalWrite(PIN_BEEP, LOW); //apagar buzzer
            break;
          case 7: //conectar
            Serial.println("clic conectar");
            pantalla = 6;
            pantalla_inicia = true;
            break;
        }
      }
    }//fin for
  }//fin else (pantalla_inicia)
}//fin loopUI0
