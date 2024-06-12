#include <Arduino.h>



#include "sensors.h"
#include "wManager.h"
#include "global.h"
#include "debug.h"
// #include "timers.h"
// freertos
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_task_wdt.h>

//String read_tmp;

#ifdef NOSTR
  #include "nostr.h"

void NOSTRTask(void *pvParameters) {  
  // relayString = Settings.nrelays;  
  while (1) {
    // debug(".");
    nostrRelayManager.loop();
    //debug("E:NR");
    nostrRelayManager.broadcastEvents();  
    //debug("E:");     
    vTaskDelay(3000/portTICK_PERIOD_MS);
  }
}
#endif

String message_data = "";

void setup() {  
  // put your setup code here, to run once:
  Serial.begin(115200);
  init_WifiManager(); 
  #ifdef MQTT
    mqtt_init();
  #endif
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
    // Serial.printf("HEAP:%d\n",ESP.getFreeHeap());
    TaskHandle_t nostrTask1 = NULL;
    debugln("SETUP NOSTR");
    nsecHex = Settings.privkey.c_str();
    npubHex = Settings.pubkey.c_str();
    master_pubkey = Settings.masterpub;
    setup_machine();
    #ifdef NOSTRTASK
     xTaskCreate(
      NOSTRTask,   /* Task function. */
      "NostrTask",     /* String with name of task. */
      15000,            /* Stack size in bytes. */
      NULL,             /* Parameter passed as input of the task */
      1,                /* Priority of the task. */
      &nostrTask1);// ,             /* Task handle. */
      // 1);        
    #endif       /* Core where the task should run */     
  #endif  
  debugln("SETUP END - HEAP:");
  debugf("Total heap: %d\n", ESP.getHeapSize());
  debugf("Free heap: %d\n", ESP.getFreeHeap());
  debugf("Total PSRAM: %d\n", ESP.getPsramSize());
  debugf("Free PSRAM: %d\n", ESP.getFreePsram());    
}

#ifdef BOARD_HAS_PSRAM 
  struct SpiRamAllocator {
    void* allocate(size_t size) {
      return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
    }

    void deallocate(void* pointer) {
      heap_caps_free(pointer);
    }

    void* reallocate(void* ptr, size_t new_size) {
      return heap_caps_realloc(ptr, new_size, MALLOC_CAP_SPIRAM);
    }
  };
  using SpiRamJsonDocument = BasicJsonDocument<SpiRamAllocator>;
#endif

void dataCollectionJson(){
    char temp_message[40];
    message_data="";
    #ifdef BOARD_HAS_PSRAM
      SpiRamJsonDocument sensor(100000);
    #else      
      StaticJsonDocument<1024> sensor;
    #endif  
    
    // JsonObject sn = doc.createNestedObject("nostrmachines");
    // JsonObject  = doc.createNestedObject("sensor");    
    #ifdef HAS_BATTERY        
        // sprintf(temp_message,"%d", analogReadMilliVolts(BATTERY_VOLTAGE_DATA) * 2);
        JsonObject battery_s = sensor.createNestedObject("battery");
        int voltage = analogReadMilliVolts(BATTERY_VOLTAGE_DATA) * 2;
        battery_s["voltage"] = voltage; // temp_message;
        battery_s["unit"] = "mV";
        int percent = 0;
        if (voltage > 4300)
        {
          percent = 100;          
        } else if (voltage < 3300)
        {
          percent = 0;
        } else {
          percent = (voltage - 3300) * 100 / 1000;
        }
        battery_s["percent"] = percent; // temp_message;        
        /* debug("Battery Voltage Data: ");
        debugln_(temp_message);  */       
    #endif
    #ifdef LIDAR_TFMINIPlus
    //debugln("READ LIDAR");                         
      /* debugf("Distance: D:%d", LidarData.distance); //, a15:%d, a1h:%d, a1d:%d");      
      debugf(" cm, Avg 15m:%d", LidarData.avg15);
      debugf(" cm, Avg 1h:%d", LidarData.avg1Hour);
      debugf(" cm, Avg 1d:%d", LidarData.avg1Day);
      debugf(" Temperature:%d\n", LidarData.temperature); */    
      JsonObject lidar_s = sensor.createNestedObject("lidar");
      lidar_s["distance"] = LidarData.distance;
      lidar_s["avg15"] = LidarData.avg15;
      lidar_s["avg1hour"] = LidarData.avg1Hour;
      lidar_s["avg1day"] = LidarData.avg1Day;
      lidar_s["temp"] = LidarData.temperature;
      lidar_s["unit"] = "cm"; 
    #endif
    #ifdef PLUV                             
      // sprintf(temp_message," Volume: %.2fmm, s15:%.2f, s1h:%.2f, s1d:%.2f", PluvData.volume, PluvData.sum15, PluvData.sum1Hour, PluvData.sum1Day);     
      // debugln(temp_message);
      JsonObject pluvi_s = sensor.createNestedObject("pluviometer");
      pluvi_s["volume"] = PluvData.volume;
      pluvi_s["sum15m"] = PluvData.sum15;
      pluvi_s["sum1hour"] = PluvData.sum1Hour;
      pluvi_s["sum1day"] = PluvData.sum1Day;
      pluvi_s["unit"] = "mm";          
    #endif 
    #ifdef DHT22_SENSOR                    
      JsonObject dht_t = sensor.createNestedObject("temperature");      
      dht_t["temperature"] = DhtData.temperature;
      dht_t["avg15"] = DhtData.tavg15;
      dht_t["avg1hour"] = DhtData.tavg1Hour;
      dht_t["avg1day"] = DhtData.tavg1Day;
      dht_t["unit"] = "°C";
      JsonObject dht_h = sensor.createNestedObject("humidity");
      dht_h["humidity"] = DhtData.humidity;
      dht_h["avg15"] = DhtData.havg15;
      dht_h["avg1hour"] = DhtData.havg1Hour;
      dht_h["avg1day"] = DhtData.havg1Day;
      dht_h["unit"] = "%";            
    #endif   
    
    sensor["ver"]=1;

    char buffer[1056];
    size_t n = serializeJson(sensor, buffer);
    // serializeJsonPretty(sensor, Serial);
      
    #ifdef MQTT        
        if (!mqtt_publish("nostrmachines/sensors", buffer, true)){
          debug("Erro publicacao MQTT: ");
          debugln_(n);  
          debugln_(buffer);
        } else {
          debugln("Publicado no MQTT: ");
        }
    #endif
    sensor.clear();      
    message_data = buffer;
    debug("HEAP 1 Min:");
    debugln_(ESP.getFreeHeap());          
}

