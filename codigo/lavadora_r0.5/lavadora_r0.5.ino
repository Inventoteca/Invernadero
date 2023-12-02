/***************************************************
  Controlador de lavadora

  Hardware:
  Arduino Mega
  LCD con controlador ILI9341
  Touch resistivo XPT2046
  Módulo de optoacopladores PC817 de 8 canales
  Módulo de relevadores de 8 canales

  Código de la pantalla basado en el ejemplo "graphictest" de Adafruit
****************************************************/

// Tiempos en ms ==================================
// Tiempo para abrir chapa
#define T_CH 100 //milisegundos
// Para lavado corto (C)
#define T_PRELAV_C  7000 //pre lavado corto
#define T_LLE1_C    5000 //llenado 1 corto
#define T_DRN1_C    5000 //drenado corto
#define T_LAV1_C    10000  //lavado (1) corto
#define T_LLE2_C    3000 //llenado 2 corto
#define T_DSQ1_C    2000  //dosificación de químico 1 corto
#define T_DRN2_C    5000 //drenado 2 corto
#define T_ENJ1_C    6000  //enjuague 1 corto
#define T_LLE3_C    3000 //llenado 3 corto
#define T_DSQ2_C    2000  //dosificación de químico 2 corto
#define T_DRN3_C    5000 //drenado 3 corto
#define T_SEC_C     10000 //secado
// Para lavado largo (L)
#define T_PRELAV_L  15000 //pre lavado largo
#define T_LLE1_L    10000 //llenado 1 largo
#define T_DRN1_L    10000 //drenado 1 largo
#define T_LAV1_L    20000  //lavado (1) largo
#define T_LLE2_L    5000 //llenado 2 largo
#define T_DSQ1_L    2000  //dosificación de químico 1 largo
#define T_DRN2_L    10000 //drenado 2 largo
#define T_ENJ1_L    10000  //enjuague (1) largo
#define T_LLE3_L    5000 //llenado 3 largo
#define T_DSQ2_L    2000  //dosificación de químico 2 largo
#define T_DRN3_L    10000 //drenado 3 largo
#define T_SEC_L     20000 //secado largo
//
// El conteo del tiempo se realiza con millis().
// Normalmente se realizan 2 acciones al mismo tiempo,
// para ello se usan 2 variables que guardan el tiempo inicial de
// cada acción y 2 variables que guardan la duración de cada acción.
unsigned long ini1, ini2; //tiempo de inicio de cada acción
unsigned long dur1, dur2; //duración de cada acción
unsigned long t1, t2; //tiempo transcurrido de cada acción
char dur_str[8]; //string para guardar el tiempo en formato mm:ss
char t_str[8];

// Entradas ===============================
#define I_CC  21//A0 //chapa cerrada
#define I_NQ1 21//A1 //nivel químico 1
#define I_NQ2 21//A2 //nivel químico 2
#define I_PRES 21//A3 //presostato

// Salidas ===============================
#define O_BL  34//14 //bomba de lavado
#define O_SEC 34//15 //secador
#define O_RES 34//16 //resistencias
#define O_AF  34//17 //agua fría (valvula?)
#define O_Q1  34//18 //químico 1
#define O_Q2  34//19 //químico 2
#define O_AC  34//20 //abrir chapa
#define O_BD  34//21 //bomba drenado

// Pantalla ===============================
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#define TFT_DC 22//44 //puede ser cualquiera
#define TFT_CS 5//48 //53 SS pin
// use hardware SPI
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// Touch ================================
#include "URTouch.h"
#define t_SCK 17//34
#define t_CS 16//32
#define t_MOSI 4//30
#define t_MISO 2//28
#define t_IRQ 15//26
URTouch ts(t_SCK, t_CS, t_MOSI, t_MISO, t_IRQ);
// estas variables sirven para detectar el momento
// en que se hace clic o se deja de tocar la pantalla
bool toque = false; //toque en el ciclo actual
bool toque_pre = false; //toque en el ciclo previo

