#include "users_http.h"
#ifdef SUPPORT_GPIO
Ticker TickGpio;
#endif 
#ifdef SUPPORT_RF
#include "RCSwitch.h" 
RCSwitch RFmodule;
#define RF_TRANSMIT_PIN 14
#endif


esp_err_t face_detected_action(char* facename){
  Serial.println("face_detected_action");
  #ifdef SUPPORT_RF
  send_name_toRF(facename);
  #endif
  #ifdef SUPPORT_GPIO
  ActivateGpio();
  #endif
};


#ifdef SUPPORT_GPIO
void DeactivateGpio(){
  Serial.println("DeactivateGpio");
 digitalWrite(GPIO_RECOGNIZE,LOW);
};
static esp_err_t ActivateGpio(){
  Serial.println("ActivateGpio");
  digitalWrite(GPIO_RECOGNIZE,HIGH);
  TickGpio.once_ms(3000, DeactivateGpio);
};

#endif

String bin(char c) 
{ 
    String res;
    byte b; 
   // for (int i = 8; i > 0; i = i / 2) 
    // for (int i = 128; i > 0; i = i / 2) 
    for (int i = 16; i > 0; i = i / 2)
       res+= ((c & i)? "1": "0"); 
   return res;
} ;
#define MAX_CHAR_TORF_SEND 4
String bin_string(char*  c) 
{ 
   
    String res;
    String encode=c;
    encode+="     ";  //5 spaces
    encode=encode.substring(0,MAX_CHAR_TORF_SEND);// will send only first 4 characters (5 bit per char
    for (int i = 0 ; i < MAX_CHAR_TORF_SEND; i ++) 
       res+= bin(encode.charAt(i)); 
      return res;
}; 
long encode_facename(char*  c) 
{ 
   
    long res=0;
    String encode=c;
    encode.toUpperCase();
    encode+="     ";  //5 spaces
    encode=encode.substring(0,MAX_CHAR_TORF_SEND);// will send only first 4 characters (5 bit per char
    for (int i = 0 ; i < MAX_CHAR_TORF_SEND; i ++){
      res= res <<5;   //5 bit perchar
       char current= encode.charAt(i);
       current=constrain(current,'A','Z');
       res+=(current-'A');
       //res+= bin(encode.charAt(i)); 
    }
      return res;
} ;

#define RF_PULSE 232
#define RF_PROTOCOL 1

#ifdef SUPPORT_RF
static esp_err_t send_name_toRF(char* facename){
  Serial.println("Sending RF");
  // Serial.println(facename);
  RFmodule.enableTransmit(RF_TRANSMIT_PIN);

  RFmodule.setPulseLength(RF_PULSE);

  // Optional set protocol (default is 1, will work for most outlets)

  RFmodule.setProtocol(RF_PROTOCOL);
  long toSend=encode_facename(facename);
  //RFmodule.send(bin_string(facename).c_str());
   Serial.println(toSend);
  RFmodule.send(toSend,MAX_CHAR_TORF_SEND*6);
  //delay(100);
  RFmodule.disableTransmit();
};
#endif
