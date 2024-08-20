#ifdef GPIO_VIEW
#include <gpio_viewer.h> // Must me the first include in your project
GPIOViewer gpioViewer;
#endif

#include <Arduino.h>

#include "sensors.h"
#include "wManager.h"
#include "global.h"
#include "debug.h"
// #include "timers.h"
// freertos

#ifdef OTA_ENABLED
#include <ESPmDNS.h>
// #include <NetworkUdp.h>
#include <ArduinoOTA.h>
bool wait_ota = false;
#endif


//String read_tmp;
#ifdef LORAV3
  #define VBAT_CTRL GPIO_NUM_37
  #define VBAT_ADC  GPIO_NUM_1
  const float min_voltage = 3.04;
  const float max_voltage = 4.26;
  const uint8_t scaled_voltage[100] = {
    254, 242, 230, 227, 223, 219, 215, 213, 210, 207,
    206, 202, 202, 200, 200, 199, 198, 198, 196, 196,
    195, 195, 194, 192, 191, 188, 187, 185, 185, 185,
    183, 182, 180, 179, 178, 175, 175, 174, 172, 171,
    170, 169, 168, 166, 166, 165, 165, 164, 161, 161,
    159, 158, 158, 157, 156, 155, 151, 148, 147, 145,
    143, 142, 140, 140, 136, 132, 130, 130, 129, 126,
    125, 124, 121, 120, 118, 116, 115, 114, 112, 112,
    110, 110, 108, 106, 106, 104, 102, 101, 99, 97,
    94, 90, 81, 80, 76, 73, 66, 52, 32, 7,
  };

  float heltec_vbat() {
    pinMode(VBAT_CTRL, OUTPUT);
    digitalWrite(VBAT_CTRL, LOW);
    delay(5);
    float vbat = analogRead(VBAT_ADC) / 238.7;
    // pulled up, no need to drive it
    pinMode(VBAT_CTRL, INPUT);
    return vbat;
  }

  int heltec_battery_percent(float vbat = -1) {
    if (vbat == -1) {
      vbat = heltec_vbat();
    }
    for (int n = 0; n < sizeof(scaled_voltage); n++) {
      float step = (max_voltage - min_voltage) / 256;
      if (vbat > min_voltage + (step * scaled_voltage[n])) {
        return 100 - n;
      }
    }
    return 0;
  }
#endif

#ifdef NOSTR
  #include "nostr.h"
    #ifdef NOSTRTASK
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include <esp_task_wdt.h>
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
#else
  unsigned long getUnixTimestampLocal() {
    time_t now;
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
      debugln("Failed to obtain time");
      return 0;
    } else {
    // debugf("Got timestamp of ", String(now));    
    }
    time(&now);
    return now;
  }
#endif

#ifdef MQTT
  char mqtt_monitor = 0;
#endif

String message_data = "";
char wifi_monitor = 0;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -10800;
const int   daylightOffset_sec = 0;
String name_mac = "";
char pub_counter = 0;

#ifdef SET_DEEP_SLEEP_SECONDS
 /*  void print_wakeup_reason(){
    esp_sleep_wakeup_cause_t wakeup_reason;

    wakeup_reason = esp_sleep_get_wakeup_cause();

    switch(wakeup_reason)
    {
      case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
      case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
      case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
      case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
      case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
      default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
    }
  } */
#endif

#ifdef OTA_ENABLED
 void setup_ota(){ 
  debugln("OTA Setup");
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else {  // U_SPIFFS
        type = "filesystem";
      }
      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
      } 
    });
  ArduinoOTA.begin();
  }
#endif

