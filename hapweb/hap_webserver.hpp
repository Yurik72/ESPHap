// ***************************************************************************
// SPIFFS Webserver
// Source: https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer/examples/FSBrowser
// ***************************************************************************
#pragma once
/*
  FSWebServer - Example WebServer with SPIFFS backend for esp8266
  Copyright (c) 2015 Hristo Gochkov. All rights reserved.
  This file is part of the ESP8266WebServer library for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  upload the contents of the data folder with MkSPIFFS Tool ("ESP8266 Sketch Data Upload" in Tools menu in Arduino IDE)
  or you can upload the contents of a folder if you CD in that folder and run the following command:
  for file in `ls -A1`; do curl -F "file=@$PWD/$file" esp8266fs.local/edit; done

  access the sample web page at http://esp8266fs.local
  edit the page by going to http://esp8266fs.local/edit
*/

#ifdef ESP8266
#include <ESP8266WebServer.h>
ESP8266WebServer server(80);

#endif
#ifdef ESP32
#include <WebServer.h>
WebServer server(80);
#endif
///#ifndef DBG_OUTPUT_PORT
//#define DBG_OUTPUT_PORT Serial
//#endif
#include "hap_filebrowse.h"
//#include "file_index_html.h"
#include "hap_file_setup.h"
#include "../qr/hapqr.hpp"
#include "../esphap_debug.h"

const char FILE_BROWSE_FILE[] PROGMEM = "/filebrowse.html";
const char FILE_INDEX[] PROGMEM = "index.html";
const char FILE_SETUP[] PROGMEM = "setup.html";
const char TEXT_PLAIN[] PROGMEM = "text/plain";
__FlashStringHelper* INDEX_HTML_INTERNAL =NULL;

void set_indexhml(const __FlashStringHelper* val) {
	INDEX_HTML_INTERNAL = (__FlashStringHelper*)val;
};
void saveIndex();
void setupOta();
void saveSetup();
void handleNotFound();
void handleFileCreate();
void handleFileDelete();
void handleJsonSave();
void handleJsonLoad();
void handleFileBrowser();
bool handleFileRead(String path);
void handleFileList();
void handleFileUpload();

#define SETUP_FILEHANDLES   server.on("/list", HTTP_GET, handleFileList); \
  server.on("/edit", HTTP_GET, []() { \
    if (!handleFileRead("/edit.htm")) server.send(404, FPSTR(TEXT_PLAIN), "FileNotFound"); \
  }); \
  server.on(PSTR("/edit"), HTTP_PUT, handleFileCreate); \
  server.on(PSTR("/restartesp"), HTTP_GET, [](){ESP.restart();}); \
  server.on(PSTR("/edit"), HTTP_DELETE, handleFileDelete); \
  server.on(PSTR("/edit"), HTTP_POST, []() { \
    server.sendHeader(PSTR("Access-Control-Allow-Origin"), "*"); \
    server.send(200, FPSTR(TEXT_PLAIN), ""); \
  }, handleFileUpload); \
  server.on(PSTR("/jsonsave"), handleJsonSave); \
  server.on(PSTR("/jsonload"), handleJsonLoad); \
  server.on(PSTR("/upload"), HTTP_POST, []() { server.send(200,  FPSTR(TEXT_PLAIN), ""); }, handleFileUpload); \
  server.on(PSTR("/browse"), handleFileBrowser);   \
  server.on(PSTR("/setup"),[]() {server.send(200,  FPSTR(TEXT_PLAIN), get_setup_json());});  \
  server.onNotFound([]() { \
		if (!handleFileRead(server.uri())) \
			handleNotFound(); \
		}); \
    setupOta(); \
   saveIndex();\
   saveSetup();


void hap_webserver_begin() {
	SETUP_FILEHANDLES
	server.begin();
};
void hap_webserver_loop() {
	server.handleClient();

};
File fsUploadFile;
unsigned char h2int(char c)
{
	if (c >= '0' && c <= '9') {
		return((unsigned char)c - '0');
	}
	if (c >= 'a' && c <= 'f') {
		return((unsigned char)c - 'a' + 10);
	}
	if (c >= 'A' && c <= 'F') {
		return((unsigned char)c - 'A' + 10);
	}
	return(0);
}
String urldecode(String input) // (based on https://code.google.com/p/avr-netino/)
{
	char c;
	String ret = "";

	for (byte t = 0; t < input.length(); t++)
	{
		c = input[t];
		if (c == '+') c = ' ';
		if (c == '%') {


			t++;
			c = input[t];
			t++;
			c = (h2int(c) << 4) | h2int(input[t]);
		}

		ret.concat(c);
	}
	return ret;

}


