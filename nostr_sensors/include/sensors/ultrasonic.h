#include <ArduinoJson.h>
// #include "timers.h"
//#include <DHT.h>
//#include <DHT_U.h>
//#include <Adafruit_Sensor.h>
#ifndef SIMPLE_READ
    #include "storage.h"
#endif
#include "debug.h"


void ultrasonic_init();
bool ultrasonic_PrintJson(bool saveconfig);
void ultrasonic_read(long ultrasonic_currentMillis);


typedef struct UltrasonicData
{
	volatile uint16_t distance;    
    #ifndef SIMPLE_READ    
    volatile float davg15;
    volatile float davg1Hour;
    volatile float davg1Day;    
    #endif
} TUSData;

extern TUSData USData;