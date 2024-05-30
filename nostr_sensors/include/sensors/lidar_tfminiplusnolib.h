#include <Arduino.h>

void tfmininl_init();

void tfmininl_read();

typedef struct TLidarData
{
	volatile uint16_t distance;
    volatile uint16_t strength;
    volatile long temperature;
} TLidarData ;

extern TLidarData LidarData;