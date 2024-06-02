#include <Arduino.h>

#ifdef NOSTR
  #include "nostr.h"
#endif

#include "sensors.h"
#include "wManager.h"
// #include "timers.h"

String read_tmp;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);   
  init_WifiManager();
  #ifdef LIDAR_TFMINIPlus
    Serial.println("SETUP LIDAR");
    tfmini_init();
  #endif
  #ifdef LIDAR_TFMINIPlusNoLib
    Serial.println("SETUP LIDAR");
    tfmininl_init();
  #endif
  #ifdef PLUV
    pluviometer_init();
  #endif
  #ifdef DHT22_SENSOR
    dht22_init();
  #endif
}

void loop() {
  // put your main code here, to run repeatedly:    
  currentMillis = millis();

  #ifdef LIDAR_TFMINIPlus
    //Serial.println("READ LIDAR");     
    if ( currentMillis >= lidar_previousMillis + LIDAR_TIME ) {
      tfmini_read(currentMillis);    
      lidar_previousMillis = currentMillis;           
    }
  #endif
  #ifdef LIDAR_TFMINIPlusNoLib
    //Serial.println("READ LIDAR");
    tfmininl_read();    
    Serial.print("Distance: ");
    Serial.print(LidarData.distance);
    Serial.print(" cm, strength: ");
    Serial.print(LidarData.strength);
    Serial.print(", temperature: ");
    Serial.println(LidarData.temperature);
  #endif 
  #ifdef PLUV
    pluviometer_read(currentMillis); 
    if ( currentMillis >= pluv_previousMillis + INTERVAL_1_MINUTE ) {        
      pluv_previousMillis = currentMillis;      
      Serial.print("Volume 1 min:");
      Serial.print(PluvData.volume);
      Serial.print(" mm, Sum 15m:");
      Serial.print(PluvData.sum15);
      Serial.print(" Sum 1h:");
      Serial.print(PluvData.sum1Hour);
      Serial.print(" Sum 1d:");
      Serial.println(PluvData.sum1Day);         
      #ifdef DEBUG_PRINT_SENSOR
          Serial.print("Dados Chuvas:");
          pluviometer_PrintJson();
      #endif      
    }
  #endif 
  #ifdef DHT22_SENSOR
    dht22_read(currentMillis); 
    if ( currentMillis >= dht22_previousMillis + INTERVAL_1_MINUTE ) {      
      dht22_previousMillis = currentMillis;      
      Serial.print("Temperature 1 min: ");
      Serial.print(DhtData.temperature);
      Serial.print("C, Avg 15m: ");
      Serial.print(DhtData.tavg15);
      Serial.print("C, Avg 1h: ");
      Serial.print(DhtData.tavg1Hour);
      Serial.print("C, 1d: ");
      Serial.println(DhtData.tavg1Day);
      Serial.print("Humidity 1 min: ");
      Serial.print(DhtData.humidity);
      Serial.print("%, Avg 15m: ");
      Serial.print(DhtData.havg15);
      Serial.print("%, Avg 1h: ");
      Serial.print(DhtData.havg1Hour);
      Serial.print("%, 1d: ");
      Serial.println(DhtData.havg1Day);         
      #ifdef DEBUG_PRINT_SENSOR
          Serial.print("Dados Temp/Hum:");
          dht22_PrintJson();
      #endif     
      Serial.println("---------"); 
    }
  #endif   
  #ifdef NOSTR
    nostrRelayManager.loop();
    nostrRelayManager.broadcastEvents();
  #endif
  // Serial.println(".");
  // vTaskDelay(1000 / portTICK_PERIOD_MS);
}