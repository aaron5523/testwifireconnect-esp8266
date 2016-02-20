/**
 * @file TGS4161.h
 * @brief The definition of class TGS4161.
 * Based off of https://github.com/vlast3k/simple-co2-monitor-fw/
 * For the TGS4161 CO2 Sensor
 * http://www.sos.sk/a_info/resource/c/figaro/tgs4161.pdf
 */

#ifndef __TGS4161_H__
#define __TGS4161_H__

#include "Arduino.h"
#include <RunningAverage.h>

#define REFTEMP 38.0f  //when the temperature at home is 22c, inside it is 38, so it is good middle point
#define TEMPCORR 0.6f
#define GAIN 5.7f
#define MV_AT_1000 (double)25.0d
#define PPM_AT_0 350.0d
#define LOG_OF_PPM_AT_0 log10((double)PPM_AT_0)
#define LOG_OF_350 log10(350.0f)
#define TG_OF_SLOPE (MV_AT_1000/(log10(1000) - LOG_OF_PPM_AT_0))
#define GRADIENT 71.0
#define VIN 5.15f
#define HEAT_TIME 30000

class TGS4161 {
public:
	TGS4161(uint8_t anlg_in_pin, uint8_t heater_pin);
	void begin(void);
	int mv2ppm(double mv);
	double ppm2mv(int ppm);
	double tempAdjustMv(double mv, RunningAverage raTempC);
	double getCO2_Mv(double v, boolean tcorr, RunningAverage raTempC);
	double getTGSEstMaxMv(int currPPM, double currMv);
	double getVolts(double pinValue);
	double analogReadFine(byte prec = 30);
	void turnOnHeater(void);
	void turnOffHeater(void);
	void doPreheat(void);
private:
	uint8_t _anlg_in_pin, _heater_pin;
};

#endif /* #ifndef __TGS4161_H__ */
