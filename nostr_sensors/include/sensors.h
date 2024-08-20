#ifdef LIDAR_TFMINIPlus
    #include "sensors/lidar_tfminiplus.h"
    long lidar_previousMillis = 0;
    #ifndef LIDAR_TIME
        #define LIDAR_TIME 5000
    #endif
#endif
#ifdef LIDAR_TFMINIPlusNoLib
    #include "sensors/lidar_tfminiplusnolib.h"
    long lidar_previousMillis = 0;
    #ifndef LIDAR_TIME
        #define LIDAR_TIME 5000
    #endif
#endif
#ifdef PLUV
    #include "sensors/pluviometer_bascar.h"
    //long pluv_previousMillis = 0;
#endif
#ifdef DHT22_SENSOR
    #include "sensors/temp_dht22.h"
    //long dht22_previousMillis = 0;
#endif
#ifdef AHT0X_SENSOR
    #include "sensors/temp_ahtx0.h"
    //long dht22_previousMillis = 0;
#endif
#ifdef USSENSOR
    #include "sensors/ultrasonic.h"
    //long us_previousMillis = 0;
#endif
#ifdef HAS_BATTERY
    
#endif
#ifdef MQTT
    #include "mqtt.h"
#endif

long nostr_previousMillis = 0;
long currentMillis = 0;
long currentMillisOffSet = 0;
long pub_previousMillis = 0;