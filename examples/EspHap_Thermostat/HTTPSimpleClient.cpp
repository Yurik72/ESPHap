#include <HardwareSerial.h>
//#define DEBUG_HTTPCLIENT

#include <Arduino.h>

#ifdef HTTPCLIENT_1_1_COMPATIBLE
#include <WiFi.h>
#include <WiFiClientSecure.h>
#endif

#include <StreamString.h>
#include <FS.h>
#if !defined(ESP8266)
#include <SPIFFS.h>
#endif

#include "HTTPSimpleClient.h"


#define DEBUG_HTTPCLIENT
/**
 * constructor
 */
HTTPSimpleClient::HTTPSimpleClient()
{
	_client = NULL;
	userHeaders = "";
}

/**
 * destructor
 */
HTTPSimpleClient::~HTTPSimpleClient()
{
	if (_client) {
		_client->stop();
	}

}

void HTTPSimpleClient::clear()
{
	_returnCode = 0;
	_size = -1;
	
}




bool HTTPSimpleClient::begin(String url)
{
#ifdef DEBUG_HTTPCLIENT
	DBG_OUTPUT_PORT.println("[HTTP] Begin " + String(url));
#endif
	clear();

	// check for : (http: or https:
	int index = url.indexOf(':');
	if (index < 0) {
		//log_e("failed to parse protocol");
		return false;
	}

	_protocol = url.substring(0, index);

	url.remove(0, (index + 3)); // remove http:// or https://

	index = url.indexOf('/');
	String host = url.substring(0, index);
	url.remove(0, index); // remove host part



	// get port
	index = host.indexOf(':');
	if (index >= 0) {
		_host = host.substring(0, index); // hostname
		host.remove(0, (index + 1)); // remove hostname + :
		_port = host.toInt(); // get port
	}
	else {
		_host = host;
		_port = (_protocol == "https" ? 443 : 80);
	}
	_uri = url;
	_secure = _protocol == "https";

	DBG_OUTPUT_PORT.println("http begin start connection "+String(_host));
	return connect();
}


/**
 * end
 * called after the payload is handled
 */
void HTTPSimpleClient::end(void)
{
	disconnect(false);
	clear();
}



/**
 * disconnect
 * close the TCP socket
 */
void HTTPSimpleClient::disconnect(bool preserveClient)
{
	if (connected()) {
		if (_client->available() > 0) {
			//log_d("still data in buffer (%d), clean up.\n", _client->available());
			while (_client->available() > 0) {
				_client->read();
			}
		}
	}

	_client->stop();
}


/**
 * connected
 * @return connected status
 */
bool HTTPSimpleClient::connected()
{

	if (_client) {
		return ((_client->available() > 0) || _client->connected());
	}
	return false;
}




/**
 * send a GET request
 * @return http code
 */
int HTTPSimpleClient::GET()
{
	return sendRequest("GET");
}


int HTTPSimpleClient::POST(uint8_t * payload, size_t size)
{
	return sendRequest("POST", payload, size);
}

int HTTPSimpleClient::POST(String payload)
{
	return POST((uint8_t *)payload.c_str(), payload.length());
}

/**
 * sends a patch request to the server
 * @param payload uint8_t *
 * @param size size_t
 * @return http code
 */
int HTTPSimpleClient::PATCH(uint8_t * payload, size_t size)
{
	return sendRequest("PATCH", payload, size);
}

int HTTPSimpleClient::PATCH(String payload)
{
	return PATCH((uint8_t *)payload.c_str(), payload.length());
}

/**
 * sends a put request to the server
 * @param payload uint8_t *
 * @param size size_t
 * @return http code
 */
int HTTPSimpleClient::PUT(uint8_t * payload, size_t size) {
	return sendRequest("PUT", payload, size);
}

int HTTPSimpleClient::PUT(String payload) {
	return PUT((uint8_t *)payload.c_str(), payload.length());
}



/**
 * sendRequest
 * @param type const char *     "GET", "POST", ....
 * @param payload uint8_t *     data for the message body if null not send
 * @param size size_t           size for the message body if 0 not send
 * @return -1 if no info or > 0 when Content-Length is set by server
 */
