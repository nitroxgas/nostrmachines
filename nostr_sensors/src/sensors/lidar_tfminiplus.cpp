
#include "sensors/lidar_tfminiplus.h"

TFMiniPlus tfmini;

bool lidar_started = false;
unsigned long previousMillisSeconds = 0;
unsigned long previousMillis1Minute = 0;

#ifndef SIMPLE_READ
// Vetores para armazenar as leituras
int distancesSeconds[vINTERVAL_1_MINUTE];
int distances1Minute[vINTERVAL_15_MINUTES];
int distances15Minutes[vINTERVAL_1_HOUR];
int distances1Hour[vINTERVAL_1_DAY];

// Índices para controle de inserção nos vetores
RTC_DATA_ATTR int indexSeconds = 0;
RTC_DATA_ATTR int index1Minute = 0;
RTC_DATA_ATTR int index15Minutes = 0;
RTC_DATA_ATTR int index1Hour = 0;

int calculateAverage(int* array, int size) {
  int sum = 0;
  int valids = 0;
  for (int i = 0; i < size; i++) {        
    if (array[i]!=0) {
       valids++;
       sum += array[i];
       // debugln(array[i]);
    }
  }
  // debugln(sum);
  // debugln(valids);
  if (valids==0) return 0;  
  return sum / valids;
}

/// @brief Load settings from config file located in SPIFFS.
/// @param TLidarData* Struct to update with new settings.
/// @return true on success
bool loadConfig(TLidarData* Settings)
{
    // Load existing configuration file
    // Read configuration from FS json
    if (init_spiffs())
    {
        if (SPIFFS.exists("/lidar_tfmini.json"))
        {
            // The file exists, reading and loading
            File configFile = SPIFFS.open("/lidar_tfmini.json", "r");
            if (configFile)
            {
                debugln("LIDAR: Loading LIDAR data file");
                StaticJsonDocument<1024> json;
                DeserializationError error = deserializeJson(json, configFile);
                configFile.close();
                // serializeJson(json, Serial);
                
                if (!error)
                {
                  if (json.containsKey("seconds")) {
                        JsonArray arraySeconds = json["seconds"];                        
                        for (int i = 0; i < vINTERVAL_1_MINUTE; i++){
                          distancesSeconds[i] = arraySeconds[i];
                        }
                  }
                  if (json.containsKey("1_minute")) {
                        JsonArray arrayJSON = json["1_minute"];                        
                        for (int i = 0; i < vINTERVAL_15_MINUTES; i++){
                          distances1Minute[i] = arrayJSON[i];
                        }
                  }
                  if (json.containsKey("15_minutes")) {
                        JsonArray arrayJSON = json["15_minutes"];                        
                        for (int i = 0; i < vINTERVAL_1_HOUR; i++){
                          distances15Minutes[i] = arrayJSON[i];
                        }
                  }
                  if (json.containsKey("1_hour")) {
                        JsonArray arrayJSON = json["1_hour"];                        
                        for (int i = 0; i < vINTERVAL_1_DAY; i++){
                          distances1Hour[i] = arrayJSON[i];
                        }
                  }        
                    return true;
                }
                else
                {
                    // Error loading JSON data
                    debugln("LIDAR: Error parsing config file!");
                }
                json.clear();
            }
            else
            {
                debugln("LIDAR: Error opening config file!");
            }
        }
        else
        {
            debugln("LIDAR: No config file available! Starting with zeros!");            
        }
    }
    memset(distancesSeconds, 0, sizeof(distancesSeconds));
    memset(distances1Minute, 0, sizeof(distances1Minute));
    memset(distances15Minutes, 0, sizeof(distances15Minutes));
    memset(distances1Hour, 0, sizeof(distances1Hour));

    LidarData.avg15 = 0;
    LidarData.avg1Day = 0;
    LidarData.avg1Hour = 0;
    LidarData.temperature = 0;
    LidarData.distance = 0;
    LidarData.strength = 0;                             
    return false;
}

