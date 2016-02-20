/*
 * TGS4161.cpp
 *
 *  Created on: Feb 20, 2016
 *      Author: asettl010
 *
 * Based off of https://github.com/vlast3k/simple-co2-monitor-fw/
 *
 */

#include "TGS4161.h"

TGS4161::TGS4161(uint8_t anlg_in_pin, uint8_t heater_pin) {
	_anlg_in_pin = anlg_in_pin;
	_heater_pin = heater_pin;
}

void TGS4161::begin(void) {
	pinMode(_anlg_in_pin, INPUT);
	digitalWrite(_anlg_in_pin, HIGH);
	pinMode(_heater_pin, OUTPUT);
	turnOffHeater();
}

int TGS4161::mv2ppm(double mv) {
	return pow((double) 10, mv / GRADIENT + LOG_OF_350);
}
double TGS4161::ppm2mv(int ppm) {
	return (log10((double) ppm) - LOG_OF_350) * GRADIENT;
}

double TGS4161::tempAdjustMv(double mv, RunningAverage raTempC) {
//  return mv;
	//it seems like temeperatures smaller than 18.5 degrees do not affect the sensor
	return mv + (REFTEMP - max(raTempC.getAverage(), 8.5)) * TEMPCORR;
}

double TGS4161::getCO2_Mv(double v, boolean tcorr, RunningAverage raTempC) {
	if (tcorr) {
		return tempAdjustMv(v / GAIN * 1000, raTempC);
	} else {
		return v / GAIN * 1000;
	}
}

double TGS4161::getTGSEstMaxMv(int currPPM, double currMv) {
	return ppm2mv(currPPM) + currMv;
}

double TGS4161::getVolts(double pinValue) {
   return pinValue * VIN / 1024;
}

double TGS4161::analogReadFine(byte prec = 30) {
  unsigned long sum = 0;
  for (long i=0; i < ((long)prec) * 1000; i++) {
    sum += analogRead(_anlg_in_pin);
   // delay(1);
  }
  return (double)sum / (prec * 1000);
}

void TGS4161::turnOnHeater(void) {
	digitalWrite(_heater_pin, LOW); // turn ON heater VCC
}

void TGS4161::turnOffHeater(void) {
	digitalWrite(_heater_pin, HIGH); 	 // turn OFF heater VCC TGS4161
}

void TGS4161::doPreheat(void) {
	turnOnHeater();

	currentMillis = millis();
	previousMillis = currentMillis;
	while ( ( currentMillis - previousMillis ) < HEAT_TIME) {
//		Waiting for defined heating period
	}
}
