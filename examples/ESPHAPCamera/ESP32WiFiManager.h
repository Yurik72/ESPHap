

#ifndef ESPAsyncWiFiManager_h
#define ESPAsyncWiFiManager_h

#include <pgmspace.h>
#include <Arduino.h>
#include "esp_http_server.h"
#include "esp_timer.h"
#include <memory>
#include <WiFi.h>

const char WFM_HTTP_HEAD[] PROGMEM        = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><title>{v}</title>";
const char HTTP_STYLE[] PROGMEM           = "<style>.c{text-align: center;} div,input{padding:5px;font-size:1em;} input{width:95%;} body{text-align: center;font-family:verdana;} button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} .q{float: right;width: 64px;text-align: right;} .l{background: url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAALVBMVEX///8EBwfBwsLw8PAzNjaCg4NTVVUjJiZDRUUUFxdiZGSho6OSk5Pg4eFydHTCjaf3AAAAZElEQVQ4je2NSw7AIAhEBamKn97/uMXEGBvozkWb9C2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcqxSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1HGfZAAAAABJRU5ErkJggg==\") no-repeat left center;background-size: 1em;}</style>";
const char HTTP_SCRIPT[] PROGMEM          = "<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}</script>";
const char HTTP_HEAD_END[] PROGMEM        = "</head><body><div style='text-align:left;display:inline-block;min-width:260px;'>";
const char HTTP_PORTAL_OPTIONS[] PROGMEM  = "<form action=\"/wifi\" method=\"get\"><button>Configure WiFi</button></form><br/><form action=\"/0wifi\" method=\"get\"><button>Configure WiFi (No Scan)</button></form><br/><form action=\"/i\" method=\"get\"><button>Info</button></form><br/><form action=\"/r\" method=\"post\"><button>Reset</button></form>";
const char HTTP_ITEM[] PROGMEM            = "<div><a href='#p' onclick='c(this)'>{v}</a>&nbsp;<span class='q {i}'>{r}%</span></div>";
const char HTTP_FORM_START[] PROGMEM      = "<form method='get' action='wifisave'><input id='s' name='s' length=32 placeholder='SSID'><br/><input id='p' name='p' length=64 type='password' placeholder='password'><br/>";
const char HTTP_FORM_PARAM[] PROGMEM      = "<br/><input id='{i}' name='{n}' length={l} placeholder='{p}' value='{v}' {c}>";
const char HTTP_FORM_END[] PROGMEM        = "<br/><button type='submit'>save</button></form>";
const char HTTP_SCAN_LINK[] PROGMEM       = "<br/><div class=\"c\"><a href=\"/wifi\">Scan</a></div>";
const char HTTP_SAVED[] PROGMEM           = "<div>Credentials Saved<br />Trying to connect ESP to network.<br />If it fails reconnect to AP to try again</div>";
const char HTTP_END[] PROGMEM             = "</div></body></html>";

#define WIFI_MANAGER_MAX_PARAMS 10

#define HANDLER_FN(meth_suffix) [](httpd_req_t *req) ->esp_err_t { \
  ESP32WiFiManager* self = (ESP32WiFiManager*)req->user_ctx; \
  if (self) \
	  return self->handle##meth_suffix(req);\
  return ESP_FAIL; \
  }
#define HANDLER_FN_1(meth_suffix,param1) [](httpd_req_t *req) ->esp_err_t { \
  ESP32WiFiManager* self = (ESP32WiFiManager*)req->user_ctx; \
  if (self) \
	  return self->handle##meth_suffix(req,param1);\
	  return ESP_FAIL; \
  }
#define URI_INIT(path,meth_suffix) { \
			.uri =path,\
			.method = HTTP_GET,\
			.handler = HANDLER_FN(meth_suffix),\
			.user_ctx = this\
			};
#define URI_INIT_1(path,meth_suffix,param1) { \
			.uri =path,\
			.method = HTTP_GET,\
			.handler = HANDLER_FN_1(meth_suffix,param1),\
			.user_ctx = this\
			};


unsigned char local_h2int(char c);
String local_urldecode(String input);

class AsyncWiFiManagerParameter {
public:
  AsyncWiFiManagerParameter(const char *custom);
  AsyncWiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length);
  AsyncWiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom);

  const char *getID();
  const char *getValue();
  const char *getPlaceholder();
  int         getValueLength();
  const char *getCustomHTML();
private:
  const char *_id;
  const char *_placeholder;
  char       *_value;
  int         _length;
  const char *_customHTML;

  void init(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom);

  friend class ESP32WiFiManager;
};


class WiFiResult
{
public:
  bool duplicate;
  String SSID;
  uint8_t encryptionType;
  int32_t RSSI;
  uint8_t* BSSID;
  int32_t channel;
  bool isHidden;

  WiFiResult()
  {
  }


};


class ESP32WiFiManager
{
public:


	ESP32WiFiManager();
	~ESP32WiFiManager();
  //ESP32WiFiManager(AsyncWebServer * server, DNSServer *dns);


  void          scan();
  String        scanModal();
  void          loop();
  void          safeLoop();
  void          criticalLoop();
  String        infoAsString();

  boolean       autoConnect(bool bstartConfig  = true);
  boolean       autoConnect(char const *apName, char const *apPassword = NULL,bool bstartConfig =true);

