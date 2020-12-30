#include "Motion.h"
#include "Arduino.h"
#include "config.h"
#include "JPEGDecoder.h"
#include "EMailSender.h"
#include "app_httpd.h"
#ifdef ENABLE_HAP
extern "C" {
#include "homeintegration.h"
}
#endif
static callback_motiondetected_t callback_motiondetected = NULL;

int8_t motion_detection_enabled = 1;
int8_t motion_emailsend_enabled = 1;

uint16_t *buf1 = NULL;
size_t len1=0;
uint16_t *buf2 = NULL;
size_t len2=0;
size_t level_sensitivity = 20;
size_t motion_sensitivity = 300;
uint16_t probe_height=0;
uint16_t probe_width = 0;

//camera_fb_t * fb1 = NULL;
//camera_fb_t * fb2 = NULL;

#define PROBE_PERIOD_MS 500
#define MOTION_PERIOD_MS 5000
long last_probe_ms = 0;
long last_motion_ms = 0;

#define RED_MASK  0xF800
#define GREEN_MASK  0x7E0
#define BLUE_MASK  0x1F



#define RED_FROM16(pixel) (pixel & RED_MASK) >> 11
#define GREEN_FROM16(pixel) (pixel & GREEN_MASK) >> 5
#define BLUE_FROM16(pixel) (pixel & BLUE_MASK)


uint8_t check_motion_grayscale();
uint8_t check_motion_decoded();
uint8_t send_capture_email();
uint8_t send_email(uint8_t* jpegbuff, size_t jpegbufflen);
void set_motioncallback(callback_motiondetected_t f) {
	callback_motiondetected = f;
}
bool read_fromdecoder(uint16_t **buf, size_t *len) {
	uint16_t *pImg;
	uint16_t mcu_w = JpegDec.MCUWidth;
	uint16_t mcu_h = JpegDec.MCUHeight;
	uint32_t max_x = JpegDec.width;
	uint32_t max_y = JpegDec.height;
	probe_width= JpegDec.width;
	probe_height = JpegDec.height;
	*len = max_x* max_y * sizeof(**buf);
	*buf = (uint16_t*)ps_malloc(*len);
	if (!(*buf)) {
		Serial.println(F("can't allocate memory"));
		return false;
	}
	while (JpegDec.read()) {

		// save a pointer to the image block
		pImg = JpegDec.pImage;

		// calculate where the image block should be drawn on the screen
		int mcu_x = JpegDec.MCUx * mcu_w ;
		int mcu_y = JpegDec.MCUy * mcu_h ;
		for (int y = mcu_y; y < (mcu_h+ mcu_y); y++) {
			for (int x = mcu_x; x < (mcu_w+ mcu_x); x++) {
				(*buf)[y*max_y + x] = *pImg;
				pImg++;
			}
		}
	}
	return true;
}
bool grab_probe( uint16_t **buf, size_t *len) {
	if (!psramFound()) {
		Serial.println(F("grab_probe not possible without psram"));
		return false;
	}
	camera_fb_t *frame_buffer = esp_camera_fb_get();    // capture frame from camera
	if (!(frame_buffer)) {
		Serial.println(F("grab_probe error"));
		return false;
	}
	JpegDec.abort();
	JpegDec.reduce = 1;
	//Serial.printf("JpegDecframe_buffer:%d \r\n", frame_buffer->len);
	bool res = JpegDec.decodeArray(frame_buffer->buf, frame_buffer->len) == 1;
	if (res == 1) {

	}

//	Serial.printf("JpegDec  jpegwidth:%d jpegheight:%d res:%d len:%d \r\n", JpegDec.width, JpegDec.height,res,*len);
	res=read_fromdecoder(buf,len);
	esp_camera_fb_return(frame_buffer);
	return res;
	/*

	sensor_t *s = esp_camera_sensor_get();
	pixformat_t old = s->pixformat;
	framesize_t oldframe = s->status.framesize;
	if (old != PIXFORMAT_GRAYSCALE) {
		Serial.printf("pixformat %d \r\n", old);
		return false;
	}
	if (old != PIXFORMAT_GRAYSCALE) {
		s->set_framesize(s, FRAMESIZE_QVGA);
		s->set_pixformat(s, PIXFORMAT_GRAYSCALE);
		camera_fb_t *frame_buffer = esp_camera_fb_get();    // capture frame from camera
		esp_camera_fb_return(frame_buffer);
	}
	camera_fb_t* temp = esp_camera_fb_get();    // capture frame from camera
	if (oldframe != PIXFORMAT_GRAYSCALE) {
		//s->set_pixformat(s, old);
		//s->set_framesize(s, oldframe);
	}
	if (!(temp)) {
		Serial.println("grab_probe error");
		return false;
	}

	//*buf = (uint8_t *)malloc(25);//(uint8_t *)ps_malloc((temp)->len);
	if (!(*buf)) {
		Serial.println("Càn't allocate memory ");
		*len = 0;
	}
	*len = 25;// (temp)->len;
	Serial.printf("allocated %d \r\n", (temp)->len);
	probe_height = 5; /// (temp)->height;
	probe_width = 5;// (temp)->width;
	Serial.printf("width %d height %d\r\n", probe_width,probe_height);
	memcpy((*buf),temp->buf, (*len));
	esp_camera_fb_return(temp);
	
	return true;
	*/
}

