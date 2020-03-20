#include "CLight.h"

uint8_t ch_map[LIGHT_CHANNELS]={4, 3, 5, 0, 1};
const unsigned char _light_gamma_table[] PROGMEM = {
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   2,   2,   2,   2,   2,   2,
	3,   3,   3,   3,   3,   3,   4,   4,   4,   4,   5,   5,   5,   5,   6,   6,
	6,   7,   7,   7,   7,   8,   8,   8,   9,   9,   9,   10,  10,  11,  11,  11,
	12,  12,  13,  13,  14,  14,  14,  15,  15,  16,  16,  17,  17,  18,  18,  19,
	19,  20,  20,  21,  22,  22,  23,  23,  24,  25,  25,  26,  26,  27,  28,  28,
	29,  30,  30,  31,  32,  33,  33,  34,  35,  35,  36,  37,  38,  39,  39,  40,
	41,  42,  43,  43,  44,  45,  46,  47,  48,  49,  50,  50,  51,  52,  53,  54,
	55,  56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  71,
	72,  73,  74,  75,  76,  77,  78,  80,  81,  82,  83,  84,  86,  87,  88,  89,
	91,  92,  93,  94,  96,  97,  98,  100, 101, 102, 104, 105, 106, 108, 109, 110,
	112, 113, 115, 116, 118, 119, 121, 122, 123, 125, 126, 128, 130, 131, 133, 134,
	136, 137, 139, 140, 142, 144, 145, 147, 149, 150, 152, 154, 155, 157, 159, 160,
	162, 164, 166, 167, 169, 171, 173, 175, 176, 178, 180, 182, 184, 186, 187, 189,
	191, 193, 195, 197, 199, 201, 203, 205, 207, 209, 211, 213, 215, 217, 219, 221,
	223, 225, 227, 229, 231, 233, 235, 238, 240, 242, 244, 246, 248, 251, 253, 255
};
// See cores/esp8266/WMath.cpp::map
// Redefining as local method here to avoid breaking in unexpected ways in inputs like (0, 0, 0, 0, 1)
template <typename T, typename Tin, typename Tout> T _lightMap(T x, Tin in_min, Tin in_max, Tout out_min, Tout out_max) {
	auto divisor = (in_max - in_min);
	if (divisor == 0) {
		return -1; //AVR returns -1, SAM returns 0
	}
	return (x - in_min) * (out_max - out_min) / divisor + out_min;
}
channel_t::channel_t() :
	pin(0),
	inverse(false),
	state(true),
	inputValue(0),
	value(0),
	target(0),
	current(0.0)
{}

channel_t::channel_t(unsigned char pin, bool inverse) :
	pin(pin),
	inverse(inverse),
	state(true),
	inputValue(0),
	value(0),
	target(0),
	current(0.0)
{
	pinMode(pin, OUTPUT);
}

Clight::Clight():my92xx(MY92XX_MODEL, MY92XX_CHIPS, MY92XX_DI_PIN, MY92XX_DCKI_PIN, MY92XX_COMMAND){
  Brigthness=90;
  Hue=0;
  Saturation=0;
  IsOn=true;
}

void Clight::show(){

	//Serial.println(String("Show") + String(Hue) + String(" s") + String(Saturation) + String("b") + String(Brigthness));
	uint32_t color = HSVColor(Hue, Saturation/100.0, Brigthness/100.0);
	
	_setRGBInputValue(REDVALUE(color), GREENVALUE(color), BLUEVALUE(color));
	//Serial.println("red" + String(REDVALUE(color)) + " green" + String(GREENVALUE(color)) + " blue" + String(BLUEVALUE(color)));
	//Serial.println("red" + String(REDVALUE(color)) + " green" + String(GREENVALUE(color)) + " blue" + String(BLUEVALUE(color)));
	_lightApplyBrightnessColor();
	lightUpdate(true, true, true);
	for (unsigned char i = 0; i < LIGHT_CHANNELS; i++) {
	
		uint8_t val = _toPWM(i);
		//Serial.println(String("Set channel:") + String(i) + String(" value") + String(val));
		this->setChannel(ch_map[i], val);
	}
	this->setState(true);
    this->update();
}

