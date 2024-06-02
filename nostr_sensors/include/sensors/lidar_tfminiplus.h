#include <TFMiniPlus.h> 
#include <ArduinoJson.h>
#include <TFMiniPlusConstants.h>
#include "timers.h"

void tfmini_init();
void tfmini_PrintJson();
void tfmini_read(unsigned long Lidar_currentMillis);

typedef struct TLidarData
{
	volatile uint16_t distance;
    volatile uint16_t strength;
    volatile uint16_t temperature;
    volatile uint16_t avg15;
    volatile uint16_t avg1Hour;
    volatile uint16_t avg1Day;
} TLidarData ;

extern TLidarData LidarData;