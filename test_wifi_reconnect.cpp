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

#define SSID        "HOME-DE18"
#define PASSWORD    "H210470C899576BD"
#define HOST_NAME   "10.0.0.12"
#define HOST_PORT   (8090)

#define DBG_RX_PIN 13
#define DBG_TX_PIN 12

ESP8266 wifi = ESP8266();

SoftwareSerial debugSerial2 = SoftwareSerial(DBG_RX_PIN, DBG_TX_PIN);

bool wifiConnected = false;

void connectToWifi() {
	if (wifi.joinAP(SSID, PASSWORD)) {
		wifiConnected = true;
		debugSerial2.print(F("Join AP success\r\n"));
	} else {
		wifiConnected = false;
		debugSerial2.print(F("Join AP failure\r\n"));
	}
}

void setup(void) {
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
		uint8_t buffer[256] = { 0 };

		if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
			debugSerial2.print(F("create tcp ok\r\n"));
		} else {
			debugSerial2.print(F("create tcp err\r\n"));
			wifiConnected = false;
		}
		if (wifiConnected) {
			char *hello = "GET /ping HTTP/1.0\r\n\r\n";
			wifi.send((const uint8_t*) hello, strlen(hello));

			uint32_t len = wifi.recv(buffer, sizeof(buffer), 10000);
			debugSerial2.print(F("Current Status: "));
			String status = wifi.getIPStatus();
			debugSerial2.println(status);
			if (len > 0) {
				debugSerial2.print(F("Received:["));
				for (uint32_t i = 0; i < len; i++) {
					debugSerial2.print((char) buffer[i]);
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
			debugSerial2.println(F("WiFi not connected, failed TCP connection test"));
		}
	} else {
		debugSerial2.println(F("WiFi not connected, not attempting TCP connection"));
	}
	delay(5000);
}

