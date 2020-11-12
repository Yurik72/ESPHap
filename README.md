[![Gitter](https://badges.gitter.im/Join%20Chat.svg)]

ESPHap - Arduino HomeKit ESP32/ESP8266
===========

ESPHap is an Arduino library which natively implements Apple's HomeKit protocol for your ESP32/ESP8266 based projects without the need for additional bridges. ESPHap currently supports ESP32 and ESP8266.

Please note: There are some minor known bugs within the ESP8266 implementation, especially during device pairing. Not all pairing attempts succeed, it can take up to 3 attempts to pair devices, but once paired, devices work well and stable.

ESPHap allows to implement up to 16 different [accessories](https://www.apple.com/ae/ios/home/accessories/) on the same ESP32/ESP8266 board.

Many thanks to [maximkulkin](https://github.com/maximkulkin) for providing some fine libraries for native integration and which are part of this project.

Many thanks to [Mixiaoxiao](https://github.com/Mixiaoxiao) for hinting on how to stop the watchdog on ESP8266 and also on how to implement low memory usage functions. 

# Build instructions

1. Prepare the Arduino IDE to work with ESP32 and/or ESP8266 (i.e. install the corresponding boards from within Arduino Board Manager).
2. Install this libary (EspHap) from the Arduino IDE library manager or clone the sources into an Arduino library folder named "ESPHap".
3. Extract the content of [wolfSSL_3_13_0.rar](https://github.com/Yurik72/ESPHap/blob/master/wolfssl/wolfSSL_3_13_0.rar) to a Arduino library folder named "wolfssl".
4. Open any sketch from the provided examples and compile. Detailed instructions on how to upload and pair with Apple Home app can be found in the [Sonoff example](https://github.com/Yurik72/ESPHap/wiki/Build-Sonoff-Basic) or on [instructables.com](https://www.instructables.com/id/Arduino-With-ESP32-and-Native-Apple-HomeKit-Integr/)

# Installing and configuring wolfSSL

This project makes use of [wolfSSL](https://github.com/wolfSSL), an embedded, lightweight SSL library written in ANSI C and targeted for embedded systems like RTOS. wolfSSL needs to be slightly patched and this section describes some technical details for users who had already installed wolfSSL. 

## For ESP32

ESPHap for ESP32 works well with wolfSSL versions 4.2.0 and 3.13.0. Full instruction on how to configure wolfSSL can be found [here](https://www.wolfssl.com/doxygen/md__Users_alexabrahamson_Work_wolfssl-CLEAN_IDE_ARDUINO_README.html)

To simplify installation, ESPHap comes with pre-configured versions of wolfSSL which can be used out of the box. Extract either [wolfSSL_3_13_0.rar](https://github.com/Yurik72/ESPHap/blob/master/wolfssl/wolfSSL_3_13_0.rar) or [wolfssl.rar](https://github.com/Yurik72/ESPHap/blob/master/wolfssl/wolfSSL.rar) into an Arduino library folder named "wolfssl".

If you want to configure wolfSSL yourself, please use settings.h and user_settings.h from the provided preconfigured libraries as reference, you will need to edit these files according to your needs.

## For ESP8266

ESPHap for ESP8266 only works with version 3.13.0.

To simplify installation, ESPHap comes with a pre-configured and slightly patched versions of wolfSSL 3.13.0 which can be used out of the box. Extract [wolfSSL_3_13_0.rar](https://github.com/Yurik72/ESPHap/blob/master/wolfssl/wolfSSL_3_13_0.rar) into an Arduino library folder named "wolfssl". ESPHap ONLY works with this patched wolfSSL version.

### ESPHap library was tested in the following environment:

- ESP32 board library version 1.0.4
- ESP8266 board library version 2.6.3
- Arduino version 1.8.12

# Getting help
## Simple example

Examples folder contains very simple examples EspHapLed (for ESP32) and EspHapLed8266 (for ESP8266) how to handle LED status

Before actually compiling your sketch you first need to:

1. Change your Wifi settings by editing the following lines:

```c
const char* ssid     = "WLAN SSID";
const char* password = "WLAN password";
```
2. Change your GPIO data pin for the connected LED

```c
const int led_gpio = 4;
```

This example does not generate a QR code for pairing, so please use manual pairing by entering password 111 11 111.

More detail instruction can be found [here](https://www.instructables.com/id/Arduino-With-ESP32-and-Native-Apple-HomeKit-Integr/)

Those examples can be used to handle other device like relays, which supports two statuses On/Off.

## Sonoff example

Example folders contains 3 sketches for the Sonoff devices. 

- [Sonoff_basic](https://github.com/Yurik72/ESPHap/tree/master/examples/Sonoff_basic) simple sonoff basic device 
- [Sonoff_basic Web](https://github.com/Yurik72/ESPHap/tree/master/examples/Sonoff_basic_web) simple sonoff basic device with built in web portal and file manager 
- [Sonoff_B1](https://github.com/Yurik72/ESPHap/tree/master/examples/Sonoff_B1_web) sonoff B1 lamp , wiki [here](https://www.instructables.com/id/Sonoff-B1-With-Native-Apple-Home-Kit-Make-by-Ardui/)

As mentioned ESP8266 in the process of testing (Sonoff is ESP8265)
But example already works quite fine. Small problem detected during the pairing. But after that works well

Please have a look [instructions](https://github.com/Yurik72/ESPHap/wiki/Build-Sonoff-Basic)

## Advanced LED example (simple switch and dimmable)

Example folders contains sketch for [Advanced LED](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHapAdvancedLed)
 and [Advanced dimmable LED](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHapAdvancedDimmableLed)
This is sketch compatible with both ESP32 & ESP8266, handles LED Switching On/Off, LED brightness and contains advanced features:

- Built-in web site
- Built in web file manager
- OTA
- Setup/pairing via QR Code (to access QR code you need enter http://\<IP ADDRESS\>/setup.html)

This is basic demonstration of powerfull IOT device, which contains such features

## Thermostat example

Example folders contains sketch for [Thermostat DHT](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHap_DHT11), 
[Universal Thermostat](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHap_Thermostat) which shows on Apple Home two icons Temperature and Humidity. There is universal sketch applicable for ESP32 and ESP8266 and using DHT sensor, BME28 or Dallas, as well any other hardware can be easily implemented
Hovewer any sensor can be used with simple code changes...
Those sketch as well includes advanced features: Web File Manager, OTA, Simple web site. For the [Universal Thermostat](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHap_Thermostat) it's example how to send historical data to [thingSpeak](https://thingspeak.com/)

Build instruction the same as for sketches above.

## Switch example

Example folders contains sketch for [Switch](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHap_Switch), which shows on Apple simple Switch icon. There is universal sketch applicable for ESP32 and ESP8266 and any relay can be used to manage external devices.

Those sketch as well includes advanced features: Web File Manager, OTA, Simple web site.

Build instruction the same as for sketches above.

## RGB Strip (WS2812) & Motion example

Example folders contains sketch for [RGB & Motion devices](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHap_RGB_Motion), which shows on Apple Home two icons: RGB Bulb and Motion Sensor. There is universal sketch applicable for ESP32 and ESP8266 and using WS2812 LED strip.
Those sketch as well includes advanced features: Web File Manager, OTA, Simple web site and allows to demonstrate two direction for the Apple HomeKit. Means when RGB (Brightness , Color ,State ) is changed from the built-in web site, Apple HomeKit refreshes the RGB bulb state.
For the motion sensor can be used anyone, skecth simples reads HIGH value for the predefined GPIO. For instance HC-SR501 can be used.
As well for RGB can be used any strip, the question is how to proceed with Brightness and Color values received from the Apple Home app.

Those combination potentially has good demonstration for Apple Home Automation, based on the Motion state RGB can be switched On/Off...

Build instruction the same as for sketches above.

## Button example

Example folders contains sketch for [Button](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHap_Button), which shows on Apple simple Button icon. There is universal sketch applicable for ESP32 and ESP8266 (Testing in progress on ESP8266) and any Button scenarios on Apple can be used to manage automation.

Those sketch as well includes advanced features: Web File Manager, OTA, Simple web site.

Build instruction the same as for sketches above.

## Air Quality sensor example

Example folders contains sketch for [ Air Quality sensor](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHap_AirQuality_MQ135), which shows on Apple Air quality Sensor icon.
Sketch is designed with usage for MQ135 sensor. There is universal sketch applicable for ESP32 and ESP8266. Main advantage that build-in web site shows history trends of PPM level of dioxide. 
Code is contains comments and one of the important thing for this sketch is calibration based on your real sensor, for this purpose following line should be changed

```c
const float factor=14.0; //to be calibrated with your MQ135
```

Those sketch as well includes advanced features: 
- Web File Manager
- OTA
- Built-in web site
- Sending data to ThingSpeak

Build instruction the same as for sketches above.

## FAN servicer example

Example folders contains sketch for [FAN](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHap_FAN), which shows on Apple FAN icon. You are able to control FAN characteristic such as 

- On/Off
- Speed
- Direction

Those sketch as well includes advanced features: 
- Web File Manager
- OTA
- Built-in web site

Build instruction the same as for sketches above.

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
Initialize file storage to keep pairing information (you can put any file name as you want)
```c
 init_hap_storage("/pair.dat");
```
Hovewer you can hahdle your function how to keep your pairing information examples EspHapLed (for ESP32) and EspHapLed8266 (for ESP8266) contains code to show custom implementation


Set base accessory type, means you will have at least one accessory and you need define a type
```c
 hap_setbase_accessorytype(homekit_accessory_category_lightbulb);
```
full list of availbale accessories you can find in the [types.h](https://github.com/Yurik72/ESPHap/blob/master/types.h) , see enum homekit_accessory_category_t

Set base information HostName, Manufacture, Serial number, Model,Firmware version , like this
```c
  hap_initbase_accessory_service(HOSTNAME,"Yurik72","0","EspHapLed","1.0");
```

Than you need a setup all accessories and their services and characteristic. Do not forgot that you already have one base accessory, therefore first we need a setup it. For instance for the lighBulb
```c
  hapservice= hap_add_lightbulb_service("LED",led_callback,(void*)&led_gpio);
```
"LED" is the name of accessory 
 
led_callback is callback function called from the Apple Home app when changes 
 
 (void*)&led_gpio is callback parameter
 
 After that we can add more accessories like this
 ```c
 hapservice_motion= hap_add_motion_service_as_accessory(homekit_accessory_category_security_system,"Motion",motion_callback,NULL);
```

Full list of services and their characteristic can be found in the [characteristic.h](https://github.com/Yurik72/ESPHap/blob/master/characteristics.h). Header is well documented and descibes service types and their characteristics).
The list of API to add services and accessories can be found here [homeintegration.h](https://github.com/Yurik72/ESPHap/blob/master/homeintegration.h). It's quite transparent based on the function names.

When accessories, services and characteristic is defined we need to finally call
```c
hap_init_homekit_server();
```

That is all for setup.

3. Implement callback and notify function

Every callback has the same signature and parameters

 - characteristic
 - value 
 - context (callback parameters)
 
 This function is called when accessories state is changed from the Apple. You can manage your devices there, based on the value.
 Please check which type (bool, int, float ) must be used for different characteristic
```c
void led_callback(homekit_characteristic_t *ch, homekit_value_t value, void *context) {
    Serial.println("led_callback");
    digitalWrite(led_gpio, value.bool_value?HIGH:LOW);
}
```
optionally implement notify function, which is neccessary to inform Apple about device state changes. This is must for accessories like termostat, for instance for the LightBulb we can notify about power state On/Off, which is bool value true/false
```c
void notify_hap(){
homekit_characteristic_t * ch= homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_ON);
 HAP_NOTIFY_CHANGES(bool, ch, <new bool value>, 0)
 }
 ```
 To get characteristic, API function homekit_service_characteristic_by_type should be used. 
 First parameter is pointer to the hapservice (from the setup), second is characteristic type
 
 4. Loop function
 In the loop we have to add only one lines and only for ESP8266
 
```c
 #ifdef ESP8266
  hap_homekit_loop();
#endif
``` 

Advanced features

- Built in web server

Since version 1.0.2, library contains submodule for built in web server. To use it you need

Include header
```c
#include <hapweb/hap_webserver.hpp>
``` 
call 
```c
set_indexhml(FPSTR(INDEX_HTML)); // optional if you want to have your own root page
hap_webserver_begin();
``` 
in the setup function

set_indexhml(FPSTR(INDEX_HTML)); allows to define your root page content, see example [Advanced Led](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHapAdvancedLed)

- Setup by QR Code

If you use built in web server, by default it provides access to setup/pairing page/image by QR code, you just need enter http://\<IP address\>/setup.html, see example [Advanced LED](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHapAdvancedLed)

# Versions history

 ## v1.0
 
 - First success version works both with ESP32 and ESP8266
 
 ## v1.0.1
 
 - Minor bug fixes and more examples
 
 ## v1.0.2
 
 - implement submodule for file storage of pairing data [hapfilestorage.hpp](https://github.com/Yurik72/ESPHap/blob/master/hapfilestorage/hapfilestorage.hpp) which allows to reuse basic function for store pairing data on SPIFFs file system. 
 - implement submodules for internal web server [hapweb](https://github.com/Yurik72/ESPHap/tree/master/hapweb). Now Web server can be easily setup and handle file browser and your own portal for device. Plus handling of OTA.
- implement submodules for pairing by QR code [hapqr.hpp](https://github.com/Yurik72/ESPHap/blob/master/qr/hapqr.hpp).Together with web server you can got on your browser QR image, which can be easily scan for pairing purpose. To access QR code you need enter http://\<IPADDRESS\>/setup.html.
 
 ## v1.0.3
 
- small bug fixes
- new examples
 
## Support this project

You can easilly do that by donating

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=JVZWJ6FSMURSL&currency_code=USD&source=url)



//*TODO* - 
