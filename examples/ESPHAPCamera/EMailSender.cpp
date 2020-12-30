#include "EMailSender.h"

//#include <SPIFFS.h>
//#include <LittleFS.h>

//#define SD SPIFFS

// BASE64 -----------------------------------------------------------
const char PROGMEM b64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                    "abcdefghijklmnopqrstuvwxyz"
                                    "0123456789+/";

#define encode64(arr) encode64_f(arr,strlen(arr))

inline void a3_to_a4(unsigned char * a4, unsigned char * a3) {
  a4[0] = (a3[0] & 0xfc) >> 2;
  a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
  a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);
  a4[3] = (a3[2] & 0x3f);
}

int base64_encode(char *output, char *input, int inputLen) {
  int i = 0, j = 0;
  int encLen = 0;
  unsigned char a3[3];
  unsigned char a4[4];

  while (inputLen--) {
    a3[i++] = *(input++);
    if (i == 3) {
      a3_to_a4(a4, a3);

      for (i = 0; i < 4; i++) {
        output[encLen++] = pgm_read_byte(&b64_alphabet[a4[i]]);
      }

      i = 0;
    }
  }

  if (i) {
    for (j = i; j < 3; j++) {
      a3[j] = '\0';
    }

    a3_to_a4(a4, a3);

    for (j = 0; j < i + 1; j++) {
      output[encLen++] = pgm_read_byte(&b64_alphabet[a4[j]]);
    }

    while ((i++ < 3)) {
      output[encLen++] = '=';
    }
  }
  output[encLen] = '\0';
  return encLen;
}

int base64_enc_length(int plainLen) {
  int n = plainLen;
  return (n + 2 - ((n + 2) % 3)) / 3 * 4;
}

const char* encode64_f(char* input, uint8_t len) {
  // encoding

	DEBUG_PRINTLN(F("Encoding"));

	DEBUG_PRINTLN(input);
	DEBUG_PRINTLN(len);

  //int encodedLen =
 base64_enc_length(len);
  static char encoded[256];
  // note input is consumed in this step: it will be empty afterwards
  base64_encode(encoded, input, len);
  return encoded;
}

// END BASE64 ---------------------------------------------------------

EMailSender::EMailSender(const char* email_login, const char* email_password, const char* email_from, const char* name_from ,
		const char* smtp_server, uint16_t smtp_port) {
	this->setEMailLogin(email_login);
	this->setEMailFrom(email_from);
	this->setEMailPassword(email_password);
	this->setSMTPServer(smtp_server);
	this->setSMTPPort(smtp_port);
	this->setNameFrom(name_from);
//	this->isSecure = isSecure;
}
EMailSender::EMailSender(const char* email_login, const char* email_password, const char* email_from,
		const char* smtp_server, uint16_t smtp_port) {
	this->setEMailLogin(email_login);
	this->setEMailFrom(email_from);
	this->setEMailPassword(email_password);
	this->setSMTPServer(smtp_server);
	this->setSMTPPort(smtp_port);

//	this->isSecure = isSecure;
}

EMailSender::EMailSender(const char* email_login, const char* email_password, const char* email_from, const char* name_from ) {
	this->setEMailLogin(email_login);
	this->setEMailFrom(email_from);
	this->setEMailPassword(email_password);
	this->setNameFrom(name_from);
	this->setNameFrom(name_from);

//	this->isSecure = isSecure;
}
EMailSender::EMailSender(const char* email_login, const char* email_password, const char* email_from) {
	this->setEMailLogin(email_login);
	this->setEMailFrom(email_from);
	this->setEMailPassword(email_password);

//	this->isSecure = isSecure;
}

EMailSender::EMailSender(const char* email_login, const char* email_password){
	this->setEMailLogin(email_login);
	this->setEMailFrom(email_login);
	this->setEMailPassword(email_password);

//	this->isSecure = isSecure;
}

void EMailSender::setSMTPPort(uint16_t smtp_port){
	this->smtp_port = smtp_port;
};
void EMailSender::setSMTPServer(const char* smtp_server){
	delete [] this->smtp_server;
	size_t len = strlen(smtp_server) + 1;
	this->smtp_server = new char[len];
	memset(this->smtp_server, 0, len);
	strcpy(this->smtp_server, smtp_server);
};

