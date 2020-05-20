[![Gitter](https://badges.gitter.im/Join%20Chat.svg)]




ESPHap
===========

This is a library for easily & efficiently integrating arduino projects based on ESP32/ESP8266 with Apple Home Kit by  native protocols 
and without any additional bridges

At this moment  ESP32 supported and tested well.
ESP8266 is ported as well and under the testing stage. There are some minor bugs and especially in the pairing process. Not all of them finished smooth. Hovewer it can be maximum 3 attempts (in my experience) and after that everything working fine. I'd say that after pairing (once operation) devices works well and stable

Library allows to setup up to the 16 different [accessories](https://www.apple.com/ae/ios/home/accessories/)  on the same ESP32/ESP8266 board

Many thanks to [maximkulkin](https://github.com/maximkulkin) for providing fine libraries for native integration,
this project uses this as well.

Many thanks to [Mixiaoxiao](https://github.com/Mixiaoxiao)  to give a hint for ESP8266 to stop watchdog and some functions with lower memory usage

# Short Build instruction

1. Prepare Arduino enviropment  to works with ESP32 and/or ESP8266.

2. Install this libary (EspHap) from the library manager or clone sources to the Arduino library folder named EspHap.

3. Extract the content of the [wolfSSL_3_13_0.rar](https://github.com/Yurik72/ESPHap/blob/master/wolfssl/wolfSSL_3_13_0.rar)  to the Arduino Libraries folder named "wolfssl"

4. Open any sketch from the examples and compile it. Detailed instruction how to upload and pair with Apple can used from the  [Sonoff example](https://github.com/Yurik72/ESPHap/wiki/Build-Sonoff-Basic) or [there](https://www.instructables.com/id/Arduino-With-ESP32-and-Native-Apple-HomeKit-Integr/)

#  wolfssl manual preparation

This section is describes more technical details for professional users if they have already wolfssl installed.
[wolfssl](https://github.com/wolfSSL)    requires some preparation before usage...


## For ESP32

ESP32 works well with  wolfssl versions 4.2.0 and 3.13.0
Full instruction how to prepare, can be found [there](https://www.wolfssl.com/doxygen/md__Users_alexabrahamson_Work_wolfssl-CLEAN_IDE_ARDUINO_README.html)

For simplify a process you can use  [wolfSSL_3_13_0.rar](https://github.com/Yurik72/ESPHap/blob/master/wolfssl/wolfSSL_3_13_0.rar)  or [wolfssl.rar](https://github.com/Yurik72/ESPHap/blob/master/wolfssl/wolfSSL.rar) archive , which already prepared. You just need to extract this content  into Arduino Libraries folder named "wolfssl"

If you are going to prepare this manually, please reuse/check settings.h and user_settings.h  from the attached archives
 You need copy/replace this files in wolfssl components.

## For ESP8266

ESP8266 works only with version 3.13.0.

For preparation you should extract a content of [wolfSSL_3_13_0.rar](https://github.com/Yurik72/ESPHap/blob/master/wolfssl/wolfSSL_3_13_0.rar) to the Arduino libraries folder named "wolfssl" .  Those version of wolfssl is slightly changed to work with ESP8266 and ONLY this sources must be used.



### ESPHAP library was tested in the following enviropment:

ESP32 board library  version 1.0.4

ESP8266 board library  version 2.6.3

Arduino version 1.8.12

# Getting help


## Simple example

Examples folder contains very simple examples EspHapLed (for ESP32) and EspHapLed8266 (for ESP8266) how to handle Led status

Before compile you need:

1.  Change your Wifi setting by the editing following lines:

```c
const char* ssid     = "your ssid";
const char* password = "pwd to ssid";
```
2. Change your GPIO, where Led is connected

```c
const int led_gpio = 4;
```
.
At this moment QR is not generated, so please use manual pairing by enetering password  11111111

More detail instruction can be found [there](https://www.instructables.com/id/Arduino-With-ESP32-and-Native-Apple-HomeKit-Integr/)

Those examples can be used to handle other device like relays, which supports two statuses ON/OFF.




## Sonoff example

Example folders contains 3 sketches for the Sonoff devices. 

- [Sonoff_basic](https://github.com/Yurik72/ESPHap/tree/master/examples/Sonoff_basic) simple sonoff basic device 

- [Sonoff_basic Web](https://github.com/Yurik72/ESPHap/tree/master/examples/Sonoff_basic_web) simple sonoff basic device  with built in web portal and file manager 

- [Sonoff_B1](https://github.com/Yurik72/ESPHap/tree/master/examples/Sonoff_B1_web)  sonoff B1 lamp 

As mentioned ESP8266 in the process of testing (Sonoff is esp8265 )
But example already works quite fine. Small problem detected during the pairing. But after that works well

Please have a look [instructions](https://github.com/Yurik72/ESPHap/wiki/Build-Sonoff-Basic)

## Thermostat example

Example folders contains sketch for [Thermostat DHT](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHap_DHT11), 
[Universal Thermostat](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHap_Thermostat) which shows on Apple Home two icons Temperathure and Humidity. There is universal sketch applicable for ESP32 and ESP8266 and using DHT sensor, BME28 or Dallas, 
as well any other hardware can be easily implemented
Hovewer any sensor can be used with simple code changes...
Those sketch as well includes advanced feathures: Web File Manager, OTA, Simple web site. For the [Universal Thermostat](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHap_Thermostat) it's example how to send historical data to [thingSpeak](https://thingspeak.com/)

Build instruction the same as for sketches avove.

## Switch example

Example folders contains sketch for [Switch](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHap_Switch), which shows on Apple simple Switch icon. There is universal sketch applicable for ESP32 and ESP8266 and any relay can be used to manage external devices.

Those sketch as well includes advanced feathures: Web File Manager, OTA, Simple web site.

Build instruction the same as for sketches above.

## RGB Strip (WS2812) & Motion example

Example folders contains sketch for [RGB & Motion devices](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHap_RGB_Motion), which shows on Apple Home two icons: RGB Bulb  and Motion Sensor. There is universal sketch applicable for ESP32 and ESP8266 and using WS2812 Led strip.
Those sketch as well includes advanced feathures: Web File Manager, OTA, Simple web site and allows to demonstrate  two direction for the Apple Home Kit. Means when RGB (Brightness , Color ,State ) is changed from the built-in web site, Apple Home Kit refreshes the RGB bulb state.
For the motion sensor can be used anyone, skecth simples reads HIGH value for the predefined GPIO. For instance HC-SR501 can be used.
As well for RGB can be used any strip, the question is how to proceed with Brightness and Color values received from the Apple.

Those combination potentially has good demonstration for Apple Home Automation, based on the Motion state RGB can be switched ON/OFF...

Build instruction the same as for sketches avove.

## Button example

Example folders contains sketch for [Button](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHap_Button), which shows on Apple simple Button icon. There is universal sketch applicable for ESP32 and ESP8266 (Testing in progress on ESP8266) and any Button scenarios on Appllle  can be used to manage automation.

Those sketch as well includes advanced feathures: Web File Manager, OTA, Simple web site.

Build instruction the same as for sketches avove.

## Air Quality sensor example

Example folders contains sketch for [ Air Quality sensor](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHap_AirQuality_MQ135), which shows on Apple Air quality Sensor icon.
Sketch is designed with usage for MQ135 sensor. There is universal sketch applicable for ESP32 and ESP8266. Main advantage that build-in web site shows history trends of PPM level of dioxide.  
Code is contains comments and one of the important thing for this sketch is calibration based on your real sensor, for this purpose following line should be changed 

const float factor=14.0;//to be calibrated with your MQ135


Those sketch as well includes advanced feathures: 
-  Web File Manager
- OTA
- Built-in web site
- Sending data to ThingSpeak

Build instruction the same as for sketches avove.


# Library API and how to build your own sketch 

1. Prepare include section
```c

#ifdef ESP32
#include <SPIFFS.h>
#endif
#ifdef ESP8266
#include <ESP8266WiFi.h>

#include <ESP8266mDNS.h>
#include "coredecls.h"
#endif
extern "C"{
#include "homeintegration.h"
}
#ifdef ESP8266
#include "homekitintegrationcpp.h"
#endif
#include <hapfilestorage\hapfilestorage.hpp>
```


2. In the setup you have to do following, instead of other 

Small adujstment for the ESP8266
```c
 #ifdef ESP8266 
  disable_extra4k_at_link_time();
 #endif 
```
Initialize file storage to keep pairing information (you can put any file name as you want
```c
 init_hap_storage("/pair.dat");
```

Set base accessory type, means you will have at least one accessory and you need define a type
```c
 hap_setbase_accessorytype(homekit_accessory_category_lightbulb);
```
full list of availbale accessories you can find in the [types.h](https://github.com/Yurik72/ESPHap/blob/master/types.h) , see enum homekit_accessory_category_t

Set base information HostName, Manufacture, Serial number, Model,Firmware version , like this
```c
  hap_initbase_accessory_service(HOSTNAME,"Yurik72","0","EspHapLed","1.0");
```
  
Than you need a setup all accessories and their services and  characteristic. Do not forgot that you already have one base accessory,
therefore first we need a setup it. For instance for the lighBulb
```c
  hapservice= hap_add_lightbulb_service("Led",led_callback,(void*)&led_gpio);
```
 "Led" is the name of accessory 
 
 led_callback is callback function called from the apple when changes 
 
 (void*)&led_gpio  is callback parameter
 
 After that we can add more accessories like this
 ```c
 hapservice_motion= hap_add_motion_service_as_accessory(homekit_accessory_category_security_system,"Motion",motion_callback,NULL);
```

Full list of services and their characteristic can be found in the [characteristic.h](https://github.com/Yurik72/ESPHap/blob/master/characteristics.h) . Header is well documented and descibes services types and their characteristic)
The list of Api to add services and accessories can be found there [homeintegration.h](https://github.com/Yurik72/ESPHap/blob/master/homeintegration.h). It's quite transparent based on the function names

When accessories, services and characteristic is defined  we need to finally call
  ```c
hap_init_homekit_server();
```

That is all for setup

3. Implement callback and notify function

Every callback has the same signature and parameters

 - characteristic
 
 - value 
 
 - context (callback parameters)
 
 This function is called when accessories state is changed from the Apple. You  can manage your devices there, based on the value.
 Please check which type (bool, int, float ) must be used for different characteristic
  ```c
void led_callback(homekit_characteristic_t *ch, homekit_value_t value, void *context) {
    Serial.println("led_callback");
    digitalWrite(led_gpio, value.bool_value?HIGH:LOW);
}
```
optionally implement notify function, which is neccessary to inform Apple about device state changes . This is must for accessories like termostat , for instance for the LightBulb we can notify about power state On/Off , which is bool value true/false
  ```c
void notify_hap(){
homekit_characteristic_t * ch= homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_ON);
 HAP_NOTIFY_CHANGES(bool, ch, <new bool value>, 0)
 }
 ```
 To get characteristic , API function homekit_service_characteristic_by_type should be used. 
 First parameter is pointer to the hapservice (from the setup), second is characteristic type
 
 4. Loop function
 In the loop we have to add only one lines and only for ESP8266
 
   ```c
 #ifdef ESP8266
  hap_homekit_loop();
#endif
``` 
 
 
 
## Are you interesting to support this project ?

You can easilly do that by donations

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=JVZWJ6FSMURSL&currency_code=USD&source=url)






*TODO* - 