uint8_t run_motion_detection() {

	uint8_t *buf = NULL;
	size_t len;

	if (!buf1) {
		if (!grab_probe(&buf1,&len1)) {
			Serial.println(F("grab_probe 1"));
			return 0;
		}

		last_probe_ms = millis();
	}
	if (!buf2 && (last_probe_ms + PROBE_PERIOD_MS) < millis()) {
		if (!grab_probe(&buf2, &len2)) {
			Serial.println(F("grab_probe 2"));
			return 0;
		}
	}
	uint8_t ismotion = 0;
	if (buf1 && buf2) {
		//Serial.println("Start motion detection");
		//check_motion_grayscale();
		ismotion=check_motion_decoded();
	}
	if (buf1 && buf2) {
		free(buf1);
		free(buf2);
		buf1 = NULL;
		buf2 = NULL;
		if (ismotion && motion_emailsend_enabled)
			send_capture_email();
	}


}
uint8_t check_motion_decoded() {
	if (!buf1 || !buf2) {
		Serial.printf("check_motion wrong input data");
		return 0;
	}

	size_t pixelsize = 2;
	long motion_level = 0;
	uint16_t* offs1 = buf1;
	uint16_t* offs2 = buf2;
	int xmin = probe_width;
	int xmax = 0;
	int ymin = probe_height;
	int ymax = 0;
	uint8_t res = 0;
	for (int y = 0; y < probe_height; y++)
	{
		for (int x = 0; x < probe_width; x++) {
			uint16_t  offs = (y * probe_width + x)* pixelsize;
			uint16_t gs1 = offs1[offs];
			uint16_t gs2 = offs2[offs];
			byte r1 = RED_FROM16(gs1);
			byte g1 = GREEN_FROM16(gs1);
			byte b1 = BLUE_FROM16(gs1);
			byte r2 = RED_FROM16(gs2);
			byte g2 = GREEN_FROM16(gs2);
			byte b2 = BLUE_FROM16(gs2);
			int w = abs(int(r1) - int(r2))+ abs(int(g1) - int(g2))+ abs(int(b1) - int(b2));

			if (w > level_sensitivity) {
				xmin = min(xmin, x);
				ymin= min(ymin, y);
				xmax = max(xmax, x);
				ymax = max(ymax, y);
				motion_level++;
			}
		}
	}
	
	if (motion_level > motion_sensitivity) {
		Serial.printf("Motion detected %d:\r\n", motion_level);
		//Serial.printf("RECT %d,%d,%d,%d:\r\n", xmin, ymin, xmax, ymax);
		last_motion_ms = millis();
		res = 1;
		if (callback_motiondetected)
			callback_motiondetected(1,motion_level);
	}
	else {
		if ((last_motion_ms + MOTION_PERIOD_MS) > millis()) {
			if (callback_motiondetected)
				callback_motiondetected(0, 0);
			last_motion_ms = 0;
		}
	}
	return res;
}
uint8_t check_motion_grayscale() {
	if (!buf1 || !buf2)  {
		Serial.printf("check_motion wrong input data");
		return 0;
	}

	size_t pixelsize = 1;
	long motion_level = 0;
	byte* offs1 = (byte *) buf1;
	byte* offs2 = (byte *)buf2;


	for (int y = 0; y < probe_height; y++)
	{
		for (int x = 0; x < probe_width; x++) {
			byte  offs = (y * probe_width + x)* pixelsize;
			byte gs1 = offs1[offs];
			byte gs2 = offs2[offs];
			int w = abs(int(gs1) - int(gs2));

			if (w > level_sensitivity) {

				motion_level++;
			}
		}
	}
	Serial.printf("Motion level   %d:\r\n", motion_level);
	if (motion_level > motion_sensitivity) {
		Serial.printf("Motion detected %d:\r\n", motion_level);
		if (callback_motiondetected)
			callback_motiondetected(1,motion_level);
	}
}
uint8_t send_capture_email() {
	Serial.println(ESP.getFreeHeap());
	sensor_t *s = esp_camera_sensor_get();
	if (!s)
		return false;
	framesize_t oldframe = s->status.framesize;
	s->set_framesize(s, FRAMESIZE_XGA);
	
	camera_fb_t *frame_buffer = NULL;
	for (int i = 0; i < 10; i++) {
		frame_buffer = esp_camera_fb_get();    // capture frame from camera
		delay(10);
		if (!(frame_buffer)) {
			Serial.println(F("capture email error"));
			return false;
		}
		esp_camera_fb_return(frame_buffer);
	}
	
	frame_buffer = esp_camera_fb_get();
	s->set_framesize(s, oldframe);
	if (!(frame_buffer)) {
		
		return false;
	}
	send_email(frame_buffer->buf, frame_buffer->len);
	esp_camera_fb_return(frame_buffer);
}
uint8_t send_email(uint8_t* jpegbuff, size_t jpegbufflen) {
	EMailSender emailSend(EMAIL_SERVER_USER, EMAIL_SERVER_PWD, EMAIL_FROM, EMAIL_SERVER, EMAIL_SERVER_PORT);
	stop_httpserver();
#ifdef ENABLE_HAP
	hap_setstopflag();
	delay(2000);
#endif
	EMailSender::EMailMessage message;
	message.subject = "Motion detected";
	message.message = "See capture";
	Serial.println(ESP.getFreeHeap());
	EMailSender::FileDescriptior fileDescriptor[1];

	fileDescriptor[0].filename = F("motion.jpg");
	fileDescriptor[0].url = F("/motion.jpg");
	fileDescriptor[0].mime = "image/jpg";
	fileDescriptor[0].encode64 = true;
	fileDescriptor[0].filedata = jpegbuff;
	fileDescriptor[0].filedatalen = jpegbufflen;
	fileDescriptor[0].storageType = EMailSender::EMAIL_STORAGE_TYPE_ARRAY;

	EMailSender::Attachments attachs = { 1, fileDescriptor };

	EMailSender::Response resp = emailSend.send(EMAIL_TO, message, attachs);

	Serial.println("Sending status: ");

	Serial.println(resp.status);
	Serial.println(resp.code);
	Serial.println(resp.desc);
#ifdef ENABLE_HAP
	hap_init_homekit_server();
	start_httpserver();
#endif
}