int HTTPSimpleClient::sendRequest(const char * type, uint8_t * payload, size_t size)
{

	if (payload && size > 0) {
		addHeader(F("Content-Length"), String(payload && size > 0 ? size : 0));
	}
	// send Header
	if (!sendHeader(type)) {
		return returnError(HTTPC_ERROR_SEND_HEADER_FAILED);
	}
    if (payload && size > 0) {
            size_t bytesWritten = 0;
            const uint8_t *p = payload;
            size_t originalSize = size;
            while (bytesWritten < originalSize) {
                int written;
                int towrite = std::min((int)size, (int)HTTP_TCP_BUFFER_SIZE);
                written = _client->write(p + bytesWritten, towrite);
                if (written < 0) {
                        return returnError(HTTPC_ERROR_SEND_PAYLOAD_FAILED);
                } else if (written == 0) {
                        return returnError(HTTPC_ERROR_CONNECTION_LOST);
                }
                bytesWritten += written;
                size -= written;
            }
      }

	// handle Server Response (Header)
	return returnError(handleHeaderResponse());
}





/**
 * size of message body / payload
 * @return -1 if no info or > 0 when Content-Length is set by server
 */
int HTTPSimpleClient::getSize(void)
{
	return _size;
}

/**
 * returns the stream of the tcp connection
 * @return WiFiClient
 */
WiFiClient& HTTPSimpleClient::getStream(void)
{
	if (connected()) {
		return *_client;
	}

	
	static WiFiClient empty;
	return empty;
}

/**
 * returns a pointer to the stream of the tcp connection
 * @return WiFiClient*
 */
WiFiClient* HTTPSimpleClient::getStreamPtr(void)
{
	if (connected()) {
		return _client;
	}

	//log_w("getStreamPtr: not connected");
	return nullptr;
}

/**
 * write all  message body / payload to Stream
 * @param stream Stream *
 * @return bytes written ( negative values are error codes )
 */
int HTTPSimpleClient::writeToStream(Stream * stream)
{

	if (!stream) {
		return returnError(HTTPC_ERROR_NO_STREAM);
	}

	if (!connected()) {
  #ifdef DEBUG_HTTPCLIENT
  DBG_OUTPUT_PORT.println("Write to stream -> not connected");
#endif
		return returnError(HTTPC_ERROR_NOT_CONNECTED);
	}

	// get length of document (is -1 when Server sends no Content-Length header)
	int len = _size;
	int ret = 0;

	if (_transferEncoding == HTTPC_TE_IDENTITY) {
		ret = writeToStreamDataBlock(stream, len);

		// have we an error?
		if (ret < 0) {
#ifdef DEBUG_HTTPCLIENT
  DBG_OUTPUT_PORT.println("Write to stream -> ret < 0");
#endif
			return returnError(ret);
		}
	}
	else if (_transferEncoding == HTTPC_TE_CHUNKED) {
		int size = 0;
		while (1) {
			if (!connected()) {
#ifdef DEBUG_HTTPCLIENT
  DBG_OUTPUT_PORT.println("Write to stream -> CONNECTION_LOST");
#endif
				return returnError(HTTPC_ERROR_CONNECTION_LOST);
			}
			String chunkHeader = _client->readStringUntil('\n');

			if (chunkHeader.length() <= 0) {
        #ifdef DEBUG_HTTPCLIENT
  DBG_OUTPUT_PORT.println("Write to stream -> READ_TIMEOUT");
#endif
				return returnError(HTTPC_ERROR_READ_TIMEOUT);
			}

			chunkHeader.trim(); // remove \r

			// read size of chunk
			len = (uint32_t)strtol((const char *)chunkHeader.c_str(), NULL, 16);
			size += len;
			//log_d(" read chunk len: %d", len);
#ifdef DEBUG_HTTPCLIENT
  DBG_OUTPUT_PORT.print("Write to stream -> read chunk len");
  DBG_OUTPUT_PORT.println(len);
#endif
			// data left?
			if (len > 0) {
				int r = writeToStreamDataBlock(stream, len);
				if (r < 0) {
					// error in writeToStreamDataBlock
#ifdef DEBUG_HTTPCLIENT
  DBG_OUTPUT_PORT.println("Write to stream ->  error in writeToStreamDataBlock");
 
#endif
					return returnError(r);
				}
				ret += r;
			}
			else {

				// if no length Header use global chunk size
				if (_size <= 0) {
					_size = size;
				}

				// check if we have write all data out
				if (ret != _size) {
#ifdef DEBUG_HTTPCLIENT
  DBG_OUTPUT_PORT.println("Write to stream ->  ERROR_STREAM_WRITE");
#endif
					return returnError(HTTPC_ERROR_STREAM_WRITE);
				}
				break;
			}

			// read trailing \r\n at the end of the chunk
			char buf[2];
			auto trailing_seq_len = _client->readBytes((uint8_t*)buf, 2);
			if (trailing_seq_len != 2 || buf[0] != '\r' || buf[1] != '\n') {
#ifdef DEBUG_HTTPCLIENT
  DBG_OUTPUT_PORT.println("Write to stream ->  ERROR_READ_TIMEOUT");
#endif
				return returnError(HTTPC_ERROR_READ_TIMEOUT);
			}

			delay(0);
		}
	}
	else {
#ifdef DEBUG_HTTPCLIENT
  DBG_OUTPUT_PORT.println("Write to stream ->  HTTPC_ERROR_ENCODING");
#endif
		return returnError(HTTPC_ERROR_ENCODING);
	}

	//    end();
	disconnect(true);
	return ret;
}

