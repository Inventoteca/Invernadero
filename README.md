# Invernadero
Maqueta de invernadero automatizado.  

Hardware:  
- ESP32
- Pantalla LCD con controlador ILI9341
- Touch resistivo con controlador XPT2046
- Ventiladores con puente H DRV8833
- Sensor de temperatura y humedad DHT22
- Bomba de agua activada con relevador
- Atomizador de agua
- Sensor de humedad de suelo
- Iluminación con LEDs RGB

En la carpeta "codigo" están los programas escritos en el IDE de Arduino.  
En la carpeta "diagramas" están los diagramas de conexiones creados con Fritzing.  

Los programas utilizan diversas librerías para cada módulo.  

## Pantalla
2.4 pulgadas, resolución 320x240, interfaz SPI, controlador ILI9341.  
http://www.lcdwiki.com/2.4inch_SPI_Module_ILI9341_SKU:MSP2402  

Libreria **Adafruit_ILI9341**  
https://github.com/adafruit/Adafruit_ILI9341  
Esa libreria depende de la librería **Adafruit_GFX** que permite dibujar primitivas gráficas y texto  
https://learn.adafruit.com/adafruit-gfx-graphics-library/graphics-primitives  
Utiliza el hardware SPI. También se puede usar software SPI y elegir los pines, pero la comunicación resulta más lenta.  

Algunos tutoriales recomiendan utilizar la librería **TFT_eSPI**.
Tiene características más avanzadas que la librería anterior.  
https://github.com/Bodmer/TFT_eSPI  
(tutorial 1: https://electropeak.com/learn/interfacing-2-8-inch-tft-lcd-touch-screen-with-esp32)  

## Touch
La pantalla tiene integrado un panel touch resistivo con controlador XPT2046  

Librería **XPT2046_Touchscreen**  
https://github.com/PaulStoffregen/XPT2046_Touchscreen  
Esta librería devuelve valores de coordenadas de 0 a 4096 (en en teoría).
No está calibrada a la resolución de la pantalla.
Para convertir las lecturas a coordenadas de la pantalla, se puede usar la función map().  
Usa hardware SPI, el mismo que la pantalla, de esa forma se reutilizan 3 pines del ESP32 (MISO, MOSI, CLK).  
(tutorial 1: https://bytesnbits.co.uk/spi-lcd-setup-arduino/ juego de bloques, cómo se descarga el código?)  
(ejemplo donde se configuran los pines SPI https://www.esp32.com/viewtopic.php?t=3746 
https://gist.github.com/CelliesProjects/99a56121e7c8a7cb00d5b46270e74b75)  

Este es un fork de la librería aterior, que agrega código para calibración. No se ha probado.  
https://github.com/ardnew/XPT2046_Calibrated  

La librería **URTouch** funcionó correctaente con Arduino Mega, pero no es compatible con ESP32  
http://www.rinkydinkelectronics.com/library.php?id=92  

La librería **TFT_eSPI** incluye funciones para usar el panel touch  
(la documentación dice que esas funciones se podrían eliminar en versiones futuras)  

En esta discusión recomiendan **TFT_eSPI**. También sugieren comprobar el valor de 
presión para no detectar falsos clics si se usa la librería **XPT2046_Touchscreen**  
https://forum.arduino.cc/t/problem-with-il9341-and-xpt2046-phantom-touch/688062  

## Interfaz gráfica
La interfaz gráfica (GUI) se puede crear con diferentes librerías a través de código o de un editor gráfico.  

## Ventiladores
Los ventiladores se conectan a un módulo DRV8833 (puente H) y se controlan 
con 2 pines que envían señales PWM para poder regular la velocidad.  
Cada ventilador puede ser controlado con 2 pines, pero solo se usará 1, ya que no es necesario invertir la dirección de giro.  
https://forum.arduino.cc/t/troubleshooting-voltage-drop-to-motors/1135652  
https://circuitdigest.com/microcontroller-projects/interface-drv8833-dual-motor-driver-module-with-arduino  

## Bomba de agua
Se activa con un relevador que es controlado por 1 pin digital.
Se usa un relevador porque no se necesita regular la velocidad.  

## Atomizador
relevador o solo optoacoplador
pwm directo o a través de optoacoplador

## Sensor de temperatura y humedad
Sensor DHT22. Solo usa 1 pin de comunicación.  

Librería 

## Sensor de humedad de suelo
Salida digital o analógica
Dependiendo de la lectura obtenida, se activa o desactiva la bomba de agua.  

## LEDs
Iluminación con LEDs RGB

## Reloj (opcional)
Módulo RTC para realizar ciclos que dependen de la hora.  
