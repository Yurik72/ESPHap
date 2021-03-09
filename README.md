![Gitter](https://badges.gitter.im/Join%20Chat.svg)

ESPHap - Arduino HomeKit ESP32/ESP8266
===========

**!!!Hint ESP32 library version 1.0.5 recently released is not supported now, due that some components became a part of core and cause linking problems. Until properly adaption 1.0.4 should be used**


ESPHap is an Arduino library which natively implements Apple's HomeKit protocol for your ESP32/ESP8266 based projects without the need for additional bridges. ESPHap currently supports both ESP32 and ESP8266.

Please note: There are some minor known bugs within the ESP8266 implementation, especially during device pairing. Not all pairing attempts succeed, it can take up to 3 attempts to pair devices. Once paired, devices work well and stable though. 
! Since version v1.0.7and usage IOS >14 the pairing process is stable for ESP8266, therefore known bugs are fixed and library can be easilly used for ESP8266

ESPHap allows to implement up to 16 different [accessories](https://www.apple.com/ae/ios/home/accessories/) on the same ESP32/ESP8266 board.

Many thanks to [maximkulkin](https://github.com/maximkulkin) for providing some fine libraries for native integration and which are part of this project.

Many thanks to [Mixiaoxiao](https://github.com/Mixiaoxiao) for hinting on how to stop the watchdog on ESP8266 and also on how to implement low memory usage functions. 

# Build instructions

1. Prepare the Arduino IDE to work with ESP32 and/or ESP8266 (i.e. install the corresponding boards from within Arduino Board Manager).
2. Install this libary (EspHap) from the Arduino IDE library manager or clone the sources into an Arduino library folder named "ESPHap".
3. Extract the content of [wolfSSL_3_13_0.rar](https://github.com/Yurik72/ESPHap/blob/master/wolfssl/wolfSSL_3_13_0.rar) to a Arduino library folder named "wolfssl".
4. Open any sketch from the provided examples and compile. Detailed instructions on how to upload and pair with Apple Home app can be found in the [Sonoff example](https://github.com/Yurik72/ESPHap/wiki/Build-Sonoff-Basic) or on [instructables.com](https://www.instructables.com/id/Arduino-With-ESP32-and-Native-Apple-HomeKit-Integr/).

# Installing and configuring wolfSSL

This project makes use of [wolfSSL](https://github.com/wolfSSL), an embedded, lightweight SSL library written in ANSI C and targeted for embedded systems like RTOS. wolfSSL needs to be slightly patched and this section describes some technical details for users who had already installed wolfSSL. 

## For ESP32

ESPHap for ESP32 works well with wolfSSL versions 4.2.0 and 3.13.0. Full instruction on how to configure wolfSSL can be found [here](https://www.wolfssl.com/doxygen/md__Users_alexabrahamson_Work_wolfssl-CLEAN_IDE_ARDUINO_README.html).

To simplify installation, ESPHap comes with pre-configured versions of wolfSSL which can be used out of the box. Extract either [wolfSSL_3_13_0.rar](https://github.com/Yurik72/ESPHap/blob/master/wolfssl/wolfSSL_3_13_0.rar) or [wolfssl.rar](https://github.com/Yurik72/ESPHap/blob/master/wolfssl/wolfSSL.rar) into an Arduino library folder named "wolfssl".

If you want to configure wolfSSL yourself, please use settings.h and user_settings.h from the provided preconfigured libraries as reference, you will need to edit these files according to your needs.

## For ESP8266

ESPHap for ESP8266 only works with wolfSSL 3.13.0.

To simplify installation, ESPHap comes with a pre-configured and slightly patched versions of wolfSSL 3.13.0 which can be used out of the box. Extract [wolfSSL_3_13_0.rar](https://github.com/Yurik72/ESPHap/blob/master/wolfssl/wolfSSL_3_13_0.rar) into an Arduino library folder named "wolfssl". **ESPHap ONLY works with the provided patched wolfSSL version.**

### ESPHap was tested in the following environments:

- ESP32 board library version 1.0.4
- ESP8266 board library version 2.6.3
- Arduino version 1.8.12 and above

### About the included webserver
Some of the more advanced code examples below come with an integrated webserver which features a SPIFFS (Serial Peripheral Interface Flash File System) backend. The webserver is often used to display sensor data, control devices, configure device WiFi settings or to store data like Homekit binding credentials or historical sensor data. Once you've connected your device to your WLAN, it will be available under ``http://IPADDRESS/``. Your IP address is displayed within Arduino's serial monitor window during the boot sequence, so the IP is 172.16.0.169 in this case:

![terminal](docs/log.png)
Also, the list of available website pages for your device is displayed, so ``http://IPADDRESS/``, ``http://IPADDRESS/browse`` and ``http://IPADDRESS/update`` in the above case.

### Default, index or root page ``http://IPADDRESS/``

Your device's root page located under ``http://IPADDRESS/`` is usually configured to display sensor data, like temperature and humidity in the [Thermostat DHT](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHap_DHT11) example: 

![index](docs/index.png)

### The file browser ``http://IPADDRESS/browse``
The file browser enables you to access the files stored on your device. More importantly, you are able to upload or delete files to or from your device. 
For instance, if you want to delete the Homekit pairing data stored in ``pair.dat``, you may simply do so by clicking the correspondin delete button.
After any file operation you need to restart your device by clicking on "Press to restart" button.
![filemanager](docs/browse.png)

# Code examples
## Simple LED example

The sketches EspHapLed (for ESP32) and EspHapLed8266 (for ESP8266) are very basic examples for how to handle (switching on/off) a simple LED connected to your board.

Before you compile your sketch you first need to:

1. Change your Wifi settings by editing these lines:
```c
const char* ssid     = "WLAN SSID";
const char* password = "WLAN password";
```
2. Connect a LED to your board and change GPIO data pin
```c
const int led_gpio = 4;
```
This example does not implement any of the more sophisticated pairing options (like QR code), instead please choose manual pairing within your Apple Home app using the password 111 11 111. More detailed instructions on how to pair HomeKit devices with you Home app can be found [here](https://www.instructables.com/id/Arduino-With-ESP32-and-Native-Apple-HomeKit-Integr/).

These examples can be used to handle other device like relays which implement two statuses on/off.

## Sonoff examples

The example folder contains 3 sketches for Sonoff devices: 
- [Sonoff_basic](https://github.com/Yurik72/ESPHap/tree/master/examples/Sonoff_basic) Sonoff basic device 
- [Sonoff_basic Web](https://github.com/Yurik72/ESPHap/tree/master/examples/Sonoff_basic_web) Sonoff basic device with built-in web portal and file manager 
- [Sonoff_B1](https://github.com/Yurik72/ESPHap/tree/master/examples/Sonoff_B1_web) Sonoff B1 lamp, see wiki [here](https://www.instructables.com/id/Sonoff-B1-With-Native-Apple-Home-Kit-Make-by-Ardui/)

Sonoff is based on ESP8265 and the examples already work quite well. But still there some known problems especially during pairing phase. For more informations, please have a look at the [extended instructions](https://github.com/Yurik72/ESPHap/wiki/Build-Sonoff-Basic).

## Sonoff RF plus Sonoff PIR2 example

The example folder also contains a more advanced sketch for [EspHapSonoff_RFBridge](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHapSonoff_RFBridge).

This is an alternative firmware for the Sonoff RF switch, which provides two accessories: switch and motion detection. The main idea of this sketch is to re-use the Sonoff RF module to accept signals from Sonoff PIR2 motion detection module and notify Apple Home app. This gives you the opportunity to define new scenarios within the Apple Home app (i.e. to define what should happen once a motion is detected). 
The Sonof RF has a RF module assigned to button in the original firmware, with this sketch you will loose this functionality. The sketch now interprets and treats a button press (directly or from RF signal) as a motion detection.

## Advanced LED example (simple switch and dimmable)

Example folders contains sketch for [Advanced LED](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHapAdvancedLed)
 and [Advanced dimmable LED](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHapAdvancedDimmableLed)
This is sketch compatible with both ESP32 & ESP8266, handles LED Switching on/off, LED brightness and contains advanced features:

- Built-in web site
- Built-in web file manager
- OTA
- Setup/pairing via QR Code (to access QR code you need enter http://\<IP ADDRESS\>/setup.html)

This is a basic demonstration of an advanced IOT device, which features such functionalities.

## Thermostat example

The example folder contains the two sketches [Thermostat DHT](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHap_DHT11) and  
[Universal Thermostat](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHap_Thermostat) which will implement temperature and humidity sensing within the Apple Home app. The sketches work both for ESP32 and ESP8266 and use Adafruit's DHT and BME280 sensor libraries which will work for many common DHT sensors. The sketches can easily be adjusted to work with other sensors.

The thermostat sketches also implement some more advanced features: 
- Web File Manager
- OTA
- Simple web site

The [Universal Thermostat sketch](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHap_Thermostat) also able to send historical data to [thingSpeak](https://thingspeak.com/).

## Switch example

The example folder contains the sketch [Switch](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHap_Switch), which implements a simple switch within the Apple Home app. The sketch works both for ESP32 and ESP8266 and could for example be used to control a relay.

This sketch also implements some advanced features: 
- Web File Manager
- OTA
- Simple web site

Build instructions are the same as for sketches above.

## RGB Strip (WS2812) & motion sensor example

The example folder contains the sketch [RGB & Motion device](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHap_RGB_Motion), which implements a RGB bulb and a motion sensor within the Apple Home app. The sketch works for ESP32 and ESP8266 and is used to control a WS2812 LED strip via an attached motion sensor (HC-SR501 in this example).

This sketch also implements some advanced features: 
- Web File Manager
- OTA
- Simple web site

The web site allows to demonstrate the bidirectional capabilities of Apple HomeKit: whenever color, brightness or state of the RGB strip is changed from the built-in web site, Apple HomeKit refreshes the corresponding values or state within the Apple Home app.

Any motion sensor can be used, the sketch simply checks for changes of the HIGH value of the predefined GPIO pin.

The sketch drives a WS2812 RGB strip but other types should work in a similier way, but you would have to figure out how to process color and brightness values received from the Home app or the built-in web site with your specific RGB strip.

The combination of a motion sensor along with a RGB strip (or a switch) is suited to demonstrate Apple Home Automation capabilities; based on the motion state, the RGB strip can be configured to be switched on or off.

Build instructions are the same as for the sketches above.

## Button example

The example folder contains the sketch [Button](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHap_Button), which implements a button within the Apple Home app. This is a universal sketch applicable for ESP32 and ESP8266 (still in beta) and can be used to create Apple Home Automation scenarios within your Apple Home app.

This sketch also implements some advanced features: 
- Web File Manager
- OTA
- Simple web site

Build instructions are the same as for the sketches above.

## Air Quality Sensor example

The example folder contains the sketch [Air Quality sensor](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHap_AirQuality_MQ135), which implements an Air Quality Sensor within Apple Home app.
This sketch uses a MQ135 sensor and works both for ESP32 and ESP8266. The built-in web site displays historic data of the CO2 concentration. 
You need to enter the appropriate calibrattion data for your sensor, for this purpose the following line should be changed

```c
const float factor=14.0; //to be calibrated with your MQ135
```

This sketch also implements some advanced features: 
- Web File Manager
- OTA
- Simple web site
- Sending data to ThingSpeak

Build instructiona are the same as for the sketches above.

## Fan example

The example folder contains the sketch  [FAN](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHap_FAN), which implements a fan within Apple Home app. You are able to control fan characteristics such as 

- On/Off
- Speed
- Direction

This sketch also implements some advanced features: 
- Web File Manager
- OTA
- Simple web site

Build instructions are the same as for the sketches above.

## ESP32 Camera with motion detection

This is one of the experimental scenario for ESP32 camera module. Sketch based on well known [example] (https://robotzero.one/esp32-face-door-entry/) with additional feathures.
First of all it can works as simple motion detector and notify Apple that motion occurs.

Additionally :
 - it keeps functionality for face recognition and internal web site for camera setting and live video streaming.
 
 - Allow to handle servo (if that attached) for camera position
 
 - If face recognized it's able to send RF signal. So any receiver can validate signal and apply any action for instance open a door lock
 
 - Possibility to send e-mail with capture, when motion detected
 
 
 All of that is experimental. But main achievemnts is to find a way how to detect motion, due to the lot of bug's and limitation in  camera API's.
 I found a fastest way (in my opinion). Main algorith is traditional: comparing two camera snapshots and based on the sensitivity level
 and pixels changed detect motion.
 This looks abnormally but fastest way is:
  - Take camera snapshot in JPEG format. Fast because it's related to camera communication 
  - Convert JPEG to raw data (RGB565) 
  - Compare to raw data snapshot
  
  To achieve this, PSRAM memory is used and it's enought for two snapshots less than 1280x1024.
  
 *Additionally, due to specific SPIFFS usage this is a good example how to keep pairing data in the EEPROM



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

#include <hapfilestorage/hapfilestorage.hpp>
```

2. If you are using ESP8266, add the following to the setup section 
```c
 #ifdef ESP8266 
  disable_extra4k_at_link_time();
 #endif 
```
Initialize the file storage to store pairing information (any filename will do)
```c
 init_hap_storage("/pair.dat");
```
It is up to you how to store device pairing information, the sketches EspHapLed (for ESP32) and EspHapLed8266 (for ESP8266) contain code examples to demonstrate custom implementations.

Set at least one base accessory type:
```c
 hap_setbase_accessorytype(homekit_accessory_category_lightbulb);
```
The full list of availbale device types can be found in the header file [types.h](https://github.com/Yurik72/ESPHap/blob/master/types.h) (section enum homekit_accessory_category_t).

Set the base information for your device like hostname, manufacturer, serial number, model, firmware version:
```c
  hap_initbase_accessory_service(HOSTNAME,"Yurik72","0","EspHapLed","1.0");
```

Then you need to define all accessories, the services they provide and the characteristics of these services. Do not forgot that you already have one base accessory, therefore you need to define a setup for this one, too. For example for a lightbulb you would need
```c
  hapservice= hap_add_lightbulb_service("LED",led_callback,(void*)&led_gpio);
```
- "LED" is the name of accessory 
- `led_callback` is the name of the callback function which is called from  Apple Home app whenever a change occurs
- `(void*)&led_gpio` is the callback parameter
 
Optinally you can set a default (initial) value to be informed Apple about initial state
```c
 homekit_characteristic_t * ch= homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_ON);
 INIT_CHARACHTERISTIC_VAL(bool,ch,false);   //  will inform apple that lights is OFF
```


 After that you can add more accessories like this
 ```c
 hapservice_motion= hap_add_motion_service_as_accessory(homekit_accessory_category_security_system,"Motion",motion_callback,NULL);
```

The full list of all services and their characteristic can be found in the header file [characteristic.h](https://github.com/Yurik72/ESPHap/blob/master/characteristics.h). This header file is well documented and descibes all service types and their characteristics.

The API to add services and accessories is documented in the header file  [homeintegration.h](https://github.com/Yurik72/ESPHap/blob/master/homeintegration.h), its funtions are self-explanatory.

Once all accessories, services and characteristics are defined, we need to call
```c
hap_init_homekit_server();
```

3. Implement callback and notify functions

Every callback has the following parameters
 - characteristic
 - value 
 - context (callback parameters)
 
The callback function is invoked whenever the accessory's state is changed from within the Apple Home app. Based on the returned values, you may manage your device accordingly. Please check carefully which type (bool, int, float) must be used for the different characteristics.
```c
void led_callback(homekit_characteristic_t *ch, homekit_value_t value, void *context) {
    Serial.println("led_callback");
    digitalWrite(led_gpio, value.bool_value?HIGH:LOW);
}
```
Optionally, you may also implement notify functions which are used to inform the Apple Home app about device state changes. This is a must for accessories like thermostat and for a lightbulb we could notify the app about power state (on/off, represented in the bool value true/false).

To retrieve characteristics, the API function ``homekit_service_characteristic_by_type`` should be used. 
The first parameter is a pointer to the hapservice (from the setup), the second parameter is the characteristic type. 

```c
void notify_hap(){
homekit_characteristic_t * ch= homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_ON);
 HAP_NOTIFY_CHANGES(bool, ch, <new bool value>, 0)
 }
 ```
 
4. Main loop function

In the main loop function and **only for ESP8266** we have to add
 
```c
 #ifdef ESP8266
  hap_homekit_loop();
#endif
``` 

### Advanced features

#### Built-in web server
Since version 1.0.2, the ESPHap library contains a submodule for a built-in web server. To use it, you need include the corresponding header file
```c
#include <hapweb/hap_webserver.hpp>
``` 
Within the setup section, include  
```c
set_indexhml(FPSTR(INDEX_HTML)); // optional if you want to have your own root page
hap_webserver_begin();
``` 

``set_indexhml(FPSTR(INDEX_HTML))`` allows to define your own root page content, see example [Advanced Led](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHapAdvancedLed).

#### Setup by QR Code
If you use the built-in web server, by default it provides access to a setup/pairing page/image by QR code which can be accessed by visiting http://\<Your Device IP Address\>/setup.html, see example [Advanced LED](https://github.com/Yurik72/ESPHap/tree/master/examples/EspHapAdvancedLed).

# Versions history
## v1.0.7
   
   Fixed issue with for stable pairing process on ESP8266
   
## v1.0.3
- small bug fixes
- new examples

## v1.0.2
 - implement submodule for file storage of pairing data [hapfilestorage.hpp](https://github.com/Yurik72/ESPHap/blob/master/hapfilestorage/hapfilestorage.hpp) which allows to reuse basic function for store pairing data on SPIFFs file system. 
 - implement submodules for internal web server [hapweb](https://github.com/Yurik72/ESPHap/tree/master/hapweb). Now Web server can be easily setup and handle file browser and your own portal for device. Plus handling of OTA.
- implement submodules for pairing by QR code [hapqr.hpp](https://github.com/Yurik72/ESPHap/blob/master/qr/hapqr.hpp).Together with web server you can got on your browser QR image, which can be easily scan for pairing purpose. To access QR code you need enter http://\<IPADDRESS\>/setup.html.

## v1.0.1
 - Minor bug fixes and more examples

## v1.0
 - First success version works both with ESP32 and ESP8266
 
## Support this project
You can support the development of this project by donating to it

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=JVZWJ6FSMURSL&currency_code=USD&source=url)

*TODO* - 
