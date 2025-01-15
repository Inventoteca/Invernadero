// Definir pines
#define PIN_SUELOD 35  //sensor de humedad del suelo (lectura digital)
#define PIN_SUELOA 34  //sensor de humedad del suelo (lectura analogica)
#define PIN_PHD 39     //sensor de PH (lectura digital)
#define PIN_PHA 36     //sensor de PH (lectura analogica)
#define PIN_VENT1 32   //ventilador 1 (PWM)
#define PIN_VENT2 33   //ventilador 2 (PMW)
#define PIN_R 12       //LED R (PMW)
#define PIN_G 13       //LED G (PMW)
#define PIN_B 14       //LED B (PMW)
#define PIN_REL1 26    //relevador de bomba 1
#define PIN_REL2 25    //relevador de bomba 2
// Los pines para la pantalla y el panel touch se definen en "Setup42_ILI9341_ESP32.h"
// nuevos pines agregados
//#define PIN_BSPRAY 13  //bomba para el spray
//#define PIN_SSPRAY 39  //36//sensor de agua para el spray
//#define PIN_BEEP 12    //2//buzzer piezoeléctrico

// sensor de temperatura y humedad
#define PIN_DHT 27
#include <dhtESP32-rmt.h>
float humedad, temperatura;

bool suelod;  //guarda un estado alto o bajo
int sueloa;   //guarda la lectura analógia (12 bits --> 0 a 4095)
//cuando la resistencia es infinita la lectura es 4095
//cuando la resistencia es cero la lectura es 0
int humedad_suelo = (4095 - sueloa) / 41;

#define T 1500

void setup() {
  Serial.begin(115200);

  // configurar entradas y salidas
  pinMode(PIN_SUELOD, INPUT);
  pinMode(PIN_SUELOA, INPUT);
  pinMode(PIN_PHD, INPUT);
  pinMode(PIN_PHA, INPUT);
  pinMode(PIN_VENT1, OUTPUT);
  pinMode(PIN_VENT2, OUTPUT);
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_REL1, OUTPUT);
  pinMode(PIN_REL2, OUTPUT);  //controlar spray
  //pinMode(PIN_BSPRAY, OUTPUT);  //bomba spray
  //pinMode(PIN_SSPRAY, INPUT);   //sensor spray
  //pinMode(PIN_BEEP, OUTPUT);    //buzzer piezoeléctrico
  //
  analogRead(PIN_SUELOA);  //descartar primera lectura
  analogRead(PIN_PHA);     //descartar primera lectura
  analogWrite(PIN_VENT1, 0);
  analogWrite(PIN_VENT2, 0);
  analogWrite(PIN_R, 0);  //PWM
  analogWrite(PIN_G, 0);
  analogWrite(PIN_B, 0);
  digitalWrite(PIN_REL1, HIGH);  //relevador apagado en HIGH
  digitalWrite(PIN_REL2, HIGH);  //relevador apagado en HIGH
  //
  //digitalWrite(PIN_BEEP, HIGH);
  //digitalWrite(PIN_BEEP, LOW);
}

void loop() {
  // probar ventiladores
  Serial.println("1");
  analogWrite(PIN_VENT1, 0);
  analogWrite(PIN_VENT2, 0);
  analogWrite(PIN_R, 120);
  analogWrite(PIN_G, 0);
  analogWrite(PIN_B, 0);
  digitalWrite(PIN_REL1, HIGH);
  digitalWrite(PIN_REL2, HIGH);
  delay(T);

  Serial.println("2");
  analogWrite(PIN_VENT1, 128);
  analogWrite(PIN_VENT2, 128);
  analogWrite(PIN_R, 0);
  analogWrite(PIN_G, 120);
  analogWrite(PIN_B, 0);
  digitalWrite(PIN_REL1, LOW);
  digitalWrite(PIN_REL2, HIGH);
  delay(T);

  Serial.println("3");
  analogWrite(PIN_VENT1, 255);
  analogWrite(PIN_VENT2, 255);
  analogWrite(PIN_R, 0);
  analogWrite(PIN_G, 0);
  analogWrite(PIN_B, 120);
  digitalWrite(PIN_REL1, HIGH);
  digitalWrite(PIN_REL2, LOW);

  uint8_t dht_error = read_dht(temperatura, humedad, PIN_DHT, DHT22);
  if (dht_error) {
    Serial.print("error: ");
    Serial.println(dht_error);
  } else {
    Serial.print(temperatura);
    Serial.print("\t");
    Serial.println(humedad);
  }

  int suelod = digitalRead(PIN_SUELOD);  //lectura digital
  int sueloa = analogRead(PIN_SUELOA);   //lectura analógica
  humedad_suelo = (4095 - sueloa) / 41;  //calcular porcentaje de humedad
  Serial.print("suelo: ");
  Serial.println(humedad_suelo);

  int phd = digitalRead(PIN_PHD);  //lectura digital
  int pha = analogRead(PIN_PHA);   //lectura analógica
  Serial.print("PH: ");
  Serial.println(pha);

  delay(T);
}
