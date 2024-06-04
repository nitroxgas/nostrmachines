#ifdef LIDAR_TFMINIPlus
    #include "sensors/lidar_tfminiplus.h"
    unsigned long lidar_previousMillis = 0;
    #ifndef LIDAR_TIME
        #define LIDAR_TIME 5000
    #endif
#endif
#ifdef LIDAR_TFMINIPlusNoLib
    #include "sensors/lidar_tfminiplusnolib.h"
    unsigned long lidar_previousMillis = 0;
    #ifndef LIDAR_TIME
        #define LIDAR_TIME 5000
    #endif
#endif
#ifdef PLUV
    #include "sensors/pluviometer_bascar.h"
    unsigned long pluv_previousMillis = 0;
#endif
#ifdef DHT22_SENSOR
    #include "sensors/temp_dht22.h"
    unsigned long dht22_previousMillis = 0;
#endif
unsigned long nostr_previousMillis = 0;
unsigned long currentMillis = 0;