// Estados ==============================
enum E {
  INICIO,
  PRELAV, //prelavado
  LLEN, //llenado
  DRN1, //drenado
  DRN2,
  DRN3,
  LAV1, //lavado
  LAV2,
  DSQ1, //dosificación de químico
  DSQ2,
  ENJ1, //enjuague
  ENJ2,
  SEC, //secado
  FIN
};
E estado = INICIO; //primer estado
bool einicia = true; //estado inicia
bool corto = true; //ciclo corto o largo/estandar
//algunos estados llevan varios pasos, por ejemplo llenado y dosificado
byte paso = 0;

void setup() {//=========================================

  // Configurar salidas
  pinMode(O_BL, OUTPUT); //bomba de lavado
  pinMode(O_SEC, OUTPUT); //secador
  pinMode(O_RES, OUTPUT); //resistencias
  pinMode(O_AF, OUTPUT); //agua fría
  pinMode(O_Q1, OUTPUT); //químico 1
  pinMode(O_Q2, OUTPUT); //químico 2
  pinMode(O_AC, OUTPUT); //abrir chapa
  pinMode(O_BD, OUTPUT); //bomba drenado
  // salidas apagadas (implica nivel alto)
  digitalWrite(O_BL, HIGH);
  digitalWrite(O_SEC, HIGH);
  digitalWrite(O_RES, HIGH);
  digitalWrite(O_AF, HIGH);
  digitalWrite(O_Q1, HIGH);
  digitalWrite(O_Q2, HIGH);
  digitalWrite(O_AC, HIGH);
  digitalWrite(O_BD, HIGH);

  // Configurar entradas
  pinMode(I_CC, INPUT_PULLUP);
  pinMode(I_NQ1, INPUT_PULLUP);
  pinMode(I_NQ2, INPUT_PULLUP);
  pinMode(I_PRES, INPUT_PULLUP);

  // Iniciar UART
  Serial.begin(115200);
  Serial.println("CONTROL DE LAVADORA");

  // Iniciar pantalla
  tft.begin();
  /*
    // read diagnostics (optional but can help debug problems)
    uint8_t x = tft.readcommand8(ILI9341_RDMODE);
    Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
    x = tft.readcommand8(ILI9341_RDMADCTL);
    Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
    x = tft.readcommand8(ILI9341_RDPIXFMT);
    Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
    x = tft.readcommand8(ILI9341_RDIMGFMT);
    Serial.print("Image Format: 0x"); Serial.println(x, HEX);
    x = tft.readcommand8(ILI9341_RDSELFDIAG);
    Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX);
  *///
  tft.setRotation(0); //
  //tft.fillScreen(ILI9341_BLACK); //llenar de color negro
  //tft.setCursor(0, 0);
  //tft.setTextColor(ILI9341_WHITE);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(2);
  //tft.setTextWrap(true);
  //tft.cp437(true); //char set
  //nota: los caracteres especiales se escriben en formato octal (\77)

  // Iniciar touch
  ts.InitTouch(PORTRAIT); //LANDSCAPE
  ts.setPrecision(PREC_HI); //PREC_EXTREME
}