bool saveConfig(){
  if (init_spiffs())
    {
      StaticJsonDocument<1024> doc;
      JsonArray arraySeconds = doc.createNestedArray("seconds");
      JsonArray array1Minute = doc.createNestedArray("1_minute");
      JsonArray array15Minutes = doc.createNestedArray("15_minutes");
      JsonArray array1Hour = doc.createNestedArray("1_hour");

      for (int i = 0; i < vINTERVAL_1_MINUTE; i++) {
        arraySeconds.add(distancesSeconds[i]);
      }

      for (int i = 0; i < vINTERVAL_15_MINUTES; i++) {
        array1Minute.add(distances1Minute[i]);
      }

      for (int i = 0; i < vINTERVAL_1_HOUR; i++) {
        array15Minutes.add(distances15Minutes[i]);
      }

      for (int i = 0; i < vINTERVAL_1_DAY; i++) {
        array1Hour.add(distances1Hour[i]);
      }

      File configFile = SPIFFS.open("/lidar_tfmini.json", "w");
        if (!configFile)
        {
            // Error, file did not open
            debugln("LIDAR: Failed to open config file for writing");
            return false;
        }

        // Serialize JSON data to write to file
        
        // debug('\n');
        if (serializeJson(doc, configFile) == 0)
        {
            // Error writing file
            debugln(F("LIDAR: Failed to write to file"));
            return false;
        }
        // Close file
        configFile.close();
        
        debugln(F("LIDAR: Success to write to file"));
        serializeJson(doc, Serial);
        doc.clear();
        return true;        
    }
    return false;
}


/*
void tfmini_PrintJson(){
   // Serializar e imprimir os dados em formato JSON
    StaticJsonDocument<1024> doc;
    JsonArray arraySeconds = doc.createNestedArray("seconds");
    JsonArray array1Minute = doc.createNestedArray("1_minute");
    JsonArray array15Minutes = doc.createNestedArray("15_minutes");
    JsonArray array1Hour = doc.createNestedArray("1_hour");

    for (int i = 0; i < vINTERVAL_1_MINUTE; i++) {
      arraySeconds.add(distancesSeconds[i]);
    }

    for (int i = 0; i < vINTERVAL_15_MINUTES; i++) {
      array1Minute.add(distances1Minute[i]);
    }

    for (int i = 0; i < vINTERVAL_1_HOUR; i++) {
      array15Minutes.add(distances15Minutes[i]);
    }

    for (int i = 0; i < vINTERVAL_1_DAY; i++) {
      array1Hour.add(distances1Hour[i]);
    }

    serializeJson(doc, Serial);
    doc.clear();
    // serializeJsonPretty(doc, Serial);    
    debugln(" "); // Para separar cada conjunto de dados
}
*/

