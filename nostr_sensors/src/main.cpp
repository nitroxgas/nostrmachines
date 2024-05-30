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
}

void loop() {
  // put your main code here, to run repeatedly:    
  currentMillis = millis();

  #ifdef LIDAR_TFMINIPlus
    //Serial.println("READ LIDAR");     
    if ( currentMillis >= lidar_previousMillis + LIDAR_TIME ) {
      tfmini_read(currentMillis);    
      lidar_previousMillis = currentMillis;      
      Serial.print("Distance:");
      Serial.print(LidarData.distance);
      Serial.print("cm, Avg 15m:");
      Serial.print(LidarData.avg15);
      Serial.print("cm, Avg 1h:");
      Serial.print(LidarData.avg1Hour);
      Serial.print("cm, Avg 1d:");
      Serial.print(LidarData.avg1Day);
      Serial.print("cm, strength:");
      Serial.print(LidarData.strength);
      Serial.print(", temperature:");
      Serial.println(LidarData.temperature);
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
  // reboot every hour
  /* 
  if (millis() > 3600000) {
    Serial.println("Rebooting");
    ESP.restart();
  } 
  */
  #ifdef NOSTR
    nostrRelayManager.loop();
    nostrRelayManager.broadcastEvents();
  #endif
  // Serial.println(".");
  // vTaskDelay(1000 / portTICK_PERIOD_MS);
}