void EMailSender::setEMailLogin(const char* email_login){
	delete [] this->email_login;
	size_t len = strlen(email_login) + 1;
	this->email_login = new char[len];
	memset(this->email_login, 0, len);
	strcpy(this->email_login, email_login);
};
void EMailSender::setEMailFrom(const char* email_from){
	delete [] this->email_from;
	size_t len = strlen(email_from) + 1;
	this->email_from = new char[len];
	memset(this->email_from, 0, len);
	strcpy(this->email_from, email_from);
};
void EMailSender::setNameFrom(const char* name_from){
	delete [] this->name_from;
	size_t len = strlen(name_from) + 1;
	this->name_from = new char[len];
	memset(this->name_from, 0, len);
	strcpy(this->name_from, name_from);
};
void EMailSender::setEMailPassword(const char* email_password){
	delete [] this->email_password;
	size_t len = strlen(email_password) + 1;
	this->email_password = new char[len];
	memset(this->email_password, 0, len);
	strcpy(this->email_password, email_password);
};

void EMailSender::setIsSecure(bool isSecure) {
	this->isSecure = isSecure;
}

EMailSender::Response EMailSender::awaitSMTPResponse(EMAIL_NETWORK_CLASS &client,
		const char* resp, const char* respDesc, uint16_t timeOut) {
	EMailSender::Response response;
	uint32_t ts = millis();
	while (!client.available()) {
		if (millis() > (ts + timeOut)) {
			response.code = F("1");
			response.desc = F("SMTP Response TIMEOUT!");
			response.status = false;
			return response;
		}
	}
	_serverResponce = client.readStringUntil('\n');

	DEBUG_PRINTLN(_serverResponce);
	if (resp && _serverResponce.indexOf(resp) == -1){
		response.code = resp;
		response.desc = respDesc +String(" (") + _serverResponce + String(")");
		response.status = false;
		return response;
	}

	response.status = true;
	return response;
}

static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
void encodeblock(unsigned char in[3],unsigned char out[4],int len) {
 out[0]=cb64[in[0]>>2]; out[1]=cb64[((in[0]&0x03)<<4)|((in[1]&0xF0)>>4)];
 out[2]=(unsigned char) (len>1 ? cb64[((in[1]&0x0F)<<2)|((in[2]&0xC0)>>6)] : '=');
 out[3]=(unsigned char) (len>2 ? cb64[in[2]&0x3F] : '=');
}

#if (defined(STORAGE_SPIFFS_ENABLED) && defined(FS_NO_GLOBALS))
		void encode(fs::File *file, EMAIL_NETWORK_CLASS *client) {
		 unsigned char in[3],out[4];
		 int i,len,blocksout=0;

		 while (file->available()!=0) {
		   len=0;
			 for (i=0;i<3;i++){
				   in[i]=(unsigned char) file->read();
					   if (file->available()!=0) len++;
							 else in[i]=0;
			 }
			 if (len){
				 encodeblock(in,out,len);
		//         for(i=0;i<4;i++) client->write(out[i]);
				 client->write(out, 4);
				 blocksout++; }
			 if (blocksout>=19||file->available()==0){
				 if (blocksout) {
					 client->print("\r\n");
				 }
				 blocksout=0;
			 }
		  }
		}
#endif