/**
 * return all payload as String (may need lot of ram or trigger out of memory!)
 * @return String
 */
String HTTPSimpleClient::getString(void)
{
	StreamString sstring;

	if (_size) {
		// try to reserve needed memmory
		if (!sstring.reserve((_size + 1))) {
			//log_d("not enough memory to reserve a string! need: %d", (_size + 1));
#ifdef DEBUG_HTTPCLIENT
  DBG_OUTPUT_PORT.println("not enough memory to reserve a string!");
#endif
			return "";
		}
	}

	writeToStream(&sstring);
	return sstring;
}


String HTTPSimpleClient::errorToString(int error)
{
	switch (error) {
	case HTTPC_ERROR_CONNECTION_REFUSED:
		return F("connection refused");
	case HTTPC_ERROR_SEND_HEADER_FAILED:
		return F("send header failed");
	case HTTPC_ERROR_SEND_PAYLOAD_FAILED:
		return F("send payload failed");
	case HTTPC_ERROR_NOT_CONNECTED:
		return F("not connected");
	case HTTPC_ERROR_CONNECTION_LOST:
		return F("connection lost");
	case HTTPC_ERROR_NO_STREAM:
		return F("no stream");
	case HTTPC_ERROR_NO_HTTP_SERVER:
		return F("no HTTP server");
	case HTTPC_ERROR_TOO_LESS_RAM:
		return F("too less ram");
	case HTTPC_ERROR_ENCODING:
		return F("Transfer-Encoding not supported");
	case HTTPC_ERROR_STREAM_WRITE:
		return F("Stream write error");
	case HTTPC_ERROR_READ_TIMEOUT:
		return F("read Timeout");
	default:
		return String();
	}
}

/**
 * adds Header to the request
 * @param name
 * @param value
 * @param first
 */



bool HTTPSimpleClient::connect(void)
{
	if (connected()) {

		while (_client->available() > 0) {
			_client->read();
		}
		return true;
	}

	if (_secure) {
		_client = new WiFiClientSecure();
#ifdef ESP8266
		((WiFiClientSecure*)_client)->setInsecure();

#endif
	}
	else {
		_client = new WiFiClient();

	}

#ifdef DEBUG_HTTPCLIENT
	DBG_OUTPUT_PORT.println("[HTTP] Start connection " + String(_host) +String(_port));
#endif
	_client->setTimeout(_connectTimeout);
	if (!_client->connect(_host.c_str(), _port)) {
		DBG_OUTPUT_PORT.println("WiFi Client not able to connect");
		return false;
	}

	// set Timeout for WiFiClient and for Stream::readBytesUntil() and Stream::readStringUntil()
	_client->setTimeout((_tcpTimeout + 500) / 1000);


	DBG_OUTPUT_PORT.println(F("http finish connection"));
	return connected();
}

/**
 * sends HTTP request header
 * @param type (GET, POST, ...)
 * @return status
 */
bool HTTPSimpleClient::sendHeader(const char * type)
{
	if (!connected()) {
		return false;
	}

	String header = String(type) + " " + _uri + F(" HTTP/1.");

	if (_useHTTP10) {
		header += "0";
	}
	else {
		header += "1";
	}

	header += String(F("\r\nHost: ")) + _host;
	if (_port != 80 && _port != 443)
	{
		header += ':';
		header += String(_port);
	}
	header += String(F("\r\nUser-Agent: Xclient")) + F("\r\nConnection: ");


	header += F("close");
	header += "\r\n";

	if (!_useHTTP10) {
		header += F("Accept-Encoding: identity;q=1,chunked;q=0.1,*;q=0\r\n");
	}

	

	header += "\r\n";

	header += userHeaders;

	return (_client->write((const uint8_t *)header.c_str(), header.length()) == header.length());
}

