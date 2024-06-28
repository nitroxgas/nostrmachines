#include <ArduinoJson.h>
// #include "timers.h"
#include <DHT.h>
#include <DHT_U.h>
#include "storage.h"
#include "debug.h"
// #include <Adafruit_Sensor.h>

void dht22_init();
bool dht22_PrintJson(bool saveconfig);
void dht22_read(unsigned long dht22_currentMillis);

typedef struct TDHT22Data
{
	volatile float temperature;
    volatile float humidity;    
    volatile float tavg15;
    volatile float tavg1Hour;
    volatile float tavg1Day;
    volatile float havg15;
    volatile float havg1Hour;
    volatile float havg1Day;
} TDHTData;

extern TDHTData DhtData;