void Clight::_setInputValue(const unsigned char id, const unsigned int value) {
	_light_channels[id].inputValue = value;
}

void Clight::_setRGBInputValue(unsigned char red, unsigned char green, unsigned char blue) {
	_setInputValue(0, constrain(red, VALUE_MIN, VALUE_MAX));
	_setInputValue(1, constrain(green, VALUE_MIN, VALUE_MAX));
	_setInputValue(2, constrain(blue, VALUE_MIN, VALUE_MAX));
}


void Clight::lightUpdate(bool save, bool forward, bool group_forward) {

	// Calculate values based on inputs and brightness
	

	// Only update if a channel has changed
	

	// Update channels
	for (unsigned int i = 0; i < LIGHT_CHANNELS; i++) {
		_light_channels[i].target = IsOn && _light_channels[i].state ? _light_channels[i].value : 0;
		if (!UseTransitions) {
			_light_channels[i].current = _light_channels[i].target;
			//Serial.println(String("Set channel current:") + String(i) + String(" value") + String(_light_channels[i].current));
		}
		//DEBUG_MSG_P("[LIGHT] Channel #%u target value: %u\n", i, _light_channels[i].target);
	}
}
uint32_t Clight::Color(uint8_t r, uint8_t g, uint8_t b) {
	return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

// Convert Hue/Saturation/Brightness values to a packed 32-bit RBG color.
// hue must be a float value between 0 and 360
// saturation must be a float value between 0 and 1
// brightness must be a float value between 0 and 1
uint32_t  Clight::HSVColor(float h, float s, float v) {
	//Serial.println(String("HSVColor") + String(h) + String(" s") + String(s)+String("b")+String(v));
	h = constrain(h, 0.0, 360.0);
	s = constrain(s, 0.0, 1.0);
	v = constrain(v, 0.0, 1.0);

	int i, b, p, q, t;
	float f;

	h /= 60.0;  // sector 0 to 5
	i = floor(h);
	f = h - i;  // factorial part of h

	b = v * 255;
	p = v * (1 - s) * 255;
	q = v * (1 - s * f) * 255;
	t = v * (1 - s * (1 - f)) * 255;

	switch (i) {
	case 0:
		return Color(b, t, p);
	case 1:
		return Color(q, b, p);
	case 2:
		return Color(p, b, t);
	case 3:
		return Color(p, q, b);
	case 4:
		return Color(t, p, b);
	default:
		return Color(b, p, q);
	}


}
unsigned int Clight::_toPWM(unsigned int value, bool gamma, bool inverse) {
	//Serial.println("_toPWM in" + String(value));
	value = constrain(value, VALUE_MIN, VALUE_MAX);
	if (gamma) value = pgm_read_byte(_light_gamma_table + value);
	if (VALUE_MAX != PWM_LIMIT) value = _lightMap(value, VALUE_MIN, VALUE_MAX, PWM_MIN, PWM_LIMIT);
	if (inverse) value = PWM_LIMIT - value;
	//Serial.println("_toPWM out" + String(value));
	return value;
}

// Returns a PWM value for the given channel ID
unsigned int Clight::_toPWM(unsigned char id) {
	bool useGamma = _light_use_gamma && _light_has_color && (id < 3);
	return Clight::_toPWM(_light_channels[id].current, useGamma, _light_channels[id].inverse);
}
void Clight::_setValue(const unsigned char id, const unsigned int value) {
	if (_light_channels[id].value != value) {
		//Serial.println("set value" + String(value));
		_light_channels[id].value = value;
		//_light_dirty = true;
	}
}

void Clight::_lightApplyBrightnessColor() {

	double brightness = static_cast<double>((Brigthness) / 100.0);

	// Substract the common part from RGB channels and add it to white channel. So [250,150,50] -> [200,100,0,50]
	unsigned char white = std::min(_light_channels[0].inputValue, std::min(_light_channels[1].inputValue, _light_channels[2].inputValue));
	for (unsigned int i = 0; i < 3; i++) {
		_setValue(i, _light_channels[i].inputValue - white);
	}

	// Split the White Value across 2 White LED Strips.
	if (_light_use_cct) {

		// This change the range from 153-500 to 0-347 so we get a value between 0 and 1 in the end.
		double miredFactor = ((double)_light_mireds - (double)_light_cold_mireds) / ((double)_light_warm_mireds - (double)_light_cold_mireds);

		// set cold white
		_light_channels[3].inputValue = 0;
		_setValue(3, lround(((double) 1.0 - miredFactor) * white));

		// set warm white
		_light_channels[4].inputValue = 0;
		_setValue(4, lround(miredFactor * white));
	}
	else {
		_light_channels[3].inputValue = 0;
		_setValue(3, white);
	}

	// Scale up to equal input values. So [250,150,50] -> [200,100,0,50] -> [250, 125, 0, 63]
	unsigned char max_in = max(_light_channels[0].inputValue, max(_light_channels[1].inputValue, _light_channels[2].inputValue));
	unsigned char max_out = max(max(_light_channels[0].value, _light_channels[1].value), max(_light_channels[2].value, _light_channels[3].value));
	unsigned char channelSize = _light_use_cct ? 5 : 4;

	if (_light_use_cct) {
		max_out = max(max_out, _light_channels[4].value);
	}

	double factor = (max_out > 0) ? (double)(max_in / max_out) : 0;
	
	for (unsigned char i = 0; i < channelSize; i++) {
	//	Serial.println("ch " + String(i) + "val" + String(_light_channels[i].value) + " factor" + String(factor) + " brightness" + String(brightness));
		_setValue(i, lround((double)_light_channels[i].value * factor * brightness));
	}

	// Scale white channel to match brightness
	for (unsigned char i = 3; i < channelSize; i++) {
		_setValue(i, constrain(static_cast<unsigned int>(_light_channels[i].value * LIGHT_WHITE_FACTOR), BRIGHTNESS_MIN, BRIGHTNESS_MAX));
	}

	// For the rest of channels, don't apply brightness, it is already in the inputValue
	// i should be 4 when RGBW and 5 when RGBWW
	for (unsigned char i = channelSize; i < LIGHT_CHANNELS; i++) {
		_setValue(i, _light_channels[i].inputValue);
	}

}
long Clight::_toKelvin(const long mireds) {
	return constrain(static_cast<long>(1000000L / mireds), _light_warm_kelvin, _light_cold_kelvin);
}

long  Clight::_toMireds(const long kelvin) {
	return constrain(static_cast<long>(lround(1000000L / kelvin)), _light_cold_mireds, _light_warm_mireds);
}

void  Clight::_lightMireds(const long kelvin) {
	_light_mireds = _toMireds(kelvin);
}

void Clight::ColorToHSI(uint32_t rgbcolor, uint32_t brightness,  double &Hue, double &Saturation, double &Intensity)
{
  uint32_t r = REDVALUE(rgbcolor);
  uint32_t g = GREENVALUE(rgbcolor);
  uint32_t b = BLUEVALUE(rgbcolor);

  if ((r < 0 && g < 0 && b < 0) || (r > 255 || g > 255 || b > 255))
  {
    Hue = Saturation = Intensity = 0;
    return;
  }

  if (g == b)
  {
    if (b < 255)
    {
      b = b + 1;
    }
    else
    {
      b = b - 1;
    }
  }
  uint32_t nImax, nImin, nSum, nDifference;
  nImax = MAXHS(r, b);
  nImax = MAXHS(nImax, g);
  nImin = MINHS(r, b);
  nImin = MINHS(nImin, g);
  nSum = nImin + nImax;
  nDifference = nImax - nImin;

  Intensity = (float)nSum / 2;

  if (Intensity < 128)
  {
    Saturation = (255 * ((float)nDifference / nSum));
  }
  else
  {
    Saturation = (float)(255 * ((float)nDifference / (510 - nSum)));
  }

  if (Saturation != 0)
  {
    if (nImax == r)
    {
      Hue = (60 * ((float)g - (float)b) / nDifference);
    }
    else if (nImax == g)
    {
      Hue = (60 * ((float)b - (float)r) / nDifference + 120);
    }
    else if (nImax == b)
    {
      Hue = (60 * ((float)r - (float)g) / nDifference + 240);
    }

    if (Hue < 0)
    {
      Hue = (60 * ((float)b - (float)r) / nDifference + 120);
    }
  }
  else
  {
    Hue = -1;
  }

  return;
}
