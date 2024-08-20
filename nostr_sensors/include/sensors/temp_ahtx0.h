#include <ArduinoJson.h>
#include <Adafruit_AHTX0.h>
// #include "timers.h"
#ifndef SIMPLE_READ
    #include "storage.h"
#endif
#include "debug.h"
// #include <Adafruit_Sensor.h>

void aht0x_init();
bool aht0x_PrintJson(bool saveconfig);
void aht0x_read(long aht0x_currentMillis);


typedef struct AHT0XData
{
	volatile float temperature;
    volatile float humidity;
    #ifndef SIMPLE_READ
    volatile float tavg15;
    volatile float tavg1Hour;
    volatile float tavg1Day;
    volatile float havg15;
    volatile float havg1Hour;
    volatile float havg1Day;
    #endif
} TAHT0XData;

extern TAHT0XData AhtData;