/**
 * @file CO2_ABC.h
 * @brief The definition of class CO2_ABC.
 * Based off of https://github.com/vlast3k/simple-co2-monitor-fw/
 */

#ifndef __CO2_ABC_H__
#define __CO2_ABC_H__

#include "Arduino.h"
#include "TGS4161.h"
#include <EEPROM.h>
#include <RunningAverage.h>

#define ONE_HOUR 60L*60*1000
#define EE_FLT_CURRENT_PERIOD_CO2_HIGHESTMV  4
#define EE_FLT_PREV_PERIOD_CO2_HIGHESTMV 8
#define EE_4B_HOUR 12
#define ANALOG_READ_PRECISION 10

uint32_t CO2_FIRST_PROCESS_TIME =  60L*15*1000;  //how much to wait before first storing of the data usually 15 min

int cfg_lowest_co2_ppm = 400;
byte cfg_abc_resetHours = 200;

unsigned long lastEEPROMWrite = 0;
unsigned long lastCO2Read = 0;

unsigned long timeOneHourStarted = 0;
uint32_t maxCO2RecheckTimeout = 0;

double currentCO2MaxMv = 0;
double prevCO2MaxMv = 0;

boolean startedCO2Monitoring = false;

uint16_t sPPM = 0;
RunningAverage raCO2mv(4);
RunningAverage raCO2mvNoTempCorr(4);

class CO2_ABC {
public:
	CO2_ABC(TGS4161 tgs);
	void init(void);
	void processCO2(RunningAverage raTempC);
	void readSensorData(RunningAverage raTempC);
	void processCO2SensorData(void);
	void computeCO2PPM(void);
	void storeCurrentCO2MaxMv(void);
	void co2OnOneHour(void);
	double getEECurrentCO2MaxMv(void);
	double getEEPrevCO2MaxMv(void);
	double getCO2MaxMv(void);
	byte eeGetHours(void);
	byte eeAddHourAndReturn(void);
	void debugInfoCO2ABC(void);
	boolean timePassed(unsigned long since, unsigned long interval);
	void clearEEPROM();
	int getFloat(float f);
private:
	TGS4161 _tgs;
};

#endif /* #ifndef __CO2_ABC_H__ */