void dataCollectionString(){
    char temp_message[256];     
    #ifdef HAS_BATTERY                        
        sprintf(temp_message,"{\"sn\":{\"baterry\":{\"voltage\":%d,\"unit\":\"mV\"}}}", analogReadMilliVolts(BATTERY_VOLTAGE_DATA) * 2);                  
        #ifdef MQTT                 
          mqtt_publish("nostrmachines/sensors/battery", temp_message);       
        #endif
    #endif
    #ifdef LIDAR_TFMINIPlus         
      sprintf(temp_message,"{\"sn\":{\"lidar\":{\"distance\":%d,\"avg15\":%d,\"avg1hour\":%d,\"avg1day\":%d,\"temp\":%d,\"unit\":\"cm\"}}}", LidarData.distance, LidarData.avg15, LidarData.avg1Hour, LidarData.avg1Day, LidarData.temperature);
      // message_data+=temp_message; 
      debugln_(temp_message);
      #ifdef MQTT                 
          mqtt_publish("nostrmachines/sensors/lidar", temp_message);       
      #endif           
    #endif
    #ifdef PLUV                             
      sprintf(temp_message,"{\"sn\":{\"pluviometer\":{\"volume\":%.2f,\"sum15m\":%.2f,\"sum1hour\":%.2f,\"sum1day\":%.2f,\"unit\":\"mm\"}}}", PluvData.volume, PluvData.sum15, PluvData.sum1Hour, PluvData.sum1Day);     
      debugln_(temp_message);
      #ifdef MQTT                 
          mqtt_publish("nostrmachines/sensors/pluviometer", temp_message);       
      #endif
    #endif 
    #ifdef DHT22_SENSOR              
      sprintf(temp_message,"{\"sn\":{\"DHT22\":{\"temperature\":{\"temperature\":%.2f,\"avg15\":%.2f,\"avg1hour\":%.2f,\"avg1day\":%.2f,\"unit\":\"°C\"},\"humidity\":{\"humidity\":%.2f,\"avg15\":%.2f,\"avg1hour\":%.2f,\"avg1day\":%.2f,\"unit\":\"%%\"}}}}", DhtData.temperature, DhtData.tavg15, DhtData.tavg1Hour, DhtData.tavg1Day, DhtData.humidity, DhtData.havg15, DhtData.havg1Hour, DhtData.havg1Day);      
      debugln_(temp_message);
      #ifdef MQTT                 
          mqtt_publish("nostrmachines/sensors/dht22", temp_message);       
      #endif              
    #endif             
    debug("HEAP 1Min:");
    debugln_(ESP.getFreeHeap());          
}

char wifi_monitor = 0;

void loop() {
  // put your main code here, to run repeatedly:    
  currentMillis = millis();    
  #ifdef LIDAR_TFMINIPlus 
    tfmini_read(currentMillis);  
  #endif
  #ifdef PLUV
    //debug("P");
    pluviometer_read(currentMillis);
    //debug("E:");
  #endif
  #ifdef DHT22_SENSOR
    //debug("D");
    dht22_read(currentMillis);
    //debug("E:");
  #endif
  
  
// Prepare data and publish to mqtt
  if ( currentMillis - pub_previousMillis >= INTERVAL_1_MINUTE ) {
    pub_previousMillis = currentMillis;
    // dataCollectionString(); 
    if(WiFi.status() == WL_CONNECTED) {
      dataCollectionJson();    
      wifi_monitor=0;       
    }
    else {
      wifi_monitor++;  
    }
    // Restart if no wifi connection for 5 minutes
    if (wifi_monitor>4) ESP.restart();
    
  } // End publish   

  #ifdef MQTT          
    //debug("M");
    mqtt_loop();
    // debug("E:");
  #endif

  #ifdef NOSTR
    // if ( currentMillis >= nostr_previousMillis + (5 * INTERVAL_1_MINUTE) ) { // testing 5 minutes
    if ( currentMillis >= nostr_previousMillis + (INTERVAL_15_MINUTES * 2) ) {
        nostr_previousMillis = currentMillis;
        debug("Nostr Antes:");
        debugln_(ESP.getFreeHeap());
        // debugln(message_data);
        sendPublicMessage(message_data);
        debug("Nostr:");
        debugln_(ESP.getFreeHeap());       
    }    
    #ifndef NOSTRTASK
      nostrRelayManager.loop();    
      nostrRelayManager.broadcastEvents();
    #endif     
  #endif    
  vTaskDelay(1000 / portTICK_PERIOD_MS);  
}
