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
#include "DHT.h"
#include <math.h>
#include "TGS4161.h"
#include "CO2_ABC.h"

#define SSID        "HOME-DE18"
#define PASSWORD    ""
#define HOST_NAME   "10.0.0.12"
#define HOST_PORT   (8090)
#define URI_STRING_BUFFER_LENGTH   (64)
#define HTTP_RETRIES (3)
#define HTTP_REQ_TIMEOUT (10000)
#define HTTP_DELAY_BETWEEN_RETRY (5000);

#define DHTPIN 4     // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

#define vref 4.51 // For the computeVolts(); funtcion.

#define CO2_HEATER_PIN 5
#define CO2_DATA_PIN A0

#define x1 350
#define y1 0
#define x2 3000
#define y2 64

const int numReadings = 10;

int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average

#define DBG_RX_PIN 13
#define DBG_TX_PIN 12

#define TEMP_ID     "t1"
#define HUMID_ID    "h1"

//#define UPDATE_DELAY 300000
#define UPDATE_DELAY 10000

int REDPin = 9;    // RED pin of the LED to PWM pin 9
int GREENPin = 11;  // GREEN pin of the LED to PWM pin 11
int BLUEPin = 10;   // BLUE pin of the LED to PWM pin 10

uint16_t calVal = 0;

ESP8266 wifi = ESP8266();
DHT dht(DHTPIN, DHTTYPE);
RGBTools rgb(REDPin, GREENPin, BLUEPin, COMMON_CATHODE);

SoftwareSerial debugSerial2 = SoftwareSerial(DBG_RX_PIN, DBG_TX_PIN);

bool wifiConnected = false;

/* RGB LED Methods*/
void setRed() {
	rgb.setColor(255, 0, 0);
}

void setGreen() {
	rgb.setColor(0, 255, 0);
}

void setBlue() {
	rgb.setColor(0, 0, 255);
}

void setOrange() {
	rgb.setColor(255, 136, 0);
}

void setWhite() {
	rgb.setColor(255, 255, 255);
}

void setBlack() {
	rgb.setColor(0, 0, 0);
}

void setTeal() {
	rgb.setColor(0, 242, 255);
}

void setPurple() {
	rgb.setColor(140, 0, 255);
}

void setYellow() {
	rgb.setColor(200, 200, 0);
}

/*RGB Animations*/
void blinkGreen(int times) {
	for (int i = 0; i < times; i++) {
		setGreen();
		delay(250);
		setBlack();
		delay(250);
	}
}

void printCurTime() {
	debugSerial2.print(F(" cur_millis: "));
	debugSerial2.println(millis());
}

/*ESP8266 Methods*/
void connectToWifi() {
	setTeal();
	if (wifi.joinAP(SSID, PASSWORD)) {
		wifiConnected = true;
		debugSerial2.print(F("Join AP success\r\n"));
		blinkGreen(3);
		setGreen();
	} else {
		wifiConnected = false;
		debugSerial2.print(F("Join AP failure\r\n"));
		setRed();
	}
}

String checkAndReleaseConnection() {
	debugSerial2.print(F("Current Status: "));
	String status = wifi.getIPStatus();
	debugSerial2.println(status);
	if (status.indexOf("STATUS:3") != -1) {
		if (wifi.releaseTCP()) {
			debugSerial2.print(F("release tcp ok\r\n"));
		} else {
			debugSerial2.print(F("release tcp err\r\n"));
		}
	} else {
		debugSerial2.println(F("no connection to release"));
	}
	return status;
}

int calcAndDoDelay(int i) {
	int delayTime = HTTP_DELAY_BETWEEN_RETRY
	;
	debugSerial2.print(F("Sleeping between retry: "));
	debugSerial2.print(delayTime * (i + 1));
	printCurTime();
	delay(delayTime * (i + 1));
	return delayTime;
}

