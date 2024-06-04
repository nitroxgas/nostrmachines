//#include <Arduino.h>
#include <ArduinoJson.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "storage.h"

#ifndef PLUV_PIN
    #define PLUV_PIN=32
#endif

void pluviometer_init();
void pluviometer_PrintJson();
void pluviometer_read(unsigned long pl_currentMillis);

typedef struct TPluvData
{
	volatile float volume;    
    volatile float sum15;
    volatile float sum1Hour;
    volatile float sum1Day;
} TPluvData ;

extern TPluvData PluvData;