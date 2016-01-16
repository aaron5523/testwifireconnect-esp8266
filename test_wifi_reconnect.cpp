/**
 * @example TCPClientSingleUNO.ino
 * @brief The TCPClientSingleUNO demo of library WeeESP8266.
 * @author Wu Pengfei<pengfei.wu@itead.cc>
 * @date 2015.03
 *
 * @par Copyright:
 * Copyright (c) 2015 ITEAD Intelligent Systems Co., Ltd. \n\n
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version. \n\n
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <ESP8266.h>
#include <SoftwareSerial.h>
#include <RGBTools.h>

#define SSID        ""
#define PASSWORD    ""
#define HOST_NAME   "10.0.0.8"
#define HOST_PORT   (8090)
#define URI_STRING_BUFFER_LENGTH   (64)

#define DBG_RX_PIN 13
#define DBG_TX_PIN 12

#define TEMP_ID     "567c133d7625422fc7a9f4e8"
int REDPin = 9;    // RED pin of the LED to PWM pin 2
int GREENPin = 10;  // GREEN pin of the LED to PWM pin 3
int BLUEPin = 11;   // BLUE pin of the LED to PWM pin 4

RGBTools rgb(REDPin,GREENPin,BLUEPin, COMMON_CATHODE);

ESP8266 wifi = ESP8266();

SoftwareSerial debugSerial2 = SoftwareSerial(DBG_RX_PIN, DBG_TX_PIN);

bool wifiConnected = false;

void connectToWifi() {



	if (wifi.joinAP(SSID, PASSWORD)) {
		wifiConnected = true;
		debugSerial2.print(F("Join AP success\r\n"));
		rgb.setColor(0,255,0);
	} else {
		wifiConnected = false;
		debugSerial2.print(F("Join AP failure\r\n"));
		rgb.setColor(255,0,0);
	}
}

void save_value(String sensorId, String value) {
	//Build a HTTP GET string to store the update
	String URI_STRING = "GET /u/";
	URI_STRING += wifi.getApMac();
	URI_STRING += "/";
	URI_STRING += sensorId;
	URI_STRING += "?rv=";
	URI_STRING += value;
	URI_STRING += " HTTP/1.0\r\n\r\n";
	debugSerial2.print(F("URI_STRING.length(): "));
	debugSerial2.println(URI_STRING.length());
	debugSerial2.print(F("URI_STRING: "));
	debugSerial2.println(URI_STRING);
	char uriCharBuffer[URI_STRING_BUFFER_LENGTH];
	memset(uriCharBuffer, 0, URI_STRING_BUFFER_LENGTH);
	URI_STRING.toCharArray(uriCharBuffer, URI_STRING_BUFFER_LENGTH);

	uint8_t responseBuffer[256] = { 0 };

	if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
		debugSerial2.print(F("create tcp ok\r\n"));
	} else {
		debugSerial2.print(F("create tcp err\r\n"));
		wifiConnected = false;
	}
	if (wifiConnected) {
		wifi.send((const uint8_t*) uriCharBuffer, strlen(uriCharBuffer));

		uint32_t len = wifi.recv(responseBuffer, sizeof(responseBuffer), 10000);
		debugSerial2.print(F("Current Status: "));
		String status = wifi.getIPStatus();
		debugSerial2.println(status);
		if (len > 0) {
			debugSerial2.print(F("Received:["));
			for (uint32_t i = 0; i < len; i++) {
				debugSerial2.print((char) responseBuffer[i]);
			}
			debugSerial2.print(F("]\r\n"));
		} else {
			wifiConnected = false;
		}

		if (status.indexOf("STATUS:3") != -1) {
			if (wifi.releaseTCP()) {
				debugSerial2.print(F("release tcp ok\r\n"));
			} else {
				debugSerial2.print(F("release tcp err\r\n"));
			}
		} else {
			debugSerial2.println(F("no connection to release"));
		}
	} else {
		debugSerial2.println(
				F("WiFi not connected, failed TCP connection test"));
	}
}

void setup(void) {
	  pinMode(REDPin, OUTPUT);
	  pinMode(GREENPin, OUTPUT);
	  pinMode(BLUEPin, OUTPUT);
//	  for (int i = 0; i < 255; i++) {
	  	  	rgb.setColor(255,0,0);
	  		delay(1000);
	  		rgb.setColor(0,255,0);
	  		delay(1000);
	  		rgb.setColor(0,0,255);
	  		delay(1000);
//	  	}
	debugSerial2.begin(115200);
	debugSerial2.print(F("setup begin\r\n"));
	wifi.begin();
	debugSerial2.print(F("Kicking ESP result: "));
	debugSerial2.println(wifi.kick());
	debugSerial2.print(F("FW Version:"));
	debugSerial2.println(wifi.getVersion().c_str());

	if (wifi.setOprToStationSoftAP()) {
		debugSerial2.print(F("to station + softap ok\r\n"));
	} else {
		debugSerial2.print(F("to station + softap err\r\n"));
	}

	connectToWifi();

	if (wifi.disableMUX()) {
		debugSerial2.print(F("single ok\r\n"));
	} else {
		debugSerial2.print(F("single err\r\n"));
	}

	debugSerial2.print(F("setup end\r\n"));
}

void loop(void) {

	if (!wifiConnected) {
		connectToWifi();
	}

	if (wifiConnected) {
		save_value("testId", "testVal");
	} else {
		debugSerial2.println(
				F("WiFi not connected, not attempting TCP connection"));
	}
	delay(5000);
}

