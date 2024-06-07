#include <Arduino.h>

#ifdef NOSTR
  #include "nostr.h"
#endif

#include "sensors.h"
#include "wManager.h"
#include "global.h"
#include "debug.h"
// #include "timers.h"

String read_tmp;

void setup() {
  
  // put your setup code here, to run once:
  Serial.begin(115200);
  debugf("Total heap: %d\n", ESP.getHeapSize());
  debugf("Free heap: %d\n", ESP.getFreeHeap());
  debugf("Total PSRAM: %d\n", ESP.getPsramSize());
  debugf("Free PSRAM: %d\n", ESP.getFreePsram());   
  init_WifiManager();
  #ifdef HAS_BATTERY
    pinMode(BATTERY_VOLTAGE_DATA, INPUT_PULLDOWN);
    analogReadResolution(12);
    analogSetPinAttenuation(BATTERY_VOLTAGE_DATA, ADC_ATTENDB_MAX);
    adcAttachPin(BATTERY_VOLTAGE_DATA);
  #endif
  #ifdef LIDAR_TFMINIPlus
    debugln("SETUP LIDAR");
    tfmini_init();
  #endif
  #ifdef LIDAR_TFMINIPlusNoLib
    debugln("SETUP LIDAR");
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
    master_pubkey = Settings.masterpub;
    setup_machine();
  #endif
  #ifdef MQTT
    mqtt_init();
  #endif
  debug("SETUP END:");
  debugf("%d\n",ESP.getFreeHeap());
  Serial.printf("HEAP:%d\n",ESP.getFreeHeap());
}
String message_data = "";

