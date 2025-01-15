//---------------------------------------------------------------------------
/*
  Slider
  el area clicable está definida por un botón
  dibuja un rectángulo que representa un riel
  y un círculo que se desliza

  variables:
  forma 1 x,y, w,h (mejor)
  forma 2 x1, y1, x2, y2, grosor
  radio de la perilla
  valor minimo
  valor maximo
  paso (opcional)
  color de la linea
  color de la perilla
  podría tener color de fondo

  la perilla se desplaza por todo lo lago de la barra riel

  cuando se detecta presion en el botón
  se obtiene la posicion del clic respecto al origen (tx - x)
  restringir, mínimo 0, máximo w
  se obtiene el valor (map) y se redondea (round)

  dibujar slider dibuja en la posicion del clic

*/

// Función utilizada por la clase Slider
// como la función map de arduino pero para números de tipo float
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

class Slider {
  public:
    Slider(TFT_eSPI *gfx, int16_t x, int16_t y, uint16_t w, uint16_t h,
           uint16_t r, float v1, float v2, uint16_t bar, uint16_t knob) {
      _gfx = gfx;
      _x = x; _y = y;
      _w = w; _h = h; _r = r;
      _v1 = v1; _v2 = v2;
      _bar = bar; _knob = knob;
      //currstate = false;
      //laststate = false;;
      _kx = _x + _dpx;
      _ky = (_h / 2) + _y;
    }

    void drawSlider(void) {
      //_kx = _x + _dpx;
      //_ky = (_h / 2) + _y;
      _gfx->fillRect(_x, _y, _w, _h, _bar);
      _gfx->fillCircle(_kx, _ky, _r, _knob);
    }

    void drawKnob(uint16_t color) {
      //_gfx->fillRect(_kx - _r, _ky - _r, _r + _r, _r + _r, color);
      _gfx->fillCircle(_kx, _ky, _r, color);
    }

    void touch(uint16_t t) {
      _dpx = t - _x; //calcular delta px
      if (_dpx < 0) _dpx = 0; //restringir al tamaño del slider
      if (_dpx > _w) _dpx = _w;
      _v =  mapfloat(_dpx, 0, _w, _v1, _v2); //calcular valor en el rango definido anteriormente

      // actualizar posición del knob
      _kx = _x + _dpx;
      //_ky = (_h / 2) + _y; //esto siempre vale lo mismo
    }

    // Getters & Setters

    float getV() {//value getter
      return _v;
    }

    // asignar el valor implica asignar también delta pixels (_dpx)
    // es el proceso inverso a touch
    void setV(float v) {
      if (v < _v1) v = _v1; //retringir a los valores minimo y máximo
      if (v > _v2) v = _v2;
      _v = v; //guardar valor en el objeto
      _dpx = round(mapfloat(_v, _v1, _v2, 0, _w)); //calcular delta pixels
      _kx = _x + _dpx; // actualizar posición del knob
      //_ky = (_h / 2) + _y; //esto siempre vale lo mismo
    }

  private:
    TFT_eSPI *_gfx;
    uint16_t _x, _y; // Coordinates of top-left corner
    uint16_t _w, _h, _r; // Width, height and knob radius
    float _v1, _v2, _v; // min and max values, and current value
    //float _pos, _pro; //position (current value) and proportion
    uint16_t _bar, _knob; //colors
    int16_t _dpx; //delta pixels
    int16_t _kx, _ky; //knob pos
}; //fin class Slider