/**
 * reads the response from the server
 * @return int http code
 */
int HTTPSimpleClient::handleHeaderResponse()
{

	if (!connected()) {
		return HTTPC_ERROR_NOT_CONNECTED;
	}

	clear();



	String transferEncoding;

	_transferEncoding = HTTPC_TE_IDENTITY;
	unsigned long lastDataTime = millis();

	while (connected()) {
		size_t len = _client->available();
		if (len > 0) {
			String headerLine = _client->readStringUntil('\n');
			headerLine.trim(); // remove \r

			lastDataTime = millis();

			//log_v("RX: '%s'", headerLine.c_str());

			if (headerLine.startsWith("HTTP/1.")) {


				_returnCode = headerLine.substring(9, headerLine.indexOf(' ', 9)).toInt();
			}
			else if (headerLine.indexOf(':')) {
				String headerName = headerLine.substring(0, headerLine.indexOf(':'));
				String headerValue = headerLine.substring(headerLine.indexOf(':') + 1);
				headerValue.trim();

				if (headerName.equalsIgnoreCase("Content-Length")) {
					_size = headerValue.toInt();
				}


				if (headerName.equalsIgnoreCase("Transfer-Encoding")) {
					transferEncoding = headerValue;
				}


			}

			if (headerLine == "") {
				//log_d("code: %d", _returnCode);

				if (_size > 0) {
					//log_d("size: %d", _size);
				}

				if (transferEncoding.length() > 0) {
					//log_d("Transfer-Encoding: %s", transferEncoding.c_str());
					if (transferEncoding.equalsIgnoreCase("chunked")) {
						_transferEncoding = HTTPC_TE_CHUNKED;
					}
					else {
						return HTTPC_ERROR_ENCODING;
					}
				}
				else {
					_transferEncoding = HTTPC_TE_IDENTITY;
				}

				if (_returnCode) {
					return _returnCode;
				}
				else {
					//log_d("Remote host is not an HTTP Server!");
					return HTTPC_ERROR_NO_HTTP_SERVER;
				}
			}

		}
		else {
			if ((millis() - lastDataTime) > _tcpTimeout) {
				return HTTPC_ERROR_READ_TIMEOUT;
			}
			delay(10);
		}
	}

	return HTTPC_ERROR_CONNECTION_LOST;
}
void HTTPSimpleClient::addHeader(const String& name, const String& value){
	userHeaders += name;
	userHeaders += ": ";
	userHeaders += value;
	userHeaders += "\r\n";

}
/**
 * write one Data Block to Stream
 * @param stream Stream *
 * @param size int
 * @return < 0 = error >= 0 = size written
 */