void save_value(String sensorId, String value) {
	printCurTime();
	value.replace(" ", "");
	//Build a HTTP GET string to store the update
//	String URI_STRING = "GET /ping/";
	String URI_STRING = "GET /u/";
	URI_STRING += wifi.getApMac();
	URI_STRING += "/";
	URI_STRING += sensorId;
	URI_STRING += "?rv=";
	URI_STRING += value;
	URI_STRING += " HTTP/1.0\r\n\r\n";
//	String URI_STRING = "GET /api/postvalue/?token=";
//	URI_STRING+=API_KEY;
//	URI_STRING += "&variable=";
//	URI_STRING += sensorId;
//	URI_STRING += "&value=";
//	URI_STRING += value;
//	URI_STRING += "\r\n\r\n";
	debugSerial2.print(F("URI_STRING.length(): "));
	debugSerial2.println(URI_STRING.length());
	debugSerial2.print(F("URI_STRING: "));
	debugSerial2.println(URI_STRING);
	char uriCharBuffer[URI_STRING_BUFFER_LENGTH];
	memset(uriCharBuffer, 0, URI_STRING_BUFFER_LENGTH);
	URI_STRING.toCharArray(uriCharBuffer, URI_STRING_BUFFER_LENGTH);

//	uint8_t responseBuffer[256] = { 0 };
	for (int i = 0; i < HTTP_RETRIES; i++) {
		debugSerial2.print(F("HTTP_RETRIES: "));
		debugSerial2.print(i);
		printCurTime();
		bool tcpEstablished = false;
		if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
			tcpEstablished = true;
			debugSerial2.print(F("create tcp ok"));
			printCurTime();
		} else {
			debugSerial2.print(F("create tcp err"));
			printCurTime();
			setYellow();
			if (i + 1 == HTTP_RETRIES) {
				wifiConnected = false;
				setRed();
			}
		}
		if (wifiConnected && tcpEstablished) {
			bool httpGetOk = false;
			if (!httpGetOk) {
				wifi.send((const uint8_t*) uriCharBuffer,
						strlen(uriCharBuffer));

				//		uint32_t len = wifi.recv(responseBuffer, sizeof(responseBuffer), 10000);

				httpGetOk = wifi.recvHTTP( HTTP_REQ_TIMEOUT);
			}

			debugSerial2.print(F("httpGetOk: "));
			debugSerial2.print(httpGetOk);
			printCurTime();
//		if (len > 0) {
//			debugSerial2.print(F("Received:["));
//			for (uint32_t i = 0; i < len; i++) {
//				debugSerial2.print((char) responseBuffer[i]);
//			}
//			debugSerial2.print(F("]\r\n"));
//		} else {
//			wifiConnected = false;
//		}

			if (httpGetOk) {
				setGreen();
				checkAndReleaseConnection();
				break;
			} else {
				setYellow();
				checkAndReleaseConnection();
			}

			calcAndDoDelay(i);
		} else {
			debugSerial2.print(F("Failed TCP connection test"));
			checkAndReleaseConnection();
			printCurTime();
			setYellow();
			calcAndDoDelay(i);
		}
	}

}

/*Arduino Methods*/
void doRGBPOST() {
	for (int i = 0; i < 5; i++) {
		setWhite();
		delay(500);
		setBlack();
		delay(500);
	}

	setRed();
	delay(1000);
	setGreen();
	delay(1000);
	setBlue();
	delay(1000);
	setWhite();
}

void setup(void) {


	pinMode(REDPin, OUTPUT);
	pinMode(GREENPin, OUTPUT);
	pinMode(BLUEPin, OUTPUT);

//    pinMode(figaroCOcircuit, OUTPUT);


	doRGBPOST();
	debugSerial2.begin(57600);
	debugSerial2.print(F("setup begin\r\n"));
//	setPurple();
//	wifi.begin();
//	if (wifi.kick()) {
//		blinkGreen(1);
//	} else {
//		setOrange();
//	}
//
//	debugSerial2.print(F("FW Version:"));
//	debugSerial2.println(wifi.getVersion().c_str());
//
//	if (wifi.setOprToStationSoftAP()) {
//		debugSerial2.print(F("to station + softap ok\r\n"));
//		blinkGreen(2);
//	} else {
//		debugSerial2.print(F("to station + softap err\r\n"));
//	}
//
//	connectToWifi();
//
//	if (wifi.disableMUX()) {
//		debugSerial2.print(F("single ok\r\n"));
//	} else {
//		debugSerial2.print(F("single err\r\n"));
//	}

	debugSerial2.print(F("setup end\r\n"));
	printCurTime();
}

//float computeVolts(int rawADC) {
//	return (rawADC * vref) / 1023.0;
//}

float computeAdc(float volts) {
	return (volts * 1023.0) / vref;
}

//int getAverage(byte sensorPin, int avgNum, int del) {
//	int i = 0;
//	long sum = 0;
//	unsigned int value = 0;
//	unsigned int result = 0;
//
//	for (i = 0; i < avgNum; i++) {
//		value = analogRead(sensorPin);
//		sum = sum + value;
//		delayMicroseconds(del);
//	}
//	result = sum / avgNum;
//	return (result);
//
//}

int getPeak(byte peakPin, int peakNum, int del) {
	int i = 0;
	unsigned int value = 0;
	unsigned int valueMax = 0;
	unsigned int result = 0;

	for (i = 0; i < peakNum; i++) {
		value = analogRead(peakPin);

		// record the maximum sensor value
		if (value > valueMax)
			valueMax = value;

		delay(del);

	}
	return (valueMax);
}

#define VIN 4.45d

double getVolts(double pinValue) {
	return pinValue * VIN / 1024;
}