void loop() {
  // put your main code here, to run repeatedly:    
  currentMillis = millis();
  #ifdef HAS_BATTERY      
    if ( currentMillis - bat_previousMillis >= INTERVAL_1_MINUTE ) {    
      bat_previousMillis = currentMillis;
      char temp_message[6];
      sprintf(temp_message,"B:%d", analogReadMilliVolts(BATTERY_VOLTAGE_DATA) * 2);
      message_data+=temp_message;
      debug("Battery Voltage Data: ");
      debugln_(temp_message);
      #ifdef MQTT
        mqtt_publish("nostrmachines/battery_v", temp_message);
      #endif
    }
  #endif
  #ifdef LIDAR_TFMINIPlus
    //debugln("READ LIDAR");
    tfmini_read(currentMillis);     
    if ( currentMillis - lidar_previousMillis >= INTERVAL_1_MINUTE ) {          
      message_data = "";          
      lidar_previousMillis = currentMillis;                   
      debugf("Distance: D:%d", LidarData.distance); //, a15:%d, a1h:%d, a1d:%d");      
      debugf(" cm, Avg 15m:%d", LidarData.avg15);
      debugf(" cm, Avg 1h:%d", LidarData.avg1Hour);
      debugf(" cm, Avg 1d:%d", LidarData.avg1Day);
      debugf(" Temperature:%d\n", LidarData.temperature);            
      #ifdef DEBUG_PRINT_SENSOR     
        debug("Dados Rio:");     
        tfmini_PrintJson();
      #endif
      char temp_message[40];
      sprintf(temp_message,"D:%d,a15:%d,a1h:%d,a1d:%d", LidarData.distance, LidarData.avg15, LidarData.avg1Hour, LidarData.avg1Day);
      message_data+=temp_message;
      #ifdef MQTT
        sprintf(temp_message, "%d", LidarData.distance);
        mqtt_publish("nostrmachines/lidar/distance", temp_message);
        sprintf(temp_message, "%d", LidarData.temperature);
        mqtt_publish("nostrmachines/lidar/temperature", temp_message);
      #endif
      debug("LIDAR:");
      debugln_(ESP.getFreeHeap());
    }  
  #endif
  #ifdef LIDAR_TFMINIPlusNoLib
    //debugln("READ LIDAR");
    tfmininl_read();    
    debug("Distance: ");
    debug(LidarData.distance);
    debug(" cm, strength: ");
    debug(LidarData.strength);
    debug(", temperature: ");
    debugln(LidarData.temperature);
  #endif 
  #ifdef PLUV
    pluviometer_read(currentMillis); 
    if ( currentMillis >= pluv_previousMillis + INTERVAL_1_MINUTE ) {        
      pluv_previousMillis = currentMillis; 
      char temp_message[105];
      sprintf(temp_message," Volume: %.2fmm, s15:%.2f, s1h:%.2f, s1d:%.2f", PluvData.volume, PluvData.sum15, PluvData.sum1Hour, PluvData.sum1Day);     
      debugln(temp_message);
      #ifdef DEBUG_PRINT_SENSOR
          debug("Dados Chuvas:");
          pluviometer_PrintJson();
      #endif            
      sprintf(temp_message," V:%.2f,s15:%.2f,s1h:%.2f,s1d:%.2f", PluvData.volume, PluvData.sum15, PluvData.sum1Hour, PluvData.sum1Day);
      message_data+=temp_message;
      #ifdef MQTT
        sprintf(temp_message,"{\"sn\":{\"Pluviometer\":{\"volume\":%.2f,\"15min\":%.2f,\"1h\":%.2f,\"1d\":%.2f},\"Unit\":\"mm\"},\"ver\":1}", PluvData.volume, PluvData.sum15, PluvData.sum1Hour, PluvData.sum1Day);
        // debugln(temp_message);
        mqtt_publish("nostrmachines/pluviometer/sensors", temp_message);
      #endif
      debug("PLUV:");
      debugln_(ESP.getFreeHeap());
    }
  #endif 
  #ifdef DHT22_SENSOR
    dht22_read(currentMillis); 
    if ( currentMillis >= dht22_previousMillis + INTERVAL_1_MINUTE ) {      
      dht22_previousMillis = currentMillis;       
      char temp_message[60];
      sprintf(temp_message, "Temperature: %.2f,a15:%.2f,a1h:%.2f,a1d:%.2f", DhtData.temperature, DhtData.tavg15, DhtData.tavg1Hour, DhtData.tavg1Day);    
      debugln(temp_message);     
      sprintf(temp_message," H:%.2f,a15:%.2f,a1h:%.2f,a1d:%.2f", DhtData.humidity, DhtData.havg15, DhtData.havg1Hour, DhtData.havg1Day);    
      debugln(temp_message);
      #ifdef DEBUG_PRINT_SENSOR
          debugln("Dados Temp/Hum:");
          dht22_PrintJson(false);
      #endif                  
      sprintf(temp_message," T:%.2f,a15:%.2f,a1h:%.2f,a1d:%.2f", DhtData.temperature, DhtData.tavg15, DhtData.tavg1Hour, DhtData.tavg1Day);
      message_data+=temp_message;
      sprintf(temp_message," H:%.2f,a15:%.2f,a1h:%.2f,a1d:%.2f", DhtData.humidity, DhtData.havg15, DhtData.havg1Hour, DhtData.havg1Day);
      message_data+=temp_message;
      #ifdef MQTT
        sprintf(temp_message, "%.2f", DhtData.temperature);
        mqtt_publish("nostrmachines/dht22/temperature", temp_message);
        sprintf(temp_message, "%.2f", DhtData.humidity);
        mqtt_publish("nostrmachines/dht22/humidity", temp_message);
      #endif
      debug("DHT:");
      debugln_(ESP.getFreeHeap());
      debugln("---------");
    }
  #endif   
  #ifdef NOSTR
    if ( currentMillis >= nostr_previousMillis + INTERVAL_15_MINUTES ) {
      nostr_previousMillis = currentMillis;
      debug("Nostr Antes:");
      debugln_(ESP.getFreeHeap());
      // debugln(message_data);
      sendPublicMessage(message_data);
      message_data = "";   
      debug("Nostr:");
      debugln_(ESP.getFreeHeap());       
    }    
    nostrRelayManager.loop();
    nostrRelayManager.broadcastEvents();  
  #endif
  #ifdef MQTT   
   mqtt_loop();
  #endif  
  vTaskDelay(500 / portTICK_PERIOD_MS);  
}