  //if you want to always start the config portal, without trying to connect first
  boolean       startConfigPortal(char const *apName, char const *apPassword = NULL);
  void startConfigPortalModeless(char const *apName, char const *apPassword);

  // get the AP name of the config portal, so it can be used in the callback
  String        getConfigPortalSSID();

  void          resetSettings();

  //sets timeout before webserver loop ends and exits even if there has been no setup.
  //usefully for devices that failed to connect at some point and got stuck in a webserver loop
  //in seconds setConfigPortalTimeout is a new name for setTimeout
  void          setConfigPortalTimeout(unsigned long seconds);
  void          setTimeout(unsigned long seconds);

  //sets timeout for which to attempt connecting, usefull if you get a lot of failed connects
  void          setConnectTimeout(unsigned long seconds);


  void          setDebugOutput(boolean debug);
  //defaults to not showing anything under 8% signal quality if called
  void          setMinimumSignalQuality(int quality = 8);
  //sets a custom ip /gateway /subnet configuration
  void          setAPStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
  //sets config for a static IP
  void          setSTAStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn, IPAddress dns1=(uint32_t)0x00000000, IPAddress dns2=(uint32_t)0x00000000);
  //called when AP mode and config portal is started
  void          setAPCallback( void (*func)(ESP32WiFiManager*) );
  //called when settings have been changed and connection was successful
  void          setSaveConfigCallback( void (*func)(void) );
  //adds a custom parameter
  void          addParameter(AsyncWiFiManagerParameter *p);
  void          cleanParameters();
  //if this is set, it will exit after config, even if connection is unsucessful.
  void          setBreakAfterConfig(boolean shouldBreak);
  //if this is set, try WPS setup when starting (this will delay config portal for up to 2 mins)
  //TODO
  //if this is set, customise style
  void          setCustomHeadElement(const char* element);
  //if this is true, remove duplicated Access Points - defaut true
  void          setRemoveDuplicateAPs(boolean removeDuplicates);
  void          startOfflineApp(char const *apName, char const *apPassword = NULL);
private:



  boolean         _modeless;
  int             scannow;
  int             shouldscan;
  boolean         needInfo = true;

  //const int     WM_DONE                 = 0;
  //const int     WM_WAIT                 = 10;

  //const String  HTTP_HEAD = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"/><title>{v}</title>";

  void          setupConfigPortal();
#ifdef NO_EXTRA_4K_HEAP
  void          startWPS();
#endif
  String        pager;
  wl_status_t   wifiStatus;
  const char*   _apName                 = "no-net";
  const char*   _apPassword             = NULL;
  String        _ssid                   = "";
  String        _pass                   = "";
  unsigned long _configPortalTimeout    = 0;
  unsigned long _connectTimeout         = 0;
  unsigned long _configPortalStart      = 0;

  IPAddress     _ap_static_ip;
  IPAddress     _ap_static_gw;
  IPAddress     _ap_static_sn;
  IPAddress     _sta_static_ip;
  IPAddress     _sta_static_gw;
  IPAddress     _sta_static_sn;
  IPAddress     _sta_static_dns1= (uint32_t)0x00000000;
  IPAddress     _sta_static_dns2= (uint32_t)0x00000000;

  int           _paramsCount            = 0;
  int           _minimumQuality         = -1;
  boolean       _removeDuplicateAPs     = true;
  boolean       _shouldBreakAfterConfig = false;
#ifdef NO_EXTRA_4K_HEAP
  boolean       _tryWPS                 = false;
#endif
  const char*   _customHeadElement      = "";

  //String        getEEPROMString(int start, int len);
  //void          setEEPROMString(int start, int len, String string);

  int           status = WL_IDLE_STATUS;
  int           connectWifi(String ssid, String pass,String hostName);
  uint8_t       waitForConnectResult();
  void          setInfo();

  String networkListAsString();
  httpd_handle_t autoconnect_httpd;
  esp_err_t          handleRoot(httpd_req_t *req);
  esp_err_t          handleWifi(httpd_req_t *req,boolean scan);
  esp_err_t          handleWifiSave(httpd_req_t *req);
  esp_err_t          handleInfo(httpd_req_t *req);
  esp_err_t          handleReset(httpd_req_t *req);
  esp_err_t          handleNotFound(httpd_req_t *req);
  esp_err_t          handle204(httpd_req_t *req);
  boolean       captivePortal(httpd_req_t *req);

  // DNS server
  const byte    DNS_PORT = 53;

  //helpers
  int           getRSSIasQuality(int RSSI);
  boolean       isIp(String str);
  String        toStringIp(IPAddress ip);

  boolean       connect;
  boolean       _debug = true;

  WiFiResult    *wifiSSIDs;
  int           wifiSSIDCount;
  boolean       wifiSSIDscan;

  void (*_apcallback)(ESP32WiFiManager*) = NULL;
  void (*_savecallback)(void) = NULL;

  AsyncWiFiManagerParameter* _params[WIFI_MANAGER_MAX_PARAMS];

  template <typename Generic>
  void          DEBUG_WM(Generic text);

  template <class T>
  auto optionalIPFromString(T *obj, const char *s) -> decltype(  obj->fromString(s)  ) {
    return  obj->fromString(s);

  }
  auto optionalIPFromString(...) -> bool {
    DEBUG_WM("NO fromString METHOD ON IPAddress, you need ESP8266 core 2.1.0 or newer for Custom IP configuration to work.");
    return false;
  }

};

#endif
