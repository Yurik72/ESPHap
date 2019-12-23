[![Gitter](https://badges.gitter.im/Join%20Chat.svg)]




ESPHap
===========

This is a library for easily & efficiently integrating arduino projects based on Esp32 with Apple Home Kit by  native protocols 
and without any additional bridges

!At this moment only ESP32 supported

Many thanks to [maximkulkin](https://github.com/maximkulkin) for providing fine libraries for native integration,
this project uses this as well.

## Build instruction

This library should be located under standart Arduino libraries folder. 
Library is depends from [wolfssl](https://github.com/wolfSSL)  , but it requires some preparation before usage,
full instruction how to do that, can be found [there](https://www.wolfssl.com/doxygen/md__Users_alexabrahamson_Work_wolfssl-CLEAN_IDE_ARDUINO_README.html)

if you are doing this manually you should made specific configuration before usage.
For simplify process you can use [wolfssl.rar](https://github.com/Yurik72/ESPHap/blob/master/wolfssl/wolfSSL.rar) archive , which already prepared. You just need to extract this content  into Arduino Libraries folder

If you are going to prepare this manually, please reuse/check settings.h and user_settings.h  from attached archive or
 [this location](https://github.com/Yurik72/ESPHap/tree/master/wolfssl)
 You need copy/replace this files in wolfssl components.

## Getting help


## Simple example

examples folder contains simple example EspHapLed how to handle led (or any such as relay) 
More examples will be later, hovewer you can see the same examples/implemenmtation
[esphapcontroller](https://github.com/Yurik72/esphapcontroller)

This example used as well implemenation for pairing storage. it will be stored in spiffs system with file name pair.dat

In the ino file change your wifi name and password. After connecting to wifi device should be accesible for pairing from Apple home.
At this moment QR is not generated, so please use manual pairing by enetering password  11111111


## For more information




*TODO* - 
