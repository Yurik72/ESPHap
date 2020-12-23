#ifndef __USERS_HTTP_H
#define __USERS_HTTP_H

#include "esp_camera.h"
#include "fr_forward.h"
#include "fr_flash.h"
#include "config.h"
#include "app_httpd.h"
#include "esp_http_server.h"
#ifdef SUPPORT_SERVO
#include "servo_control.h"
#endif 
#define GPIO_RECOGNIZE 2   // GPIO=4 is highlight led, can be used for demostration efect
  
#include "html_subj.h"

#ifdef SUPPORT_GPIO
#include <Ticker.h>
#endif 
static String setUserName="unkn";

esp_err_t face_detected_action(char* facename);

static esp_err_t setupsubjects(){
#ifdef SUPPORT_GPIO
  pinMode(GPIO_RECOGNIZE, OUTPUT);
  digitalWrite(GPIO_RECOGNIZE,LOW);
  Serial.println("GPIO setup");
#endif
#ifdef SUPPORT_SERVO
setup_servo();
#endif
}
static  char* get_enroll_user_name(){
  return (char*)setUserName.c_str();
}
static String get_faces_list(){
  face_id_node *head = st_face_list.head;
  String  faces="";
  for (int i = 0; i < st_face_list.count; i++) // loop current faces
  {
    faces+= head->id_name;
    
    head = head->next;
  }
  return faces;
}
static String get_html_faces_list(){
  face_id_node *head = st_face_list.head;
  String  faces="";
  for (int i = 0; i < st_face_list.count; i++) // loop current faces
  {
      faces+="<div class=\"input-group\">";
      faces+=String("<label for=\"user")+String(i)+String("\">")+String(i)+String("</label>");
      faces+=String("<input id=\"user2")+String(i)+String(" type=\"text\" class=\"default-action\" value=\"") +String(head->id_name)+String("\">");
      faces+="</div>";
   
    
    head = head->next;
  }
  return faces;
}
static esp_err_t delete_all_faces()
{
  delete_face_all_in_flash_with_name(&st_face_list);
  
}
static esp_err_t delete_single_face(char* name)
{
  delete_face_id_in_flash_with_name(&st_face_list, name);
}
static esp_err_t run_command(httpd_req_t *req,char * var,char* value){
       Serial.println("run_command");
       Serial.println(var);
        Serial.println(value);
       
 if(!strcmp(var, "setname")) {
        if(value){
          setUserName=value;
        }
 }
 else if(!strcmp(var, "dell-all")) {
      delete_all_faces();
 }
 else if(!strcmp(var, "dell-this")) {
      
      delete_single_face(value);
 }
 #ifdef SUPPORT_SERVO
 else if(!strcmp(var, "vrot")) {
      uint8_t uval=atoi(value);
      set_servo_v(uval);
 }
 else if(!strcmp(var, "hrot")) {
      uint8_t uval=atoi(value);
      set_servo_h(uval);
 }
#endif
// return httpd_resp_send_500(req);
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, NULL, 0);
}
static esp_err_t user_handler(httpd_req_t *req){
    httpd_resp_set_type(req, "text/html");
    String response="Hello World";
    char*  buf;
    size_t buf_len;
    char variable[32] = {0,};
    char value[32] = {0,};

     httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*"); //YK we will support cross domain as well
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = (char*)malloc(buf_len);
        if(!buf){
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            if (httpd_query_key_value(buf, "var", variable, sizeof(variable)) == ESP_OK &&
                httpd_query_key_value(buf, "val", value, sizeof(value)) == ESP_OK) {
            } else {
                free(buf);
                httpd_resp_send_404(req);
                return ESP_FAIL;
            }
        } else {
            free(buf);
            httpd_resp_send_404(req);
            return ESP_FAIL;
        }
        free(buf);
    } else {
        response=HTML_SUBJ;
        response.replace("{user}", get_html_faces_list());
        return httpd_resp_send(req, response.c_str(), response.length());
    }

    return run_command(req,variable,value);
    
 
}
#ifdef SUPPORT_GPIO
void DeactivateGpio();

static esp_err_t ActivateGpio();

#endif
#if defined(SUPPORT_RF)

String bin(char c);
 
#define MAX_CHAR_TORF_SEND 4
String bin_string(char*  c) ;

long encode_facename(char*  c) ;
 
#define RF_PULSE 232
#define RF_PROTOCOL 1


static esp_err_t send_name_toRF(char* facename);

#endif





#endif