void loop(void) {//=======================================
  // Leer panel touch
  int x = -1, y = -1; //coordenadas
  toque_pre = toque;
  toque = false;
  if (ts.dataAvailable()) {
    ts.read();
    x = ts.getX();
    y = ts.getY();
    if ((x != -1) && (y != -1)) {
      toque = true;
      x = 238 - x;
      y = y + 8;
      // dibujar un punto donde se hizo clic
      //tft.fillCircle(x, y, 4, ILI9341_YELLOW);
    }
  }
  bool clic = toque && !toque_pre; // se acaba de hacer clic?

  switch (estado) {

    case INICIO: //-----------------------------------------------
      if (einicia) {//cuando el estado inicia se imprime un menú
        einicia = false; //no imprimir en el sig ciclo
        // salidas apagadas (implica nivel alto)
        digitalWrite(O_BL, HIGH);
        digitalWrite(O_SEC, HIGH);
        digitalWrite(O_RES, HIGH);
        digitalWrite(O_AF, HIGH);
        digitalWrite(O_Q1, HIGH);
        digitalWrite(O_Q2, HIGH);
        digitalWrite(O_AC, HIGH);
        digitalWrite(O_BD, HIGH);
        tft.fillScreen(ILI9341_BLACK); //llenar de color negro
        tft.setCursor(84, 12);
        tft.print("INICIO"); //encabezado
        // boton 1
        tft.drawRoundRect(5, 45, 230, 70, 10, ILI9341_BLUE);
        tft.setCursor(30, 72);
        tft.print("CICLO CORTO");
        // boton 2
        tft.drawRoundRect(5, 125, 230, 70, 10, ILI9341_BLUE);
        tft.setCursor(30, 152);
        tft.print("LAVADO ESTANDAR");
        // boton 3
        tft.drawRoundRect(5, 205, 230, 70, 10, ILI9341_BLUE);
        tft.setCursor(30, 232);
        tft.print("ABRIR PUERTA");
      }
      // mensaje de puerta abierta
      if (digitalRead(I_CC) == HIGH) {
        tft.setCursor(40, 292);
        tft.print("PUERTA ABIERTA");
      }
      else {
        tft.setCursor(40, 292);
        tft.print("              ");
      }
      //si se hace clic, revisar botones
      if (clic) {
        if (pointInRect(x, y, 5, 45, 230, 70)) {
          tft.fillRoundRect(5, 45, 230, 70, 10, ILI9341_WHITE);
          // si las condiciones son correctas, iniciar ciclo
          if (digitalRead(I_CC) == LOW) {
            estado = PRELAV; //siguiente estado PRELAVADO
            einicia = true;
            corto = true; //ciclo corto
            //delay(100);
          }
          tft.fillRect(5, 45, 230, 70, ILI9341_BLACK);
          tft.drawRoundRect(5, 45, 230, 70, 10, ILI9341_BLUE);
          tft.setCursor(30, 72);
          tft.print("CICLO CORTO");
        }
        // clic en el botón de ciclo estandar
        if (pointInRect(x, y, 5, 125, 230, 70)) {
          tft.fillRoundRect(5, 125, 230, 70, 10, ILI9341_WHITE);
          // si las condiciones son correctas, iniciar ciclo
          if (digitalRead(I_CC) == LOW) {
            estado = PRELAV;
            einicia = true;
            corto = false; //ciclo largo (estandar)
            //delay(100);
          }
          tft.fillRect(5, 125, 230, 70, ILI9341_BLACK);
          tft.drawRoundRect(5, 125, 230, 70, 10, ILI9341_BLUE);
          tft.setCursor(30, 152);
          tft.print("LAVADO ESTANDAR");
        }

        // clic en el botón para abrir chapa/puerta
        if (pointInRect(x, y, 5, 205, 230, 70)) {
          digitalWrite(O_AC, LOW); //abrir chapa
          tft.fillRoundRect(5, 205, 230, 70, 10, ILI9341_WHITE);
          //
          delay(T_CH);
          //
          tft.fillRect(5, 205, 230, 70, ILI9341_BLACK);
          tft.drawRoundRect(5, 205, 230, 70, 10, ILI9341_BLUE);
          tft.setCursor(30, 232);
          tft.print("ABRIR PUERTA");
          digitalWrite(O_AC, HIGH); //apagar relay de la chapa
        }
      }

      break; //fin del estado INICIO

    case PRELAV: //prelavado ----------------------------------------
      // En este estado se usa agua fría
      if (einicia) {//cuando el estado inicia se imprime un texto
        einicia = false; //no imprimir en el sig ciclo
        tft.fillScreen(ILI9341_BLACK); //llenar de color negro
        tft.setCursor(12, 12);
        if (corto) tft.print("CICLO CORTO"); //encabezado
        else tft.print("LAVADO ESTANDAR");
        //
        tft.setCursor(12, 52);
        tft.print("PRE LAVADO"); //nombre del estado actual
        //
        tft.drawRoundRect(5, 205, 230, 70, 10, ILI9341_BLUE);
        tft.setCursor(90, 232);
        tft.print("PARAR"); //botón de paro
        //
        // asignar duración de las acciones
        // dependiendo de si el ciclo es corto o largo
        if (corto) {
          dur1 = T_PRELAV_C;
          dur2 = T_LLE1_C;
        }
        else {//largo (estandar)
          dur1 = T_PRELAV_L;
          dur2 = T_LLE1_L;
        }
        // imprimir contador de tiempo
        formatTime(0, t_str); //formato de reloj
        formatTime(dur1 / 1000, dur_str);
        tft.setCursor(12, 92);
        tft.print(t_str); //tiempo transcurrido
        tft.print(" / ");
        tft.print(dur_str); //duración del estado
        //
        // activar salidas
        digitalWrite(O_AF, LOW); //activar agua fría
        digitalWrite(O_BL, LOW); //encender bomba de lavado
        //
        // guardar tiempo de inicio de las acciones
        ini1 = millis();
        ini2 = ini1;
      }
      // calcular tiempo transcurrido
      t1 = (unsigned long)(millis() - ini1);
      t2 = (unsigned long)(millis() - ini2);
      // imprmir tiempo transcurrido
      formatTime(t1 / 1000, t_str);
      tft.setCursor(12, 92);
      tft.print(t_str);
      //
      // detener acciones si se llega a la duración asignada
      if (t1 > dur1) {
        estado = DRN1;
        einicia = true;
        digitalWrite(O_BL, HIGH); //apagar bomba de lavado
      }
      if (t2 > dur2) {
        digitalWrite(O_AF, HIGH); //apagar agua fría
      }
      if (clic) {// clic en el botón paro
        if (pointInRect(x, y, 5, 205, 230, 70)) {
          estado = INICIO;
          einicia = true;
          tft.fillRoundRect(5, 205, 230, 70, 10, ILI9341_WHITE);
          delay(100);
        }
      }
      // imprimir nivles de quimicos
      tft.setCursor(12, 132);
      if (digitalRead(I_NQ1) == HIGH) {
        tft.print("DETERGENTE BAJO");
      } else {
        tft.print("DETERGENTE OK  ");
      }
      tft.setCursor(12, 172);
      if (digitalRead(I_NQ2) == HIGH) {
        tft.print("ABRILLANTADOR BAJO");
      } else {
        tft.print("ABRILLANTADOR OK  ");
      }
      break; //fin del estado PRE LAVADO

    case LLEN: //llenado ------------------------------------------
      //este estado no se utiliza
      //el llenado se hace en el prelavado
      break;

    case DRN1: //drenado 1 -----------------------------------------
      // En este estado se drena el agua del pre lavado
      if (einicia) {//cuando el estado inicia se imprime un texto
        einicia = false; //no imprimir en el sig ciclo
        tft.setCursor(12, 52);
        tft.print("DRENADO 1  "); //nombre del estado actual
        // asignar duración de las acciones
        // dependiendo de si el ciclo es corto o largo
        if (corto) {
          //dur1 = T_LLEN_C;
          dur1 = T_DRN1_C;
        }
        else {//largo (estandar)
          //dur1 = T_LLEN_L;
          dur1 = T_DRN1_L;
        }
        // imprimir contador de tiempo
        formatTime(0, t_str); //formato de reloj
        formatTime(dur1 / 1000, dur_str);
        tft.setCursor(12, 92);
        tft.print(t_str); //tiempo transcurrido
        tft.print(" / ");
        tft.print(dur_str); //duración del estado
        //
        // activar salidas
        //digitalWrite(O_AF, LOW); //activar agua fría
        digitalWrite(O_BD, LOW); //encender bomba de drenado
        //
        // guardar tiempo de inicio de las acciones
        ini1 = millis();
        //ini2 = ini1;
      }
      // calcular tiempo transcurrido
      //t1 = (unsigned long)(millis() - ini1);
      t1 = (unsigned long)(millis() - ini1);
      // imprmir tiempo transcurrido
      formatTime(t1 / 1000, t_str);
      tft.setCursor(12, 92);
      tft.print(t_str);
      //
      // detener acciones si se llega a la duración asignada
      if (t1 > dur1) {
        estado = LAV1;
        einicia = true;
        digitalWrite(O_BD, HIGH); //apagar bomba de drenado
      }
      if (clic) {// clic en el botón paro
        if (pointInRect(x, y, 5, 205, 230, 70)) {
          estado = INICIO;
          einicia = true;
          tft.fillRoundRect(5, 205, 230, 70, 10, ILI9341_WHITE);
          delay(100);
        }
      }
      // imprimir nivles de quimicos
      tft.setCursor(12, 132);
      if (digitalRead(I_NQ1) == HIGH) {
        tft.print("DETERGENTE BAJO");
      } else {
        tft.print("DETERGENTE OK  ");
      }
      tft.setCursor(12, 172);
      if (digitalRead(I_NQ2) == HIGH) {
        tft.print("ABRILLANTADOR BAJO");
      } else {
        tft.print("ABRILLANTADOR OK  ");
      }
      break; //fin del estado DRENADO 1

    case LAV1: //lavado --------------------------------------------
      // En este estado se hace el llenado con agua fría y se usa el quimico 1
      if (einicia) {//cuando el estado inicia se imprime un texto
        einicia = false; //no imprimir en el sig ciclo
        tft.setCursor(12, 52);
        tft.print("LAVADO    "); //nombre del estado actual
        // asignar duración de las acciones
        // dependiendo de si el ciclo es corto o largo
        if (corto) {
          dur1 = T_LAV1_C;
          dur2 = T_LLE2_C;
        }
        else {//largo (estandar)
          dur1 = T_LAV1_L;
          dur2 = T_LLE2_L;
        }
        // imprimir contador de tiempo
        formatTime(0, t_str); //formato de reloj
        formatTime(dur1 / 1000, dur_str);
        tft.setCursor(12, 92);
        tft.print(t_str); //tiempo transcurrido
        tft.print(" / ");
        tft.print(dur_str); //duración del estado
        //
        // activar salidas
        digitalWrite(O_AF, LOW); //activar agua fría
        digitalWrite(O_BL, LOW); //encender bomba de lavado
        //
        // guardar tiempo de inicio de las acciones
        ini1 = millis();
        ini2 = ini1;
        paso = 0; //la acción 2 lleva 2 pasos
      }
      // calcular tiempo transcurrido
      t1 = (unsigned long)(millis() - ini1);
      t2 = (unsigned long)(millis() - ini2);
      // imprmir tiempo transcurrido
      formatTime(t1 / 1000, t_str);
      tft.setCursor(12, 92);
      tft.print(t_str);
      //
      // detener acciones si se llega a la duración asignada
      if (t1 > dur1) {
        estado = DRN2; //siguiente estado DRENADO 2
        einicia = true;
        digitalWrite(O_BL, HIGH); //apagar bomba de lavado
      }
      if (t2 > dur2) {
        if (paso == 0) {//termina llenado
          digitalWrite(O_AF, HIGH); //apagar agua fría
          digitalWrite(O_Q1, LOW); //dosificar químico 1
          ini2 = millis(); //tiempo de inicio de la dosificación
          if (corto) dur2 = T_DSQ1_C; //duración (corto o largo)
          else dur2 = T_DSQ1_L;
          paso = 1; //siguiente paso
        }
        else if (paso == 1) {//termina dosificación
          digitalWrite(O_Q1, HIGH); //químico 1
          paso = 2; //ya no hace nada
        }
      }
      if (clic) {// clic en el botón paro
        if (pointInRect(x, y, 5, 205, 230, 70)) {
          estado = INICIO;
          einicia = true;
          tft.fillRoundRect(5, 205, 230, 70, 10, ILI9341_WHITE);
          delay(100);
        }
      }
      // imprimir nivles de quimicos
      tft.setCursor(12, 132);
      if (digitalRead(I_NQ1) == HIGH) {
        tft.print("DETERGENTE BAJO");
      } else {
        tft.print("DETERGENTE OK  ");
      }
      tft.setCursor(12, 172);
      if (digitalRead(I_NQ2) == HIGH) {
        tft.print("ABRILLANTADOR BAJO");
      } else {
        tft.print("ABRILLANTADOR OK  ");
      }
      break; //fin del estado LAVADO (1)

    case LAV2: //otro LAVADO ------------------------------------------------
      break; //no se utiliza

    case DRN2: //DRENADO 2 --------------------------------------------------
      // En este estado se drena el agua del lavado
      if (einicia) {//cuando el estado inicia se imprime un texto
        einicia = false; //no imprimir en el sig ciclo
        tft.setCursor(12, 52);
        tft.print("DRENADO 2  "); //nombre del estado actual
        // asignar duración de las acciones
        // dependiendo de si el ciclo es corto o largo
        if (corto) {
          dur1 = T_DRN2_C;
        }
        else {//largo (estandar)
          //dur1 = T_LLEN_L;
          dur1 = T_DRN2_L;
        }
        // imprimir contador de tiempo
        formatTime(0, t_str); //formato de reloj
        formatTime(dur1 / 1000, dur_str);
        tft.setCursor(12, 92);
        tft.print(t_str); //tiempo transcurrido
        tft.print(" / ");
        tft.print(dur_str); //duración del estado
        //
        // activar salidas
        digitalWrite(O_BD, LOW); //encender bomba de drenado
        //
        // guardar tiempo de inicio de las acciones
        ini1 = millis();
      }
      // calcular tiempo transcurrido
      t1 = (unsigned long)(millis() - ini1);
      // imprmir tiempo transcurrido
      formatTime(t1 / 1000, t_str);
      tft.setCursor(12, 92);
      tft.print(t_str);
      //
      // detener acciones si se llega a la duración asignada
      if (t1 > dur1) {
        estado = ENJ1;
        einicia = true;
        digitalWrite(O_BD, HIGH); //apagar bomba de drenado
      }
      if (clic) {// clic en el botón paro
        if (pointInRect(x, y, 5, 205, 230, 70)) {
          estado = INICIO;
          einicia = true;
          tft.fillRoundRect(5, 205, 230, 70, 10, ILI9341_WHITE);
          delay(100);
        }
      }
      // imprimir nivles de quimicos
      tft.setCursor(12, 132);
      if (digitalRead(I_NQ1) == HIGH) {
        tft.print("DETERGENTE BAJO");
      } else {
        tft.print("DETERGENTE OK  ");
      }
      tft.setCursor(12, 172);
      if (digitalRead(I_NQ2) == HIGH) {
        tft.print("ABRILLANTADOR BAJO");
      } else {
        tft.print("ABRILLANTADOR OK  ");
      }
      break; //fin del estado DRENADO 2

    case ENJ1: //ENJUAGUE 1 ------------------------------------------
      // En este estado se enjuaga y se usa el quimico 2
      if (einicia) {//cuando el estado inicia se imprime un texto
        einicia = false; //no imprimir en el sig ciclo
        tft.setCursor(12, 52);
        tft.print("ENJUAGUE    "); //nombre del estado actual
        // asignar duración de las acciones
        // dependiendo de si el ciclo es corto o largo
        if (corto) {
          dur1 = T_ENJ1_C;
          dur2 = T_LLE3_C;
        }
        else {//largo (estandar)
          dur1 = T_ENJ1_L;
          dur2 = T_LLE3_L;
        }
        // imprimir contador de tiempo
        formatTime(0, t_str); //formato de reloj
        formatTime(dur1 / 1000, dur_str);
        tft.setCursor(12, 92);
        tft.print(t_str); //tiempo transcurrido
        tft.print(" / ");
        tft.print(dur_str); //duración del estado
        //
        // activar salidas
        digitalWrite(O_AF, LOW); //activar agua fría
        digitalWrite(O_BL, LOW); //encender bomba de lavado
        //
        // guardar tiempo de inicio de las acciones
        ini1 = millis();
        ini2 = ini1;
        paso = 0; //la acción 2 lleva 2 pasos
      }
      // calcular tiempo transcurrido
      t1 = (unsigned long)(millis() - ini1);
      t2 = (unsigned long)(millis() - ini2);
      // imprmir tiempo transcurrido
      formatTime(t1 / 1000, t_str);
      tft.setCursor(12, 92);
      tft.print(t_str);
      //
      // detener acciones si se llega a la duración asignada
      if (t1 > dur1) {
        estado = DRN3; //siguiente estado DRENADO 3
        einicia = true;
        digitalWrite(O_BL, HIGH); //apagar bomba de lavado
      }
      if (t2 > dur2) {
        if (paso == 0) {//termina llenado
          digitalWrite(O_AF, HIGH); //apagar agua fría
          digitalWrite(O_Q2, LOW); //dosificar químico 2
          ini2 = millis(); //tiempo de inicio de la dosificación
          if (corto) dur2 = T_DSQ2_C; //duración (corto o largo)
          else dur2 = T_DSQ2_L;
          paso = 1; //siguiente paso
        }
        else if (paso == 1) {//termina dosificación
          digitalWrite(O_Q2, HIGH); //químico 1
          paso = 2; //ya no hace nada
        }
      }
      if (clic) {// clic en el botón paro
        if (pointInRect(x, y, 5, 205, 230, 70)) {
          estado = INICIO;
          einicia = true;
          tft.fillRoundRect(5, 205, 230, 70, 10, ILI9341_WHITE);
          delay(100);
        }
      }
      // imprimir nivles de quimicos
      tft.setCursor(12, 132);
      if (digitalRead(I_NQ1) == HIGH) {
        tft.print("DETERGENTE BAJO");
      } else {
        tft.print("DETERGENTE OK  ");
      }
      tft.setCursor(12, 172);
      if (digitalRead(I_NQ2) == HIGH) {
        tft.print("ABRILLANTADOR BAJO");
      } else {
        tft.print("ABRILLANTADOR OK  ");
      }
      break; //fin del estado ENJUAGUE

    case ENJ2: //enjuague 2 ----------------------------------------
      break; //no se utiliza

    case DRN3: //DRENADO 3 -----------------------------------------
      // En este estado se drena el agua del enjuague
      if (einicia) {//cuando el estado inicia se imprime un texto
        einicia = false; //no imprimir en el sig ciclo
        tft.setCursor(12, 52);
        tft.print("DRENADO 3  "); //nombre del estado actual
        // asignar duración de las acciones
        // dependiendo de si el ciclo es corto o largo
        if (corto) {
          dur1 = T_DRN3_C;
        }
        else {//largo (estandar)
          dur1 = T_DRN3_L;
        }
        // imprimir contador de tiempo
        formatTime(0, t_str); //formato de reloj
        formatTime(dur1 / 1000, dur_str);
        tft.setCursor(12, 92);
        tft.print(t_str); //tiempo transcurrido
        tft.print(" / ");
        tft.print(dur_str); //duración del estado
        //
        // activar salidas
        digitalWrite(O_BD, LOW); //encender bomba de drenado
        //
        // guardar tiempo de inicio de las acciones
        ini1 = millis();
      }
      // calcular tiempo transcurrido
      t1 = (unsigned long)(millis() - ini1);
      // imprmir tiempo transcurrido
      formatTime(t1 / 1000, t_str);
      tft.setCursor(12, 92);
      tft.print(t_str);
      //
      // detener acciones si se llega a la duración asignada
      if (t1 > dur1) {
        estado = SEC; //siguiente estado SECADO
        einicia = true;
        digitalWrite(O_BD, HIGH); //apagar bomba de drenado
      }
      if (clic) {// clic en el botón paro
        if (pointInRect(x, y, 5, 205, 230, 70)) {
          estado = INICIO;
          einicia = true;
          tft.fillRoundRect(5, 205, 230, 70, 10, ILI9341_WHITE);
          delay(100);
        }
      }
      // imprimir nivles de quimicos
      tft.setCursor(12, 132);
      if (digitalRead(I_NQ1) == HIGH) {
        tft.print("DETERGENTE BAJO");
      } else {
        tft.print("DETERGENTE OK  ");
      }
      tft.setCursor(12, 172);
      if (digitalRead(I_NQ2) == HIGH) {
        tft.print("ABRILLANTADOR BAJO");
      } else {
        tft.print("ABRILLANTADOR OK  ");
      }
      break; //fin del estado DRENADO 3

    case SEC: //SECADO --------------------------------------------
      // En este estado se activa el secador
      if (einicia) {//cuando el estado inicia se imprime un texto
        einicia = false; //no imprimir en el sig ciclo
        tft.setCursor(12, 52);
        tft.print("SECADO    "); //nombre del estado actual
        // asignar duración de las acciones
        // dependiendo de si el ciclo es corto o largo
        if (corto) {
          dur1 = T_SEC_C;
        }
        else {//largo (estandar)
          dur1 = T_SEC_L;
        }
        // imprimir contador de tiempo
        formatTime(0, t_str); //formato de reloj
        formatTime(dur1 / 1000, dur_str);
        tft.setCursor(12, 92);
        tft.print(t_str); //tiempo transcurrido
        tft.print(" / ");
        tft.print(dur_str); //duración del estado
        //
        // activar salidas
        digitalWrite(O_SEC, LOW); //encender secador
        //
        // guardar tiempo de inicio de las acciones
        ini1 = millis();
      }
      // calcular tiempo transcurrido
      t1 = (unsigned long)(millis() - ini1);
      // imprmir tiempo transcurrido
      formatTime(t1 / 1000, t_str);
      tft.setCursor(12, 92);
      tft.print(t_str);
      //
      // detener acciones si se llega a la duración asignada
      if (t1 > dur1) {
        estado = FIN; //siguiente estado INICIO
        einicia = true;
        digitalWrite(O_SEC, HIGH); //apagar bomba de drenado
      }
      if (clic) {// clic en el botón paro
        if (pointInRect(x, y, 5, 205, 230, 70)) {
          estado = INICIO;
          einicia = true;
          tft.fillRoundRect(5, 205, 230, 70, 10, ILI9341_WHITE);
          delay(100);
        }
      }
      // imprimir nivles de quimicos
      tft.setCursor(12, 132);
      if (digitalRead(I_NQ1) == HIGH) {
        tft.print("DETERGENTE BAJO");
      } else {
        tft.print("DETERGENTE OK  ");
      }
      tft.setCursor(12, 172);
      if (digitalRead(I_NQ2) == HIGH) {
        tft.print("ABRILLANTADOR BAJO");
      } else {
        tft.print("ABRILLANTADOR OK  ");
      }
      break; //final del estado SECADO

    case FIN: //------------------------------------------------
      // Ciclo completado
      if (einicia) {//cuando el estado inicia se imprime un texto
        einicia = false; //no imprimir en el sig ciclo
        tft.setCursor(12, 52);
        tft.print("TERMINADO   "); //mensaje
        tft.setCursor(12, 92);
        //tft.print("----------------");
        tft.print("================");
        tft.setCursor(12, 132);   //
        tft.print("                   ");
        tft.setCursor(12, 172);
        tft.print("                   ");
        //
        //tft.drawRoundRect(5, 205, 230, 70, 10, ILI9341_BLUE);
        tft.setCursor(84, 232);
        tft.print("  OK  "); //botón OK
        ini1 = millis();
        paso = 0;
      }
      // parpadeo
      t1 = (unsigned long)(millis() - ini1);
      if (t1 > 500) {
        ini1 = millis();
        tft.setCursor(12, 92);
        if (paso == 0) {
          tft.print("                ");
          paso = 1;
        } else {
          tft.print("================");
          paso = 0;
        }
      }
      if (clic) {// clic en el botón OK
        if (pointInRect(x, y, 5, 205, 230, 70)) {
          estado = INICIO;
          einicia = true;
          tft.fillRoundRect(5, 205, 230, 70, 10, ILI9341_WHITE);
          delay(100);
        }
      }
      break;

    case DSQ1: //dosificación de químico
      break; //no se utiliza
    case DSQ2:
      break;
  }
}

bool pointInRect(int px, int py, int x, int y, int w, int h) {
  //return (px >= x && px <= (x + w) && py <= y && py >= (y - h));
  return (px > x && px < (x + w) && py > y && py < (y + h));
}

void formatTime(unsigned int sec, char * str) {
  unsigned int m = sec / 60; //% 60; //minutos
  unsigned int s = sec % 60; //segundos
  sprintf(str, "%02u:%02u" , m, s);
  //sprintf(str;%02d:%d:%d&quot;,hr, min, sec);
}
