#pragma once

    #define MANUFACTURER        "ITEAD"
    #define DEVICE              "SONOFF_B1"
    #define RELAY_PROVIDER      RELAY_PROVIDER_LIGHT
    #define LIGHT_PROVIDER      LIGHT_PROVIDER_MY92XX
    #define DUMMY_RELAY_COUNT   1

    // Light
    #define LIGHT_CHANNELS      5
    #define MY92XX_MODEL        MY92XX_MODEL_MY9231
    #define MY92XX_CHIPS        2
    #define MY92XX_DI_PIN       12
    #define MY92XX_DCKI_PIN     14
    #define MY92XX_COMMAND      MY92XX_COMMAND_DEFAULT
    #define MY92XX_MAPPING      4, 3, 5, 0, 1
    #define LIGHT_WHITE_FACTOR  (0.9)                    // White LEDs are way more bright in the B1

#define LIGHT_COLDWHITE_MIRED   153
#define LIGHT_WARMWHITE_MIRED   500   

//color converters
#define REDVALUE(x) ((x >> 16) & 0xFF)
#define GREENVALUE(x)  ((x >> 8) & 0xFF)
#define BLUEVALUE(x) ((x >> 0) & 0xFF)

#define MAXHS(x,y) ((x)>(y) ? (x) : (y))
#define MINHS(x,y) ((x)<(y) ? (x) : (y))
#include "my92XX.h"

struct channel_t {

	channel_t();
	channel_t(unsigned char pin, bool inverse);

	unsigned char pin;           // real GPIO pin
	bool inverse;                // whether we should invert the value before using it
	bool state;                  // is the channel ON
	unsigned char inputValue;    // raw value, without the brightness
	unsigned char value;         // normalized value, including brightness
	unsigned char target;        // target value
	double current;              // transition value

};


class Clight: public my92xx
{
public:
  Clight();
  void show();
  static uint32_t Color(uint8_t r, uint8_t g, uint8_t b);
  static uint32_t HSVColor(float h, float s, float v);
  static void ColorToHSI(uint32_t rgbcolor, uint32_t brightness,  double &Hue, double &Saturation, double &Intensity);
  unsigned int _toPWM(unsigned int value, bool gamma, bool inverse);
  unsigned int _toPWM(unsigned char id);
  void _setInputValue(const unsigned char id, const unsigned int value);
  void _setRGBInputValue(unsigned char red, unsigned char green, unsigned char blue);
  void _setValue(const unsigned char id, const unsigned int value);
  void lightUpdate(bool save, bool forward, bool group_forward);
  void _lightApplyBrightnessColor();
  long _toKelvin(const long mireds);
  long  _toMireds(const long kelvin);
  void _lightMireds(const long kelvin);
 float Brigthness;
 float  Hue;
 float Saturation;
 bool IsOn=true;
 bool UseTransitions = false;
protected :
	channel_t _light_channels[LIGHT_CHANNELS];
	bool _light_has_color = true;
	bool _light_use_white = true;
	bool _light_use_cct = true;
	bool _light_use_gamma = true;

	const long VALUE_MIN = 0;
	const long VALUE_MAX = 255;

	const long BRIGHTNESS_MIN = 0;
	const long BRIGHTNESS_MAX = 255;

	const long PWM_MIN = 0;
	const long PWM_MAX = 255;
	const long PWM_LIMIT = 255;

	long _light_cold_mireds = LIGHT_COLDWHITE_MIRED;
	long _light_warm_mireds = LIGHT_WARMWHITE_MIRED;

	long _light_cold_kelvin = (1000000L / _light_cold_mireds);
	long _light_warm_kelvin = (1000000L / _light_warm_mireds);
	long _light_mireds= lround((_light_cold_mireds + _light_warm_mireds) / 2L);
	
};