#if (defined(STORAGE_SD_ENABLED) || (defined(STORAGE_SPIFFS_ENABLED) && !defined(FS_NO_GLOBALS)))
void encode(File *file, EMAIL_NETWORK_CLASS *client) {
 unsigned char in[3],out[4];
 int i,len,blocksout=0;

 while (file->available()!=0) {
   len=0;
     for (i=0;i<3;i++){
           in[i]=(unsigned char) file->read();
               if (file->available()!=0) len++;
                     else in[i]=0;
     }
     if (len){
         encodeblock(in,out,len);
//         for(i=0;i<4;i++) client->write(out[i]);
         client->write(out, 4);
         blocksout++; }
     if (blocksout>=19||file->available()==0){
         if (blocksout) {
        	 client->print("\r\n");
         }
         blocksout=0;
     }
  }
}
#endif
#if (defined(STORAGE_RAW_ENABLED) )
void encode(uint8_t* data,uint16_t datalen, EMAIL_NETWORK_CLASS *client) {
	unsigned char in[3], out[4];
	int i, len, blocksout = 0;
	int position = 0;
	while (position< datalen) {
		len = 0;
		for (i = 0; i < 3; i++) {
			in[i] = (unsigned char)data[position];
			position++;
			if (position < datalen) len++;
			else in[i] = 0;
		}
		if (len) {
			encodeblock(in, out, len);
			//         for(i=0;i<4;i++) client->write(out[i]);
			client->write(out, 4);
			blocksout++;
		}
		if (blocksout >= 19 || position >= datalen == 0) {
			if (blocksout) {
				client->print("\r\n");
			}
			blocksout = 0;
		}
	}
}
#endif
const char** toCharArray(String arr[], int num) {
    // If we ever alloc with new with have to delete
    const char** buffer = new const char*[num];

    for(int i = 0; i < num; i++) {
        buffer[i] = arr[i].c_str();
    }

    return buffer;
}
const char** toCharArray(char* arr[], int num) {
    // If we ever alloc with new with have to delete
    const char** buffer = new const char*[num];

    for(int i = 0; i < num; i++) {
        buffer[i] = arr[i];
    }

    return buffer;
}

EMailSender::Response EMailSender::send(char* tos[], byte sizeOfTo, EMailMessage &email, Attachments attachments) {
	return send(toCharArray(tos, sizeOfTo), sizeOfTo, 0, 0, email, attachments);
}
EMailSender::Response EMailSender::send(char* tos[], byte sizeOfTo,  byte sizeOfCc,  EMailMessage &email, Attachments attachments) {
	return send(toCharArray(tos, sizeOfTo+sizeOfCc), sizeOfTo, sizeOfCc, 0, email, attachments);
}
EMailSender::Response EMailSender::send(char* tos[], byte sizeOfTo,  byte sizeOfCc,byte sizeOfCCn, EMailMessage &email, Attachments attachments){
	return send(toCharArray(tos, sizeOfTo+sizeOfCc+sizeOfCCn), sizeOfTo, sizeOfCc, sizeOfCCn, email, attachments);
}


EMailSender::Response EMailSender::send(String to, EMailMessage &email, Attachments attachments){
	  DEBUG_PRINT(F("ONLY ONE RECIPIENT"));

	const char* arrEmail[] =  {to.c_str()};
	return send(arrEmail, 1, email, attachments);
}

EMailSender::Response EMailSender::send(String tos[], byte sizeOfTo, EMailMessage &email, Attachments attachments) {
	return send(toCharArray(tos, sizeOfTo), sizeOfTo, 0, 0, email, attachments);
}

EMailSender::Response EMailSender::send(String tos[], byte sizeOfTo,  byte sizeOfCc,  EMailMessage &email, Attachments attachments) {
	return send(toCharArray(tos, sizeOfTo+sizeOfCc), sizeOfTo, sizeOfCc, 0, email, attachments);
}

EMailSender::Response EMailSender::send(String tos[], byte sizeOfTo,  byte sizeOfCc,byte sizeOfCCn, EMailMessage &email, Attachments attachments){
	return send(toCharArray(tos, sizeOfTo+sizeOfCc+sizeOfCCn), sizeOfTo, sizeOfCc, sizeOfCCn, email, attachments);
}

EMailSender::Response EMailSender::send(const char* to, EMailMessage &email, Attachments attachments){
	  DEBUG_PRINT(F("ONLY ONE RECIPIENT"));

	const char* arrEmail[] =  {to};
	return send(arrEmail, 1, email, attachments);
}

EMailSender::Response EMailSender::send(const char* to[], byte sizeOfTo, EMailMessage &email, Attachments attachments) {
	return send(to, sizeOfTo, 0, email, attachments);
}

EMailSender::Response EMailSender::send(const char* to[], byte sizeOfTo,  byte sizeOfCc,  EMailMessage &email, Attachments attachments)
{
	return send(to, sizeOfTo, sizeOfCc, 0, email, attachments);
}

