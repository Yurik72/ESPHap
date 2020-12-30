#ifndef __APP_HTTPD_H
#define __APP_HTTPD_H
#include "fd_forward.h"
#include "fr_forward.h"
#include "fr_flash.h"
extern face_id_name_list st_face_list;

extern  int8_t detection_enabled ;
extern int8_t recognition_enabled;
extern int8_t is_enrolling ;

mtmn_config_t* get_mtmn_config();
int run_face_recognition_ex(dl_matrix3du_t *image_matrix, box_array_t *net_boxes);
void stop_httpserver();
void start_httpserver();
#endif
