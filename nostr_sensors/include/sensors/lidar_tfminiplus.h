#include <TFMiniPlus.h> 
#include <ArduinoJson.h>
#include <TFMiniPlusConstants.h>
// #include "timers.h"
#include "storage.h"
#include "debug.h"

void tfmini_init(void *pvParameters);
void tfmini_init_task();
void tfmini_PrintJson();
void tfmini_read(long Lidar_currentMillis);

typedef struct TLidarData
{
	volatile uint16_t distance;
    volatile uint16_t strength;
    volatile uint16_t temperature;
    #ifndef SIMPLE_READ
    volatile uint16_t avg15;
    volatile uint16_t avg1Hour;
    volatile uint16_t avg1Day;
    #endif
} TLidarData ;

extern TLidarData LidarData;