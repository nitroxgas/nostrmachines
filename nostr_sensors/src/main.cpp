#include <Arduino.h>

#ifdef NOSTR
  #include "nostr.h"
#endif

#include "sensors.h"
#include "wManager.h"
#include "global.h"
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
  #ifdef NOSTR
    // relayString = Settings.nrelays;
    nsecHex = Settings.privkey.c_str();
    npubHex = Settings.pubkey.c_str();
    setup_machine();
  #endif
}
String message_data = "";

void loop() {
  // put your main code here, to run repeatedly:    
  currentMillis = millis();

  #ifdef LIDAR_TFMINIPlus
    //Serial.println("READ LIDAR");
    tfmini_read(currentMillis);     
    if ( currentMillis - lidar_previousMillis >= INTERVAL_1_MINUTE ) {          
      lidar_previousMillis = currentMillis;                   
      Serial.print("Distance:");
      Serial.print(LidarData.distance);
      Serial.print(" cm, Avg 15m:");
      Serial.print(LidarData.avg15);
      Serial.print(" Avg 1h:");
      Serial.print(LidarData.avg1Hour);
      Serial.print(" Avg 1d:");
      Serial.print(LidarData.avg1Day);
      Serial.print(" strength:");
      Serial.print(LidarData.strength);
      Serial.print(", temperature:");
      Serial.println(LidarData.temperature);      
      #ifdef DEBUG_PRINT_SENSOR     
        Serial.print("Dados Rio:");     
        tfmini_PrintJson();
      #endif
      char temp_message[40];
      sprintf(temp_message,"D:%d,a15:%d,a1h:%d,a1d:%d", LidarData.distance, LidarData.avg15, LidarData.avg1Hour, LidarData.avg1Day);
      message_data+=temp_message;
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
      Serial.print("Volume 1min:");
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
      char temp_message[40];
      sprintf(temp_message," V:%.2f,s15:%.2f,s1h:%.2f,s1d:%.2f", PluvData.volume, PluvData.sum15, PluvData.sum1Hour, PluvData.sum1Day);
      message_data+=temp_message;
    }
  #endif 
  #ifdef DHT22_SENSOR
    dht22_read(currentMillis); 
    if ( currentMillis >= dht22_previousMillis + INTERVAL_1_MINUTE ) {      
      dht22_previousMillis = currentMillis;      
      Serial.print("Temperature 1min: ");
      Serial.print(DhtData.temperature);
      Serial.print("C, Avg 15m: ");
      Serial.print(DhtData.tavg15);
      Serial.print("C, Avg 1h: ");
      Serial.print(DhtData.tavg1Hour);
      Serial.print("C, 1d: ");
      Serial.println(DhtData.tavg1Day);
      Serial.print("Humidity 1min: ");
      Serial.print(DhtData.humidity);
      Serial.print("%, Avg 15m: ");
      Serial.print(DhtData.havg15);
      Serial.print("%, Avg 1h: ");
      Serial.print(DhtData.havg1Hour);
      Serial.print("%, 1d: ");
      Serial.println(DhtData.havg1Day);         
      #ifdef DEBUG_PRINT_SENSOR
          Serial.print("Dados Temp/Hum:");
          dht22_PrintJson(false);
      #endif     
      Serial.println("---------"); 
      char temp_message[40];
      sprintf(temp_message," T:%.2f,a15:%.2f,a1h:%.2f,a1d:%.2f", DhtData.temperature, DhtData.tavg15, DhtData.tavg1Hour, DhtData.tavg1Day);
      message_data+=temp_message;
      sprintf(temp_message," H:%.2f,a15:%.2f,a1h:%.2f,a1d:%.2f", DhtData.humidity, DhtData.havg15, DhtData.havg1Hour, DhtData.havg1Day);
      message_data+=temp_message;
    }
  #endif   
  #ifdef NOSTR
    if ( currentMillis >= nostr_previousMillis + INTERVAL_1_MINUTE ) {
      nostr_previousMillis = currentMillis;
      sendPublicMessage(message_data);
      message_data = "";          
    } else {
      // if (currentMillis >= nostr_previousMillis + 30000) {
        nostrRelayManager.loop();
        nostrRelayManager.broadcastEvents();
      // }
    }
  #endif
  // Serial.println(".");
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}