#include <ArduinoJson.h>
// #include "timers.h"
#include <DHT.h>
#include <DHT_U.h>
#include "storage.h"
#include "debug.h"
// #include <Adafruit_Sensor.h>

void ultrasonic_init();
bool ultrasonic_PrintJson(bool saveconfig);
void ultrasonic_read(long ultrasonic_currentMillis);


typedef struct UltrasonicData
{
	volatile float distance;    
    #ifndef SIMPLE_READ    
    volatile float davg15;
    volatile float davg1Hour;
    volatile float davg1Day;    
    #endif
} TUSData;

extern TUSData USData;