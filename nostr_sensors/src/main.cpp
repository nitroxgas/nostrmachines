#include <Arduino.h>

#ifdef NOSTR
  #include "nostr.h"
#endif

#include "sensors.h"
#include "wManager.h"

String read_tmp;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);   
  init_WifiManager();
  #ifdef LIDAR_TFMINIPlus
    Serial.println("SETUP LIDAR");
    tfmini_init();
  #endif
}

void loop() {
  // put your main code here, to run repeatedly:
  
  #ifdef LIDAR_TFMINIPlus
    //Serial.println("READ LIDAR");
    read_tmp = tfmini_read();    
    Serial.println(read_tmp);
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
  vTaskDelay(1000);
}