void setup() {  
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  #ifdef STATUS_LED
    pinMode(STATUS_LED, OUTPUT);
  #endif
  #ifdef VEXT
    pinMode(VEXT_PIN, OUTPUT);
    if (VEXT==1) {
      digitalWrite(VEXT_PIN, LOW);
    } else {
      digitalWrite(VEXT_PIN, HIGH);
    }
  #endif
  debugf("Total heap: %d\n", ESP.getHeapSize());
  debugf("Free heap: %d\n", ESP.getFreeHeap());
  debugf("Total PSRAM: %d\n", ESP.getPsramSize());
  debugf("Free PSRAM: %d\n", ESP.getFreePsram());  
  
  #ifdef SET_DEEP_SLEEP_SECONDS    
    esp_sleep_wakeup_cause_t wakeup_reason;
    wakeup_reason = esp_sleep_get_wakeup_cause();
    switch(wakeup_reason)
    {
      case ESP_SLEEP_WAKEUP_EXT0 : 
            debugln("Wakeup caused by external signal using RTC_IO"); 
            #ifdef PLUV
              pluviometer_AddReaging();
            #endif
            currentMillisOffSet = INTERVAL_1_MINUTE / 2;
            break;
      case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
      case ESP_SLEEP_WAKEUP_TIMER : 
            debugln("Wakeup caused by timer"); 
            #ifdef MQTT_READ
            currentMillisOffSet = INTERVAL_1_MINUTE - 5000; // to give time to read mqtt publications
            #else
            currentMillisOffSet = INTERVAL_1_MINUTE;
            #endif
            break;
      case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
      case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
      default : debugf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
    }
    esp_sleep_enable_timer_wakeup(Settings.sleepsec * 1000000);
    #ifdef PLUV
      esp_sleep_enable_ext0_wakeup(PLUV_GPIO,1);
    #endif
  #endif
   
  init_WifiManager(); 

  #ifdef GPIO_VIEW
  gpioViewer.setSamplingInterval(125);
  gpioViewer.begin();
  #endif
  #ifdef PLUV
  // debugln("PLUVINIT");
    pluviometer_init();
  #endif

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  debugln("TIMEINIT");
  Settings.name+"_"+Settings.macaddr.substring(10);
  name_mac = Settings.macaddr.substring(9);
  name_mac.replace(":","");
  name_mac = Settings.name+"_"+name_mac;
  debugln_(name_mac);
  #ifdef MQTT
    client_name = name_mac.c_str();    
    tag_name = "nostrmachines/"+name_mac;
    debugln_(tag_name);
    mqtt_init();
  #endif
  #ifdef HAS_BATTERY
    pinMode(BATTERY_VOLTAGE_DATA, INPUT_PULLDOWN);
    #ifndef LORAV3    
      analogReadResolution(12);
      analogSetPinAttenuation(BATTERY_VOLTAGE_DATA, ADC_ATTENDB_MAX);
      adcAttachPin(BATTERY_VOLTAGE_DATA);    
    #endif
    #ifdef SLOWCLOCK
    // Better battery performance with WIFI
      setCpuFrequencyMhz(160);
    #endif
  #endif
  #ifdef LIDAR_TFMINIPlus
    debugln("SETUP LIDAR");
    tfmini_init_task();
    //tfmini_init(NULL);
  #endif
  #ifdef LIDAR_TFMINIPlusNoLib
    debugln("SETUP LIDAR");
    tfmininl_init();
  #endif

  #ifdef DHT22_SENSOR
    dht22_init();
  #endif 
  #ifdef AHT0X_SENSOR
    aht0x_init();
  #endif    
  #ifdef USSENSOR
    ultrasonic_init();
  #endif
  #ifdef NOSTR
    // Serial.printf("HEAP:%d\n",ESP.getFreeHeap());
    
    debugln("SETUP NOSTR");
    nsecHex = Settings.privkey.c_str();
    npubHex = Settings.pubkey.c_str();
    master_pubkey = Settings.masterpub;
    start_time = getUnixTimestamp() - (864000);
    setup_machine();
    #ifdef NOSTRTASK
     TaskHandle_t nostrTask1 = NULL;
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
    // debugln("RAM");
    #ifdef BOARD_HAS_PSRAM
      SpiRamJsonDocument sensor(50000);
    #else      
      StaticJsonDocument<1024> sensor;
    #endif  
    sensor["Name"] = name_mac;
    #ifdef NOSTR
     sensor["Date"] = getUnixTimestamp();
    #else
      sensor["Date"] = getUnixTimestampLocal();
    #endif
    sensor["ver"]=1;
    #ifndef SIMPLE_READ
      sensor["simple_read"]=0;
    #else    
      sensor["simple_read"]=1;
    #endif
      sensor["RSSI"]=WiFi.RSSI();
      sensor["ESPtemp"]=temperatureRead();
    // JsonObject sn = doc.createNestedObject("nostrmachines");
    // JsonObject  = doc.createNestedObject("sensor");    
    #ifdef HAS_BATTERY  
      JsonObject battery_s = sensor.createNestedObject("battery");
        #ifdef LORAV3
          battery_s["voltage"] = heltec_vbat();
          battery_s["unit"] = "V";
          battery_s["percent"] = heltec_battery_percent();
        #else      
          // sprintf(temp_message,"%d", analogReadMilliVolts(BATTERY_VOLTAGE_DATA) * 2);
          debugln("Batt");
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
    #endif
    #ifdef LIDAR_TFMINIPlus
    //debugln("READ LIDAR");                         
      /*
      debugf("Distance: D:%d\n", LidarData.distance); //, a15:%d, a1h:%d, a1d:%d");            
      debugf(" cm, Avg 15m:%d", LidarData.avg15);
      debugf(" cm, Avg 1h:%d", LidarData.avg1Hour);
      debugf(" cm, Avg 1d:%d", LidarData.avg1Day);
      debugf(" Temperature:%d\n", LidarData.temperature); */    
      JsonObject lidar_s = sensor.createNestedObject("lidar");
      lidar_s["distance"] = LidarData.distance;
      lidar_s["temp"] = LidarData.temperature;
      #ifndef SIMPLE_READ    
      lidar_s["avg15"] = LidarData.avg15;
      lidar_s["avg1hour"] = LidarData.avg1Hour;
      lidar_s["avg1day"] = LidarData.avg1Day;
      #endif 
      lidar_s["unit"] = "cm"; 
    #endif
    #ifdef PLUV                             
      // sprintf(temp_message," Volume: %.2fmm, s15:%.2f, s1h:%.2f, s1d:%.2f", PluvData.volume, PluvData.sum15, PluvData.sum1Hour, PluvData.sum1Day);     
      // debugln(temp_message);
      JsonObject pluvi_s = sensor.createNestedObject("pluviometer");
      pluvi_s["volume"] = PluvData.volume;
      #ifndef SIMPLE_READ    
      pluvi_s["sum15m"] = PluvData.sum15;
      pluvi_s["sum1hour"] = PluvData.sum1Hour;
      pluvi_s["sum1day"] = PluvData.sum1Day;
      #endif
      pluvi_s["unit"] = "mm";          
    #endif 
    #ifdef DHT22_SENSOR                    
      JsonObject dht_t = sensor.createNestedObject("temperature");      
      dht_t["temperature"] = DhtData.temperature;
      #ifndef SIMPLE_READ
      dht_t["avg15"] = DhtData.tavg15;
      dht_t["avg1hour"] = DhtData.tavg1Hour;
      dht_t["avg1day"] = DhtData.tavg1Day;
      #endif
      dht_t["unit"] = "°C";
      JsonObject dht_h = sensor.createNestedObject("humidity");
      dht_h["humidity"] = DhtData.humidity;
      #ifndef SIMPLE_READ
      dht_h["avg15"] = DhtData.havg15;
      dht_h["avg1hour"] = DhtData.havg1Hour;
      dht_h["avg1day"] = DhtData.havg1Day;
      #endif
      dht_h["unit"] = "%";            
    #endif
    #ifdef AHT0X_SENSOR                    
      JsonObject aht_t_m = sensor.createNestedObject("AHT0X");
      JsonObject aht_t = aht_t_m.createNestedObject("temperature");      
      aht_t["value"] = AhtData.temperature;
      #ifndef SIMPLE_READ
      aht_t["avg15"] = AhtData.tavg15;
      aht_t["avg1hour"] = AhtData.tavg1Hour;
      aht_t["avg1day"] = AhtData.tavg1Day;
      #endif
      aht_t["unit"] = "°C";
      JsonObject aht_h = aht_t_m.createNestedObject("humidity");
      aht_h["value"] = AhtData.humidity;
      #ifndef SIMPLE_READ
      aht_h["avg15"] = AhtData.havg15;
      aht_h["avg1hour"] = AhtData.havg1Hour;
      aht_h["avg1day"] = AhtData.havg1Day;
      #endif
      aht_h["unit"] = "%";            
    #endif       
    #ifdef USSENSOR    
      JsonObject us_s = sensor.createNestedObject("US");
      us_s["distance"] = USData.distance;      
      #ifndef SIMPLE_READ    
      us_s["avg15"] = USData.avg15;
      us_s["avg1hour"] = USData.avg1Hour;
      us_s["avg1day"] = USData.avg1Day;
      #endif 
      lidar_s["unit"] = "cm"; 
    #endif

    char buffer[512];
    size_t n = serializeJson(sensor, buffer);
    // debugln_(n);
    // serializeJsonPretty(sensor, Serial);
    sensor.clear();     
    #ifdef MQTT        
        // String tag_mqtt = "nostrmachines/"+name_mac;
        if (!mqtt_publish(tag_name.c_str(), buffer, true)){
          debug("Erro publicacao MQTT: ");
          debugln_(n);  
          debugln_(buffer);
          mqtt_monitor++;
        } else {
          debugln("Publicado no MQTT: ");
          pub_counter++;
          vTaskDelay(500 / portTICK_PERIOD_MS);
          #ifdef PLUV
            PluvData.volume = 0;
          #endif
        }
        if (mqtt_monitor>10) ESP.restart();
    #endif
    message_data = buffer;
    debug("HEAP 1 Min:");
    debugln_(ESP.getFreeHeap());          
}