String urlencode(String str)
{
	String encodedString = "";
	char c;
	char code0;
	char code1;
	//char code2;
	for (unsigned int i = 0; i < str.length(); i++) {
		c = str.charAt(i);
		if (c == ' ') {
			encodedString += '+';
		}
		else if (isalnum(c)) {
			encodedString += c;
		}
		else {
			code1 = (c & 0xf) + '0';
			if ((c & 0xf) > 9) {
				code1 = (c & 0xf) - 10 + 'A';
			}
			c = (c >> 4) & 0xf;
			code0 = c + '0';
			if (c > 9) {
				code0 = c - 10 + 'A';
			}
			//code2 = '\0';
			encodedString += '%';
			encodedString += code0;
			encodedString += code1;
			//encodedString+=code2;
		}
		yield();
	}
	return encodedString;

}

//format bytes
String formatBytes(size_t bytes) {
	if (bytes < 1024) {
		return String(bytes) + "B";
	}
	else if (bytes < (1024 * 1024)) {
		return String(bytes / 1024.0) + "KB";
	}
	else if (bytes < (1024 * 1024 * 1024)) {
		return String(bytes / 1024.0 / 1024.0) + "MB";
	}
	else {
		return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
	}
}


String getContentType(String filename) {
	if (server.hasArg("download")) return PSTR("application/octet-stream");

	else if (filename.endsWith(PSTR(".htm"))) return PSTR("text/html");
	else if (filename.endsWith(PSTR(".html"))) return PSTR("text/html");
	else if (filename.endsWith(PSTR(".css"))) return PSTR("text/css");
	else if (filename.endsWith(PSTR(".js"))) return PSTR("application/javascript");
	else if (filename.endsWith(PSTR(".png"))) return PSTR("image/png");
	else if (filename.endsWith(PSTR(".gif"))) return PSTR("image/gif");
	else if (filename.endsWith(PSTR(".jpg"))) return PSTR("image/jpeg");
	else if (filename.endsWith(PSTR(".ico"))) return PSTR("image/x-icon");
	else if (filename.endsWith(PSTR(".xml"))) return PSTR("text/xml");
	else if (filename.endsWith(PSTR(".pdf"))) return PSTR("application/x-pdf");
	else if (filename.endsWith(PSTR(".zip"))) return PSTR("application/x-zip");
	else if (filename.endsWith(PSTR(".gz"))) return PSTR("application/x-gzip");
	return "text/plain";
}

bool handleFileRead(String path) {
	//INFO("handleFileRead:%s", path);
	if (path.endsWith("/")) path += FPSTR(FILE_INDEX);
	if (path.indexOf(".") == -1) path += FPSTR(FILE_INDEX); //some body asking non existing service. can happen as well with react routing
	String contentType = getContentType(path);
	path = urldecode(path);
	String pathWithGz = path + ".gz";
	if (SPIFFS.exists(pathWithGz))
		path += ".gz";
	if ( SPIFFS.exists(path)) {
		File file = SPIFFS.open(path, "r");
		
		server.sendHeader(PSTR("Access-Control-Allow-Origin"), "*");
		size_t sent = server.streamFile(file, contentType);
		file.close();
		return true;
	}
	ERROR("File not found %s", path.c_str());
	return false;
}

void handleFileUpload() {
	// DBG_OUTPUT_PORT.println("file upload start");

	if (server.uri() != "/upload") return;
	HTTPUpload& upload = server.upload();
	if (upload.status == UPLOAD_FILE_START) {

		String filename = upload.filename;

		if (!filename.startsWith("/")) filename = "/" + filename;
		String dirname = "";
		if (server.hasArg("dir")) {
			dirname = server.arg("dir");
			if (dirname.length() > 0) {
				if (!dirname.startsWith("/")) dirname = "/" + dirname;
				filename = dirname + filename;
			}
		}
		//DBG_OUTPUT_PORT.print("handleFileUpload filename: "); 
		//DBG_OUTPUT_PORT.println(filename);
		fsUploadFile = SPIFFS.open(filename, "w");
		filename = String();
	}
	else if (upload.status == UPLOAD_FILE_WRITE) {
		//DBG_OUTPUT_PORT.print("handleFileUpload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
		if (fsUploadFile)
			fsUploadFile.write(upload.buf, upload.currentSize);
	}
	else if (upload.status == UPLOAD_FILE_END) {
		if (fsUploadFile)
			fsUploadFile.close();
		//	DBG_OUTPUT_PORT.print(PSTR("handleFileUpload Size: ")); DBG_OUTPUT_PORT.println(upload.totalSize);
	}
}
bool handleFileDownload(const char* szName = NULL)
{
	String path;
	if (szName == NULL) {
		if (server.args() == 0) {
			server.send(500, FPSTR(TEXT_PLAIN), PSTR("BAD ARGS"));
			return false;
		}
		path = server.arg(0);

	}
	else {
		path = szName;
	}
	// DBG_OUTPUT_PORT.print("handleFileDownload: " + path);
	String contentType = "application/octet-stream";
	path = "/" + path;
	path = urldecode(path);
	//check if a public file.

	if (SPIFFS.exists(path)) {
		File file = SPIFFS.open(path, "r");
		server.streamFile(file, contentType);
		file.close();
		return true;
	}
	return false;
}
void handleFileDeleteByName(String path) {
	// DBG_OUTPUT_PORT.println("handleFileDeleteByName: " + path);
	if (path == "/")
		return server.send(500, FPSTR(TEXT_PLAIN), PSTR("BAD PATH"));
	String filetodel = path;
	if (!filetodel.startsWith("/"))
		filetodel = "/" + filetodel;

	if (!SPIFFS.exists(filetodel))
		return server.send(404, FPSTR(TEXT_PLAIN), PSTR("FileNotFound"));
	SPIFFS.remove(filetodel);
	server.send(200, "text/plain", "");

}
void handleFileDelete() {
	String path;
	//DBG_OUTPUT_PORT.println("handleFileDeleteByName start");
	if (server.args() == 0) return server.send(500, FPSTR(TEXT_PLAIN), PSTR("BAD ARGS"));
	path = server.arg(0);
	handleFileDeleteByName(path);
	path = String();
}