int HTTPSimpleClient::writeToStreamDataBlock(Stream * stream, int size)
{
	int buff_size = HTTP_TCP_BUFFER_SIZE;
	int len = size;
	int bytesWritten = 0;

	// if possible create smaller buffer then HTTP_TCP_BUFFER_SIZE
	if ((len > 0) && (len < HTTP_TCP_BUFFER_SIZE)) {
		buff_size = len;
	}

	// create buffer for read
	uint8_t * buff = (uint8_t *)malloc(buff_size);

	if (buff) {
		// read all data from server
		while (connected() && (len > 0 || len == -1)) {

			// get available data size
			size_t sizeAvailable = _client->available();

			if (sizeAvailable) {

				int readBytes = sizeAvailable;

				// read only the asked bytes
				if (len > 0 && readBytes > len) {
					readBytes = len;
				}

				// not read more the buffer can handle
				if (readBytes > buff_size) {
					readBytes = buff_size;
				}

				// stop if no more reading    
				if (readBytes == 0)
					break;

				// read data
				int bytesRead = _client->readBytes(buff, readBytes);

				// write it to Stream
				int bytesWrite = stream->write(buff, bytesRead);
				bytesWritten += bytesWrite;

				// are all Bytes a writen to stream ?
				if (bytesWrite != bytesRead) {
					//log_d("short write asked for %d but got %d retry...", bytesRead, bytesWrite);

					// check for write error
					if (stream->getWriteError()) {
						//log_d("stream write error %d", stream->getWriteError());

						//reset write error for retry
						stream->clearWriteError();
					}

					// some time for the stream
					delay(1);

					int leftBytes = (readBytes - bytesWrite);

					// retry to send the missed bytes
					bytesWrite = stream->write((buff + bytesWrite), leftBytes);
					bytesWritten += bytesWrite;

					if (bytesWrite != leftBytes) {
						// failed again
						//log_w("short write asked for %d but got %d failed.", leftBytes, bytesWrite);
						free(buff);
						return HTTPC_ERROR_STREAM_WRITE;
					}
				}

				// check for write error
				if (stream->getWriteError()) {
					//log_w("stream write error %d", stream->getWriteError());
					free(buff);
					return HTTPC_ERROR_STREAM_WRITE;
				}

				// count bytes to read left
				if (len > 0) {
					len -= readBytes;
				}

				delay(0);
			}
			else {
				delay(1);
			}
		}

		free(buff);

		//log_d("connection closed or file end (written: %d).", bytesWritten);

		if ((size > 0) && (size != bytesWritten)) {
			//log_d("bytesWritten %d and size %d mismatch!.", bytesWritten, size);
			return HTTPC_ERROR_STREAM_WRITE;
		}

	}
	else {
		//log_w("too less ram! need %d", HTTP_TCP_BUFFER_SIZE);
		return HTTPC_ERROR_TOO_LESS_RAM;
	}

	return bytesWritten;
}

/**
 * called to handle error return, may disconnect the connection if still exists
 * @param error
 * @return error
 */
int HTTPSimpleClient::returnError(int error)
{
	if (error < 0) {
		//log_w("error(%d): %s", error, errorToString(error).c_str());
		if (connected()) {
			//log_d("tcp stop");
			_client->stop();
		}
	}
	return error;
}

bool HTTPSimpleClient::downloadfile(String baseurl,  String filename)
{

	
	bool outcome = true;
	String url = baseurl;
	url += filename;
#ifdef DEBUG_HTTPCLIENT
	DBG_OUTPUT_PORT.println(url);
#endif
	// configure server and url
	if (!this->begin(url)) {
		DBG_OUTPUT_PORT.println(F("donload file ->Connect failed!"));
		return false;
	}

	// start connection and send HTTP header
	int httpCode = this->GET();
#ifdef DEBUG_HTTPCLIENT
	DBG_OUTPUT_PORT.println(("[HTTP] GET DONE with code " + String(httpCode)));
#endif
	if (httpCode > 0)
	{
		DBG_OUTPUT_PORT.println(F("-- >> OPENING FILE..."));

		SPIFFS.remove(filename);
		File f = SPIFFS.open(filename, "w+");
		if (!f)
		{
			DBG_OUTPUT_PORT.println(F("file open failed"));
			return false;
		}

		// HTTP header has been sent and Server response header has been handled
		DBG_OUTPUT_PORT.printf("-[HTTP] GET... code: %d\n", httpCode);
		DBG_OUTPUT_PORT.println("Free Heap: " + String(ESP.getFreeHeap()));

		// file found at server
		if (httpCode == HTTP_CODE_OK)
		{
			// get lenght of document (is -1 when Server sends no Content-Length header)
			int total = this->getSize();
			int len = total;

			DBG_OUTPUT_PORT.println("HTTP SIZE IS " + String(total));

			// create buffer for read
			uint8_t buff[128] = { 0 };

			// get tcp stream
			WiFiClient * stream = this->getStreamPtr();

			// read all data from server
			while (this->connected() && (len > 0 || len == -1))
			{

				// get available data size
				size_t size = stream->available();
				
				{
					// read up to 128 byte
					int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
					//DBG_OUTPUT_PORT.println("Write portion" + String(c));
					//DBG_OUTPUT_PORT.println((char*)buff);
					// write it to
					f.write(buff, c);

					if (len > 0)
					{
						len -= c;
					}
				}
				delay(20);
			}

			DBG_OUTPUT_PORT.println("[HTTP] connection closed or file end.");
		}
		f.close();
	}
	else
	{
		DBG_OUTPUT_PORT.println("[HTTP] GET... failed, error: " + HTTPSimpleClient::errorToString(httpCode));
		outcome = false;
	}
	this->end();
	return outcome;
}