EMailSender::Response EMailSender::send(const char* to[], byte sizeOfTo,  byte sizeOfCc,byte sizeOfCCn, EMailMessage &email, Attachments attachments)
{

	EMAIL_NETWORK_CLASS client;
//	SSLClient client(base_client, TAs, (size_t)TAs_NUM, A5);

  DEBUG_PRINT(F("Insecure client:"));
  DEBUG_PRINTLN(this->isSecure);

#if (EMAIL_NETWORK_TYPE == NETWORK_ESP8266 || EMAIL_NETWORK_TYPE == NETWORK_ESP8266_242)
	#ifndef ARDUINO_ESP8266_RELEASE_2_4_2
	  if (this->isSecure == false){
		  client.setInsecure();
		  bool mfln = client.probeMaxFragmentLength(this->smtp_server, this->smtp_port, 512);

		  DEBUG_PRINT("MFLN supported: ");
		  DEBUG_PRINTLN(mfln?"yes":"no");

		  if (mfln) {
			  client.setBufferSizes(512, 512);
		  }
	  }
	#endif
#endif

  EMailSender::Response response;

  if(!client.connect(this->smtp_server, this->smtp_port)) {
	  response.desc = F("Could not connect to mail server");
	  response.code = F("2");
	  response.status = false;

	  return response;
  }
  Serial.println("Connected");

  response = awaitSMTPResponse(client, "220", "Connection Error");
  if (!response.status) return response;

  String helo = "HELO "+String(publicIPDescriptor)+": ";
  DEBUG_PRINTLN(helo);
  client.println(helo);

  response = awaitSMTPResponse(client, "250", "Identification error");
  if (!response.status) return response;

  if (useAuth){
	  DEBUG_PRINTLN(F("AUTH LOGIN:"));
	  client.println(F("AUTH LOGIN"));
	  awaitSMTPResponse(client);

	  DEBUG_PRINTLN(encode64(this->email_login));
	  client.println(encode64(this->email_login));
	  awaitSMTPResponse(client);

	  DEBUG_PRINTLN(encode64(this->email_password));
	  client.println(encode64(this->email_password));

	  response = awaitSMTPResponse(client, "235", "SMTP AUTH error");
	  if (!response.status) return response;
  }
  DEBUG_PRINT(F("MAIL FROM: <"));
  DEBUG_PRINT(this->email_from);
  DEBUG_PRINTLN(F(">"));

  client.print(F("MAIL FROM: <"));
  client.print(this->email_from);
  client.println(F(">"));
  awaitSMTPResponse(client);

//  String rcpt = "RCPT TO: <" + String(to) + '>';
//
//  DEBUG_PRINTLN(rcpt);
//  client.println(rcpt);

  int cont;
  for (cont=0;cont<(sizeOfTo+sizeOfCc+sizeOfCCn);cont++){
	  DEBUG_PRINT(F("RCPT TO: <"));
	  DEBUG_PRINT(to[cont]);
	  DEBUG_PRINTLN(F(">"));

	  client.print(F("RCPT TO: <"));
	  client.print(to[cont]);
	  client.println(F(">"));
	  awaitSMTPResponse(client);
  }

  DEBUG_PRINTLN(F("DATA:"));
  client.println(F("DATA"));

  response = awaitSMTPResponse(client, "354", "SMTP DATA error");
  if (!response.status) return response;

//  client.println("From: <" + String(this->email_from) + '>');

  client.print(F("From: "));
  if (this->name_from){
	  client.print(this->name_from);
  }
  client.print(F(" <"));
  client.print(this->email_from);
  client.println(F(">"));

//  client.println("To: <" + String(to) + '>');

  client.print(F("To: "));
  for (cont=0;cont<sizeOfTo;cont++){
	  client.print(F("<"));
	  client.print(to[cont]);
	  client.print(">");
	  if (cont!=sizeOfTo-1){
		  client.print(",");
	  }
  }
  client.println();

  if (sizeOfCc>0){
	  client.print(F("Cc: "));
	  for (;cont<sizeOfTo+sizeOfCc;cont++){
		  client.print(F("<"));
		  client.print(to[cont]);
		  client.print(">");
		  if (cont!=sizeOfCc-1){
			  client.print(",");
		  }
	  }
	  client.println();
  }

  if (sizeOfCCn>0){
	  client.print(F("CCn: "));
	  for (;cont<sizeOfTo+sizeOfCc+sizeOfCCn;cont++){
		  client.print(F("<"));
		  client.print(to[cont]);
		  client.print(">");
		  if (cont!=sizeOfCCn-1){
			  client.print(",");
		  }
	  }
	  client.println();
  }

  client.print(F("Subject: "));
  client.println(email.subject);

//  client.println(F("Mime-Version: 1.0"));

  client.println(F("MIME-Version: 1.0"));
  client.println(F("Content-Type: Multipart/mixed; boundary=frontier"));

  client.println(F("--frontier"));

    client.print(F("Content-Type: "));
    client.print(email.mime);
    client.println(F("; charset=\"UTF-8\""));
//  client.println(F("Content-Type: text/html; charset=\"UTF-8\""));
  client.println(F("Content-Transfer-Encoding: 7bit"));
  client.println();
  if (email.mime==F("text/html")){
//	  String body = "<!DOCTYPE html><html lang=\"en\">" + String(email.message) + "</html>";

	  client.print(F("<!DOCTYPE html><html lang=\"en\">"));
	  client.print(email.message);
	  client.println(F("</html>"));

//	  client.println(body);
  }else{
	  client.println(email.message);
  }
  client.println();

#ifdef STORAGE_SPIFFS_ENABLED
  bool spiffsActive = false;
#endif
#ifdef STORAGE_SD_ENABLED
  bool sdActive = false;
#endif

#if defined(ENABLE_ATTACHMENTS) && (defined(STORAGE_SD_ENABLED) || defined(STORAGE_SPIFFS_ENABLED) || defined(STORAGE_RAW_ENABLED))
//  if ((sizeof(attachs) / sizeof(attachs[0]))>0){
  if (sizeof(attachments)>0 && attachments.number>0){

	  DEBUG_PRINT(F("Array: "));
//	  for (int i = 0; i<(sizeof(attachs) / sizeof(attachs[0])); i++){
	  for (int i = 0; i<attachments.number; i++){
		  uint8_t tBuf[64];

		  DEBUG_PRINTLN(attachments.fileDescriptor[i].filename);
		  client.println(F("--frontier"));
		  client.print(F("Content-Type: "));
		  client.print(attachments.fileDescriptor[i].mime);
		  client.println(F("; charset=\"UTF-8\""));

		  if (attachments.fileDescriptor[i].encode64){
			  client.println(F("Content-Transfer-Encoding: base64"));
		  }

		  client.print(F("Content-Disposition: attachment; filename="));
		  client.print(attachments.fileDescriptor[i].filename);
		  client.println("\n");

			int clientCount = 0;
#ifdef STORAGE_RAW_ENABLED
			if (attachments.fileDescriptor[i].storageType == EMAIL_STORAGE_TYPE_ARRAY) {
				if (attachments.fileDescriptor[i].filedata) {
					if (attachments.fileDescriptor[i].encode64) {
						encode(attachments.fileDescriptor[i].filedata, attachments.fileDescriptor[i].filedatalen, &client);
						//DEBUG_PRINTLN("base64 not supported in raw mode");
					}
					else {
						uint16_t position = 0;
						while (position< attachments.fileDescriptor[i].filedatalen) {
							clientCount = ((position + RAW_BUFF_CHUNK) > attachments.fileDescriptor[i].filedatalen)? 
								(attachments.fileDescriptor[i].filedatalen-position) : RAW_BUFF_CHUNK;
							
							client.write((byte*)(attachments.fileDescriptor[i].filedata+position), clientCount);
							position += clientCount;
						}
					}
					

					client.println();
				}
			}
#endif
#ifdef STORAGE_SPIFFS_ENABLED
			if (attachments.fileDescriptor[i].storageType==EMAIL_STORAGE_TYPE_SPIFFS){
#ifdef OPEN_CLOSE_SPIFFS
				if (!SPIFFS.exists(attachments.fileDescriptor[i].url)){
					if(!SPIFFS.begin()){
						  EMailSender::Response response;
						  response.code = F("500");
						  response.desc = F("Error on startup SPIFFS filesystem!");
						  response.status = false;
						  return response;
					}

					spiffsActive = true;
					DEBUG_PRINTLN("SPIFFS BEGIN, ACTIVE");
				}
#endif

				fs::File myFile = SPIFFS.open(attachments.fileDescriptor[i].url, "r");
				  if(myFile) {
					  if (attachments.fileDescriptor[i].encode64){
						  encode(&myFile, &client);
					  }else{
						while(myFile.available()) {
						clientCount = myFile.read(tBuf,64);
						  client.write((byte*)tBuf,clientCount);
						}
					  }
					myFile.close();

					client.println();
				  }
				  else {
					  EMailSender::Response response;
					  response.code = F("404");
					  response.desc = "Error opening attachments file "+attachments.fileDescriptor[i].url;
					  response.status = false;
					  return response;
				  }

			}
#endif
#ifdef STORAGE_SD_ENABLED
			if (attachments.fileDescriptor[i].storageType==EMAIL_STORAGE_TYPE_SD){
//				File myFile = SD.open(attachments.fileDescriptor[i].url, "r");
//				  if(myFile) {
//					while(myFile.available()) {
//					clientCount = myFile.read(tBuf,64);
//					  client.write((byte*)tBuf,clientCount);
//					}
//					myFile.close();
//				  }
//				  else {
//					  EMailSender::Response response;
//					  response.code = "404";
//					  response.desc = "Error opening attachments file "+attachments.fileDescriptor[i].url;
//					  response.status = false;
//					  return response;
//				  }
#ifdef OPEN_CLOSE_SD
				 DEBUG_PRINTLN(F("SD Check"));
				 if (!SD.exists(attachments.fileDescriptor[i].url.c_str())){
					if(!SD.begin(4)){
						  response.code = F("500");
						  response.desc = F("Error on startup SD filesystem!");
						  response.status = false;
						  return response;
					}
					sdActive = true;
				 }
#endif

			    DEBUG_PRINTLN(F("Open file: "));
			File myFile = SD.open(attachments.fileDescriptor[i].url.c_str());
			  if(myFile) {
				  myFile.seek(0);
				  DEBUG_PRINTLN(F("OK"));
				  if (attachments.fileDescriptor[i].encode64){
					  DEBUG_PRINTLN(F("BASE 64"));
					  encode(&myFile, &client);
				  }else{
					  DEBUG_PRINTLN(F("NORMAL"));
					while(myFile.available()) {
						clientCount = myFile.read(tBuf,64);
						client.write((byte*)tBuf,clientCount);
					}
				  }
				myFile.close();

				client.println();
			  }
			  else {
				  response.code = F("404");
				  response.desc = "Error opening attachments file "+attachments.fileDescriptor[i].url;
				  response.status = false;
				  return response;
			  }

			}
#endif

	  }
	  client.println();
	  client.println(F("--frontier--"));
#ifdef STORAGE_SD_ENABLED
	  #ifdef OPEN_CLOSE_SD
		  if (sdActive){
			  DEBUG_PRINTLN(F("SD end"));
#ifndef ARDUINO_ESP8266_RELEASE_2_4_2
			  SD.end();
#endif
			  DEBUG_PRINTLN(F("SD end 2"));
		  }
	#endif
#endif

#ifdef STORAGE_SPIFFS_ENABLED
	#ifdef OPEN_CLOSE_SPIFFS
		  if (spiffsActive){
			  SPIFFS.end();
			  DEBUG_PRINTLN(F("SPIFFS END"));
		  }
	#endif
#endif

  }

#endif

  DEBUG_PRINTLN(F("Message end"));
  client.println(F("."));

  response = awaitSMTPResponse(client, "250", "Sending message error");
  if (!response.status) return response;

  client.println(F("QUIT"));

  response = awaitSMTPResponse(client, "221", "SMTP QUIT error");
  if (!response.status) return response;

  response.status = true;
  response.code = F("0");
  response.desc = F("Message sent!");

  return response;
}