#ifndef SIMPLE_READ    
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
#endif

#ifdef STATUS_LED
bool ledblink = true;
#endif

void loop() {
  // put your main code here, to run repeatedly:    
  currentMillis = millis() + currentMillisOffSet; 
  
  if (wait_ota) ArduinoOTA.handle();

  #ifdef STATUS_LED
    digitalWrite(STATUS_LED,ledblink);
    ledblink = !ledblink;
  #endif

  #ifdef MQTT          
    //debug("M");
    mqtt_loop();
    // debug("E:");
    #ifdef MQTT_READ
      if (has_mqtt_message) {                
        switch (read_mqtt_topic)
        {
        case 1:
          /* Change Sleep timer */
          debugln("Changing sleep timer");
          esp_sleep_enable_timer_wakeup(atoi(read_mqtt_message.c_str()) * 1000000);
          has_mqtt_message = false;
          break;   
        case 2:
          /* Wait for OTA */
          debugln("Wait OTA update");
          // esp_sleep_enable_timer_wakeup(5 * 1000000);
          currentMillisOffSet = -1 * (INTERVAL_1_MINUTE * atoi(read_mqtt_message.c_str()));
          // debugln_(currentMillis);
          currentMillis += currentMillisOffSet;          
          has_mqtt_message = false;
          if (!wait_ota) setup_ota();
          wait_ota = true;
          break;       
        default:
          break;
        }      
      }
    #endif
  #endif

  #ifdef LIDAR_TFMINIPlus 
  // debug("L");
    tfmini_read(currentMillis);  
  #endif
  #ifdef PLUV
    // debug("P");
    pluviometer_read(currentMillis);
    //debug("E:");
  #endif
  #ifdef DHT22_SENSOR
    // debug("D");
    dht22_read(currentMillis);
    // debug("E:");
  #endif
  #ifdef AHT0X_SENSOR
    // debug("A");
    aht0x_read(currentMillis);
    // debug("E:");
  #endif
  #ifdef USSENSOR
    // debug("U");
    ultrasonic_read(currentMillis);
    // debug("E:");
  #endif  
  
// Prepare data and publish to mqtt
  if ( currentMillis - pub_previousMillis >= INTERVAL_1_MINUTE ) {
    debugln("Preparing publication...");
    pub_previousMillis = currentMillis;
    // dataCollectionString(); 
    if(WiFi.status() == WL_CONNECTED) {
      dataCollectionJson();    
      wifi_monitor=0;
      #ifdef SET_DEEP_SLEEP_SECONDS
        if (pub_counter>=SET_DEEP_SLEEP_PUB) {
          debugln("Sleeping...");
          #ifdef STATUS_LED
            digitalWrite(STATUS_LED, LOW);
          #endif
          #ifdef VEXT
            digitalWrite(VEXT_PIN, HIGH);
          #endif
          vTaskDelay(1000/portTICK_PERIOD_MS);
          esp_deep_sleep_start();
        }
      #endif
    }
    else {
      wifi_monitor++;
    }
    // Restart if no wifi connection for 5 minutes
    if (wifi_monitor>4) ESP.restart();    
  } // End publish   

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
  vTaskDelay(500 / portTICK_PERIOD_MS);
  // debugln("V:");
}