void handleFileCreate() {
	if (server.args() == 0)
		return server.send(500, FPSTR(TEXT_PLAIN), "BAD ARGS");
	String path = server.arg(0);
	//DBG_OUTPUT_PORT.println("handleFileCreate: " + path);
	if (path == "/")
		return server.send(500, FPSTR(TEXT_PLAIN), PSTR("BAD PATH"));
	if (SPIFFS.exists(path))
		return server.send(500, FPSTR(TEXT_PLAIN), PSTR("FILE EXISTS"));
	File file = SPIFFS.open(path, "w");
	if (file)
		file.close();
	else
		return server.send(500, FPSTR(TEXT_PLAIN), PSTR("CREATE FAILED"));
	server.send(200, FPSTR(TEXT_PLAIN), "");
	path = String();
};

void handleFileList() {

	String path = "/";
	if (server.hasArg("dir")) {
		//server.send(500,  FPSTR(TEXT_PLAIN), PSTR("BAD ARGS"));
		//return;
		path = server.arg("dir");
	}
#if defined(ESP8266)
	Dir dir = SPIFFS.openDir(path);
#else
	File root = SPIFFS.open(path);


	if (!root) {
		//DBG_OUTPUT_PORT.println("- failed to open directory");
		return;
	}
	if (!root.isDirectory()) {
		// DBG_OUTPUT_PORT.println(" - not a directory");
		return;
	}
#endif
	String output = "{\"success\":true, \"is_writable\" : true, \"results\" :[";
	bool firstrec = true;
#if !defined(ESP8266)
	File file = root.openNextFile();
	while (file) {
#else
	while (dir.next()) {
#endif
		if (!firstrec) { output += ','; }  //add comma between recs (not first rec)
		else {
			firstrec = false;
		}
#if !defined(ESP8266)
		//if (file.isDirectory())
		//	continue;
		String fn = file.name();
#else
		String fn = dir.fileName();
#endif
#if !defined(ESP8266)
		fn.remove(0, 1); //remove slash
		if (file.isDirectory()) {
			output += "{\"is_dir\":true";
		}
		else {
			output += "{\"is_dir\":false";
		}
#else

		fn.remove(0, 1); //remove slash
		output += "{\"is_dir\":false";
#endif
		output += ",\"name\":\"" + fn;
#if !defined(ESP8266)
		output += "\",\"size\":" + String(file.size());
#else
		output += "\",\"size\":" + String(dir.fileSize());
#endif
		output += ",\"path\":\"";
		output += "\",\"is_deleteable\":true";
		output += ",\"is_readable\":true";
		output += ",\"is_writable\":true";
		output += ",\"is_executable\":true";
		output += ",\"mtime\":1452813740";   //dummy time
		output += "}";
#if !defined(ESP8266)
		file = root.openNextFile();
#endif
	}
	output += "]}";
	
	server.send(200, "text/json", output);

};
void saveFile(const __FlashStringHelper* name, const __FlashStringHelper* content) {
	String data = FPSTR(content);
	String filename = name;
	if (!filename.startsWith("/")) filename = "/" + filename;
	//INFO("Save file %s", filename.c_str());
	//INFO("Save file content %s", data.substring(1,90).c_str());
	File fb = SPIFFS.open(filename, "w+");
	if (fb)
		fb.print(data);
	else
		ERROR("Save file failed %s", filename.c_str());
	if (fb)
		fb.close();
}
void saveFileBrowse() {
	//INFO("Save file browse");
	saveFile(FPSTR(FILE_BROWSE_FILE), FPSTR(FILE_BROWSE_HTML));
	/*
	  //DBG_OUTPUT_PORT.println("saveFileBrowse: " );
	  String data=FPSTR(FILE_BROWSE_HTML);
	  File fb = SPIFFS.open(FPSTR(FILE_BROWSE_FILE), "w");
	  if(fb)
		fb.print(data);
	  if(fb)
		fb.close();
	 // DBG_OUTPUT_PORT.println("saveFileBrowse: " );
	 */
};
void saveSetup() {
	saveFile(FPSTR(FILE_SETUP), FPSTR(SETUP_HTML));
};
void saveIndex() {
	if(INDEX_HTML_INTERNAL)
		saveFile(FPSTR(FILE_INDEX), FPSTR(INDEX_HTML_INTERNAL));

	/*
	  String data=FPSTR(INDEX_HTML);
	  File fb = SPIFFS.open(String("/")+String(FPSTR(FILE_INDEX)), "w");
	  if(fb)
		fb.print(data);
	  if(fb)
		fb.close();
	 */
};;

void handleFileBrowser()
{

	if (server.arg("do") == "list") {
		handleFileList();
	}
	else
		if (server.arg("do") == "delete") {
			handleFileDeleteByName(server.arg("file").c_str());
		}
		else
			if (server.arg("do") == "download") {
				handleFileDownload(server.arg("file").c_str());
			}
			else
			{
				SPIFFS.remove(FPSTR(FILE_BROWSE_FILE));
				if (!SPIFFS.exists(FPSTR(FILE_BROWSE_FILE))) {
					saveFileBrowse();
				}
				if (!handleFileRead(FPSTR(FILE_BROWSE_FILE))) { //send GZ version of embedded browser
										//server.sendHeader("Content-Encoding", "gzip");
										//server.send_P(200, "text/html", PAGE_FSBROWSE, sizeof(PAGE_FSBROWSE));

					server.send(500, FPSTR(TEXT_PLAIN), "handleFileBrowser can't proceed");
				}
				//MyWebServer.isDownloading = true; //need to stop all cloud services from doing anything!  crashes on upload with mqtt...
			}
}
void handleJsonSave()
{

	if (server.args() == 0)
		return server.send(500, FPSTR(TEXT_PLAIN), PSTR("BAD JsonSave ARGS"));

	String fname = "/" + server.arg(0);
	fname = urldecode(fname);

	//DBG_OUTPUT_PORT.println("handleJsonSave: " + fname);


	File file = SPIFFS.open(fname, "w");
	if (file) {
		file.println(server.arg(1));  //save json data
		file.close();
	}
	else  //cant create file
		return server.send(500, FPSTR(TEXT_PLAIN), PSTR("JSONSave FAILED"));
	server.send(200, FPSTR(TEXT_PLAIN), "");

}

void handleJsonLoad()
{

	if (server.args() == 0)
		return server.send(500, "text/plain", "BAD JsonLoad ARGS");
	String fname = "/" + server.arg(0);

	fname = urldecode(fname);
	//DBG_OUTPUT_PORT.println("handleJsonRead: " + fname);



	File file = SPIFFS.open(fname, "r");
	if (file) {
		server.streamFile(file, "application/json");
		file.close();
	}
}
void handleNotFound() {
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += (server.method() == HTTP_GET) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";
	for (uint8_t i = 0; i < server.args(); i++) {
		message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
	}
	server.send(404, "text/plain", message);
}

//OTA Update
void setupOta() {
#ifdef ENABLE_OTA
	server.on("/update", HTTP_GET, []() {
		server.sendHeader("Connection", "close");
		server.send(200, "text/html", PSTR("<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>"));
	});
	server.on("/update", HTTP_POST, []() {
		server.sendHeader("Connection", "close");
		server.send(200, FPSTR(TEXT_PLAIN), (Update.hasError()) ? "FAIL" : "OK");
		ESP.restart();
	}, []() {
		HTTPUpload& upload = server.upload();
		if (upload.status == UPLOAD_FILE_START) {
			Serial.setDebugOutput(true);
#ifdef ESP8266
			WiFiUDP::stopAll();
#endif      
			Serial.printf("Update: %s\n", upload.filename.c_str());
			uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
			if (!Update.begin(maxSketchSpace)) { //start with max available size
				Update.printError(Serial);
			}
		}
		else if (upload.status == UPLOAD_FILE_WRITE) {
			if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
				Update.printError(Serial);
			}
		}
		else if (upload.status == UPLOAD_FILE_END) {
			if (Update.end(true)) { //true to set the size to the current progress
				Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
			}
			else {
				Update.printError(Serial);
			}
			Serial.setDebugOutput(false);
		}
	});
#endif
}