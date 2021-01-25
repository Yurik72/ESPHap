#pragma once
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7200;
const int   daylightOffset_sec = -3600;
#include <time.h>
#if defined(ESP8266)
#include <WiFiUdp.h>
WiFiUDP ntpUDP;
NTPClient* pTimeClient=null;
#endif

void startTime(){
#if defined(ESP8266)
  ptimeClient = new NTPClient(ntpUDP, ntpServer.c_str(), gmtOffset_sec);
  ptimeClient->begin();
  ptimeClient->update();
#else
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
#endif
};

unsigned long getLocalTime_ms(){
  
unsigned long tm=0; 
 #if defined(ESP8266)
 if(ptimeClient){
    ptimeClient->update();
    tm= ptimeClient->getEpochTime();
 }
#else   
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      tm = mktime(&timeinfo);
    }
#endif
return tm;
};