void tfmini_read(unsigned long Lidar_currentMillis) {  
  // read the data frame sent by the mini
  // Enable readings
  if ( (Lidar_currentMillis >= previousMillisSeconds + LIDAR_TIME)&&(lidar_started==true) ) {
    debug("LIDAR Reading:");
    debugf("%d \n", ESP.getFreeHeap());
    //debug(".");
    previousMillisSeconds = millis();
    tfmini.setEnabled(false); 
    tfmini.triggerDetection();
    if (tfmini.readData()) {
      LidarData.distance = tfmini.getDistance();
      debugf("L:%d\n",LidarData.distance);
      LidarData.strength = tfmini.getSignalStrength();
      LidarData.temperature = tfmini.getSensorTemperature();   
      
      distancesSeconds[indexSeconds] = LidarData.distance;
      indexSeconds = (indexSeconds + 1) % vINTERVAL_1_MINUTE;

      if (Lidar_currentMillis - previousMillis1Minute >= INTERVAL_1_MINUTE) {
        previousMillis1Minute = Lidar_currentMillis;      
        distances1Minute[index1Minute] = calculateAverage(distancesSeconds, vINTERVAL_1_MINUTE);
        index1Minute = (index1Minute + 1) % vINTERVAL_15_MINUTES;        

        // Calcular a média das leituras de 15 minutos
        if (index1Minute == 0) {
          LidarData.avg15 = calculateAverage(distances1Minute, vINTERVAL_15_MINUTES);
          distances15Minutes[index15Minutes] = LidarData.avg15;
          index15Minutes = (index15Minutes + 1) % vINTERVAL_1_HOUR;    
          

          // Calcular a média das leituras de 1 hora
          if (index15Minutes == 0) {
            LidarData.avg1Hour = calculateAverage(distances15Minutes, vINTERVAL_1_HOUR);
            distances1Hour[index1Hour] = LidarData.avg1Hour;
            index1Hour = (index1Hour + 1) % vINTERVAL_1_DAY;
          }
        }
        // Calcular a média das leituras de 1 dia
        if (index1Hour == 0 && index15Minutes == 0) {
          LidarData.avg1Day = calculateAverage(distances1Hour, vINTERVAL_1_DAY);        
        }
      }
      LidarData.avg15 = calculateAverage(distancesSeconds, vINTERVAL_1_MINUTE);
      LidarData.avg1Day = calculateAverage(distances1Hour, vINTERVAL_1_DAY);
      LidarData.avg1Hour = calculateAverage(distances15Minutes, vINTERVAL_1_HOUR);
      saveConfig();
    } else {
      debugln("LIDAR: Falha de leitura"); 
      LidarData.distance = 0;
      debugf("L:%d\n",LidarData.distance);
      LidarData.strength = 0;
      LidarData.temperature = 0;           
    }
  // Disable readings, reduces temperature and readings buffer  
  tfmini.setEnabled(true); 
  //debugln("LIDAR FIM");
  }
}
#else
void tfmini_read(unsigned long Lidar_currentMillis) {  
  if ( (Lidar_currentMillis >= previousMillisSeconds)&&(lidar_started==true) ) {
    debug("LIDAR Reading:");
    // debugf("%d \n", ESP.getFreeHeap());
    //debug(".");
    previousMillisSeconds = millis();
    // read the data frame sent by the mini
    // Enable readings
    tfmini.setEnabled(false); 
    tfmini.triggerDetection();
    if (tfmini.readData()) {
      LidarData.distance = tfmini.getDistance();
      // debugf("L:%d\n",LidarData.distance);
      LidarData.strength = tfmini.getSignalStrength();
      LidarData.temperature = tfmini.getSensorTemperature();         
    } else {
      debugln("LIDAR: Falha de leitura"); 
      LidarData.distance = 0;      
      LidarData.strength = 0;
      LidarData.temperature = 0;           
    }
  // Disable readings, reduces temperature and readings buffer  
  tfmini.setEnabled(true); 
  //debugln("LIDAR FIM");
  }
}
#endif

void tfmini_init(void *pvParameters) {
  // Start serial port to communicate with the TFMini
  // Default baud rate is 115200  

  // Pass the Serial class initialized to the tfmini
  #ifdef LIDAR_SERIAL1
    Serial1.begin(115200, SERIAL_8N1, RXD1, TXD1);
    tfmini.begin(&Serial1);
  #elif defined(LIDAR_SERIAL2)
    Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
    tfmini.begin(&Serial2);
  #else
    tfmini.begin(&Serial);
  #endif
  debugln("SETUP LIDAR START");
  LidarData.distance = 0;
  LidarData.strength = 0;
  LidarData.temperature = 0;  
  // Set baud rate "Only standard baud rates are supported"
  //tfmini.setBaudRate(115200);

  // Get firmware version
  //debugf("Versão: %s\n", tfmini.getVersion());

  // System Reset
  // tfmini.systemReset();

  // Set the data frame rate 1Hz
  tfmini.setFrameRate(0);

  // If frame rate is 0 use this method to trigger a data frame
  // tfmini.triggerDetection();

  // Set ouput distance format to Centimiters
  tfmini.setMeasurementTo(TFMINI_MEASUREMENT_CM);

  // Set output distance format to Milimeters
  // tfmini.setMeasurementTo(TFMINI_MEASUREMENT_MM);

  // Disable the device
  // tfmini.setEnabled(false);
 
  // Enable the device
  // tfmini.setEnabled(true);

  // Restore to factory settings
  // tfmini.restoreFactorySettings();

  // Persist configuration into the device otherwise will be reset with the next  
  tfmini.saveSettings();
  #ifndef SIMPLE_READ
    loadConfig(&LidarData);
  #endif
  lidar_started = true;
  debugln("SETUP LIDAR END");
  
}

void tfmini_init_task(){
  xTaskCreatePinnedToCore(
    tfmini_init,   // Task function. 
    "TfMiniControlTask",     // String with name of task. 
    5000,             // Stack size in bytes. 
    NULL,             // Parameter passed as input of the task 
    1,                // Priority of the task.
    NULL,             // Task handle. 
    0);               // Core where the task should run
}