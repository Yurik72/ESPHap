
#include "looptask_int.h"
#include "Arduino.h"
#include "esp_camera.h"
#include "fb_gfx.h"
#include "fd_forward.h"
#include "fr_forward.h"
#include "app_httpd.h"
#include "Motion.h"
#include <WiFi.h>
static unsigned long next_wifi_check =0;


TaskHandle_t taskhandle;
SemaphoreHandle_t sem_intloop;
#define TASK_MEM_STACK 16384
void setup_internal_loop(){
  Serial.println("setup_internal_loop ");
#ifdef INTERNAL_LOOP 
 Serial.println("setup_internal_loop  entered ");
sem_intloop= xSemaphoreCreateMutex();
xTaskCreatePinnedToCore(
    runcore,
    "cam_int_loop",
    TASK_MEM_STACK,
    TASK_PARAM,
    TASK_PRIORITY,
    &taskhandle,
    TASK_CORE);

#endif
}
sem_guard::sem_guard(){
 #ifdef INTERNAL_LOOP
 xSemaphoreTake( sem_intloop, ( TickType_t ) SEMAPHORE_DELAY_TICK *1000);
#endif
}
sem_guard::~sem_guard(){
   #ifdef INTERNAL_LOOP
   xSemaphoreGive(sem_intloop);
   #endif
 }
static void runcore(void*param){
   for(;;){ 
    if( xSemaphoreTake( sem_intloop, ( TickType_t ) SEMAPHORE_DELAY_TICK ) == pdTRUE ) {
      // xSemaphoreTake(sem_intloop, portMAX_DELAY);
       loop_camera();
       check_wifi_connection();
       vTaskDelay(10);
       xSemaphoreGive(sem_intloop);
    }
   }
}

static void loop_camera(){
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t * _jpg_buf = NULL;
    char * part_buf[64];
    dl_matrix3du_t *image_matrix = NULL;
    bool detected = false;
    int face_id = 0;
    int64_t fr_start = 0;
    int64_t fr_ready = 0;
    int64_t fr_face = 0;
    int64_t fr_recognize = 0;
    int64_t fr_encode = 0;
    detected = false;
    face_id = 0;
	if (motion_detection_enabled) {
		run_motion_detection();
	}
        fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Camera capture failed");
           return;
        }
		if (detection_enabled) {
			fr_start = esp_timer_get_time();
			fr_ready = fr_start;
			fr_face = fr_start;
			fr_encode = fr_start;
			fr_recognize = fr_start;
			if (fb->width > 400) {
				if (fb->format != PIXFORMAT_JPEG) {
					bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
					esp_camera_fb_return(fb);
					fb = NULL;
					if (!jpeg_converted) {
						Serial.println("JPEG compression failed");
						res = ESP_FAIL;
					}
				}
				else {
					_jpg_buf_len = fb->len;
					_jpg_buf = fb->buf;
				}
			}
			else {

				image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);

				if (!image_matrix) {
					Serial.println("dl_matrix3du_alloc failed");
					res = ESP_FAIL;
				}
				else {
					if (!fmt2rgb888(fb->buf, fb->len, fb->format, image_matrix->item)) {
						Serial.println("fmt2rgb888 failed");
						res = ESP_FAIL;
					}
					else {
						fr_ready = esp_timer_get_time();
						box_array_t *net_boxes = NULL;

						net_boxes = face_detect(image_matrix, get_mtmn_config());

						fr_face = esp_timer_get_time();
						fr_recognize = fr_face;
						if (net_boxes || fb->format != PIXFORMAT_JPEG) {
							if (net_boxes) {
								detected = true;

								face_id = run_face_recognition_ex(image_matrix, net_boxes);

								fr_recognize = esp_timer_get_time();
								//draw_face_boxes(image_matrix, net_boxes, face_id);
								free(net_boxes->score);
								free(net_boxes->box);
								free(net_boxes->landmark);
								free(net_boxes);
							}
							if (!fmt2jpg(image_matrix->item, fb->width*fb->height * 3, fb->width, fb->height, PIXFORMAT_RGB888, 90, &_jpg_buf, &_jpg_buf_len)) {
								Serial.println("fmt2jpg failed");
								res = ESP_FAIL;
							}
							esp_camera_fb_return(fb);
							fb = NULL;
						}
						else {
							_jpg_buf = fb->buf;
							_jpg_buf_len = fb->len;
						}
						fr_encode = esp_timer_get_time();
					}
					dl_matrix3du_free(image_matrix);
				}
			}

		}
        if(fb){
            esp_camera_fb_return(fb);
            fb = NULL;
            _jpg_buf = NULL;
        } else if(_jpg_buf){
            free(_jpg_buf);
            _jpg_buf = NULL;
        }

        int64_t fr_end = esp_timer_get_time();

        int64_t ready_time = (fr_ready - fr_start)/1000;
        int64_t face_time = (fr_face - fr_ready)/1000;
        int64_t recognize_time = (fr_recognize - fr_face)/1000;
        int64_t encode_time = (fr_encode - fr_recognize)/1000;
        int64_t process_time = (fr_encode - fr_start)/1000;
//          Serial.println("Internal loop");
          if(detected)
            Serial.println("face detected");
          if(face_id)
            Serial.println(face_id);
          
        /*
        int64_t frame_time = fr_end - last_frame;
        last_frame = fr_end;
        frame_time /= 1000;
        uint32_t avg_frame_time = ra_filter_run(&ra_filter, frame_time);
        Serial.printf("MJPG: %uB %ums (%.1ffps), AVG: %ums (%.1ffps), %u+%u+%u+%u=%u %s%d\n",
            (uint32_t)(_jpg_buf_len),
            (uint32_t)frame_time, 1000.0 / (uint32_t)frame_time,
            avg_frame_time, 1000.0 / avg_frame_time,
            (uint32_t)ready_time, (uint32_t)face_time, (uint32_t)recognize_time, (uint32_t)encode_time, (uint32_t)process_time,
            (detected)?"DETECTED ":"", face_id
        );
    */

}


void check_wifi_connection(){
  if(next_wifi_check >millis())
   return;


if (!WiFi.isConnected()) {
      
     
        unsigned long delaytime = millis() + 8000;
      
        while (!WiFi.isConnected() && delaytime > millis()) { //let chanse to wifi to be established 
          vTaskDelay(2000);
        }
       
  
}
  next_wifi_check =millis()+100000;
}