float computeFscale(float originalMin, float originalMax, float newBegin,
		float newEnd, float inputValue, float curve) {

	float OriginalRange = 0;
	float NewRange = 0;
	float zeroRefCurVal = 0;
	float normalizedCurVal = 0;
	float rangedValue = 0;
	boolean invFlag = 0;

	// condition curve parameter
	// limit range

	if (curve > 10)
		curve = 10;
	if (curve < -10)
		curve = -10;

	curve = (curve * -.1); // - invert and scale - this seems more intuitive - postive numbers give more weight to high end on output
	curve = pow(10, curve); // convert linear scale into lograthimic exponent for other pow function

	/*
	 Serial.println(curve * 100, DEC);   // multply by 100 to preserve resolution
	 Serial.println();
	 */

	// Check for out of range inputValues
	if (inputValue < originalMin) {
		inputValue = originalMin;
	}
	if (inputValue > originalMax) {
		inputValue = originalMax;
	}

	// Zero Refference the values
	OriginalRange = originalMax - originalMin;

	if (newEnd > newBegin) {
		NewRange = newEnd - newBegin;
	} else {
		NewRange = newBegin - newEnd;
		invFlag = 1;
	}

	zeroRefCurVal = inputValue - originalMin;
	normalizedCurVal = zeroRefCurVal / OriginalRange; // normalize to 0 - 1 float

	/*
	 Serial.print(OriginalRange, DEC);
	 Serial.print("   ");
	 Serial.print(NewRange, DEC);
	 Serial.print("   ");
	 Serial.println(zeroRefCurVal, DEC);
	 Serial.println();
	 */

	// Check for originalMin > originalMax  - the math for all other cases i.e. negative numbers seems to work out fine
	if (originalMin > originalMax) {
		return 0;
	}

	if (invFlag == 0) {
		rangedValue = (pow(normalizedCurVal, curve) * NewRange) + newBegin;

	} else     // invert the ranges
	{
		rangedValue = newBegin - (pow(normalizedCurVal, curve) * NewRange);
	}

	return rangedValue;
}

/* ASK Shield - CO2 TGS4161 */

/* Linearization equation from Libelium Forum (All values in millivolts)
 y = 62.877*log(x) - 158.631
 x = 10^( (y + 158.631)/62.877 )
 x= pow(10, ((y + 158,631)/62,877));
 where x: ppm concentration and y: DELTA(EMF) */

//SMOTHING FILTER CONSTANT
#define alphaTgs 0.95

//TIME FOR HEATING BEFORE MEASURE
#define heatTime 30000

//CALIBRATION FOR EVERY CO2 SENSOR
//#define TGS4161_350  0.145 // Iaac
#define TGS4161_350  0.345  // Hangar
//#define TGS4161_350  0.68 // Bordils

//SERIAL DEBUGING
#define debugCO2 1

int figaroCO2Val = 0;
float ppm = 0;
float ppm_last = 0;
float incEmc = 0;

unsigned long currentMillis = 0;
unsigned long previousMillis = 0;

int getTgs4161(byte tgsPin) {

	//digitalWrite(figaroCircuit, LOW); // disconect R12 of the circuit (only for tgs2442)

	currentMillis = millis();
	previousMillis = currentMillis;

	while (currentMillis - previousMillis < heatTime) { //read during heater Time

		currentMillis = millis();
		//Serial.println(currentMillis - previousMillis);

//		figaroCO2Val = getAverage(tgsPin, 32, 1000); // take average measurement here every 1ms

//		incEmc = computeVolts((computeAdc(TGS4161_350) - figaroCO2Val)) * 1000;
//		incEmc = computeVolts((figaroCO2Val)) * 1000;
		if (incEmc < 0)
			incEmc = 0;

		ppm = pow(10, ((incEmc + 158.631) / 62.877));
		debugSerial2.print("PPM RAW: ");
		debugSerial2.print(ppm);
		if (ppm > 10000)
			ppm = 10000;

		ppm = (alphaTgs * ppm_last + (1 - alphaTgs) * ppm); //smoothing
		ppm_last = ppm;

		debugSerial2.print(" RAW CO2: ");
//		debugSerial2.print(computeVolts(figaroCO2Val), 4);
		debugSerial2.print("  AEMC: ");
		debugSerial2.print(incEmc);
		debugSerial2.print("  ADC: ");
		debugSerial2.print(computeAdc(TGS4161_350));
		debugSerial2.print("  PPM: ");
		debugSerial2.println(ppm);
	}

	//digitalWrite(figaroCO2Heater, HIGH); // turn OFF heater VCC

	debugSerial2.print("Final PPM: ");
	debugSerial2.println(ppm);

	return ((int) ppm);

}

void loop(void) {

//	if (!wifiConnected) {
//		connectToWifi();
//	}
//
//	if (wifiConnected) {
//		float f = dht.readTemperature(true);
//		float h = dht.readHumidity();
//
//		debugSerial2.print(F("Temp: "));
//		debugSerial2.print(f);
//		debugSerial2.print(F(" Humid: "));
//		debugSerial2.print(h);
//		printCurTime();
//		save_value(HUMID_ID, String(h));
//		delay(5000);
//		save_value(TEMP_ID, String(f));
////		save_value("testId", "testVal");
//	} else {
//		rgb.setColor(255, 0, 0);
//		debugSerial2.println(
//				F("WiFi not connected, not attempting TCP connection"));
//	}

	delay(5000);
}

