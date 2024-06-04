
#include "sensors/lidar_tfminiplus.h"

// #define RXD2 16
// #define TXD2 17

TFMiniPlus tfmini;

// Intervalos de tempo em milissegundos
// const int INTERVAL_1_MINUTE = 60 * 1000;
/* const int vINTERVAL_1_MINUTE = (60 / (LIDAR_TIME/1000)); 
const int vINTERVAL_15_MINUTES = 15;
const int vINTERVAL_1_HOUR = 4;
const int vINTERVAL_1_DAY = 24; */

unsigned long previousMillisSeconds = 0;
unsigned long previousMillis1Minute = 0;

// Vetores para armazenar as leituras
int distancesSeconds[vINTERVAL_1_MINUTE];
int distances1Minute[vINTERVAL_15_MINUTES];
int distances15Minutes[vINTERVAL_1_HOUR];
int distances1Hour[vINTERVAL_1_DAY];

// Índices para controle de inserção nos vetores
int indexSeconds = 0;
int index1Minute = 0;
int index15Minutes = 0;
int index1Hour = 0;

int calculateAverage(int* array, int size) {
  int sum = 0;
  int valids = 0;
  for (int i = 0; i < size; i++) {        
    if (array[i]!=0) {
       valids++;
       sum += array[i];
       // Serial.println(array[i]);
    }
  }
  // Serial.println(sum);
  // Serial.println(valids);
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
                Serial.println("LIDAR: Loading LIDAR data file");
                StaticJsonDocument<1024> json;
                DeserializationError error = deserializeJson(json, configFile);
                configFile.close();
                serializeJson(json, Serial);
                Serial.print('\n');
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
                    Serial.println("LIDAR: Error parsing config file!");
                }
            }
            else
            {
                Serial.println("LIDAR: Error opening config file!");
            }
        }
        else
        {
            Serial.println("LIDAR: No config file available! Starting with zeros!");            
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
            Serial.println("LIDAR: Failed to open config file for writing");
            return false;
        }

        // Serialize JSON data to write to file
        // serializeJsonPretty(doc, Serial);
        // Serial.print('\n');
        if (serializeJson(doc, configFile) == 0)
        {
            // Error writing file
            Serial.println(F("LIDAR: Failed to write to file"));
            return false;
        }
        // Close file
        configFile.close();
        Serial.println(F("LIDAR: Success to write to file"));
        return true;        
    }
    return false;
}

void tfmini_init() {
  // Start serial port to communicate with the TFMini
  // Default baud rate is 115200  

  // Pass the Serial class initialized to the tfmini
  #ifdef LIDAR_SERIAL1
    Serial1.begin(115200, SERIAL_8N1, RXD2, TXD2);
    tfmini.begin(&Serial1);
  #elif defined(LIDAR_SERIAL2)
    Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
    tfmini.begin(&Serial2); 
  #elif
    tfmini.begin(&Serial);
  #endif
  Serial.println("SETUP LIDAR START");
  // Set baud rate "Only standard baud rates are supported"
  //tfmini.setBaudRate(115200);

  // Get firmware version
  Serial.println("Versão: "+tfmini.getVersion());

  // System Reset
  // tfmini.systemReset();

  // Set the data frame rate 1Hz
  tfmini.setFrameRate(1);

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

  loadConfig(&LidarData);

  Serial.println("SETUP LIDAR END");
  
}

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
    // serializeJsonPretty(doc, Serial);    
    Serial.println(); // Para separar cada conjunto de dados
}

void tfmini_read(unsigned long Lidar_currentMillis) {  
  // read the data frame sent by the mini
  // Enable readings
  if ( Lidar_currentMillis >= previousMillisSeconds + LIDAR_TIME ) {
    previousMillisSeconds = millis();
    tfmini.setEnabled(false); 
    if (tfmini.readData()) {
      LidarData.distance = tfmini.getDistance();
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
          saveConfig();

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

        LidarData.avg15 = calculateAverage(distancesSeconds, vINTERVAL_1_MINUTE);
        LidarData.avg1Day = calculateAverage(distances1Hour, vINTERVAL_1_DAY);
        LidarData.avg1Hour = calculateAverage(distances15Minutes, vINTERVAL_1_HOUR);        
      }
    }     
  // Disable readings, reduces temperature and readings buffer  
  tfmini.setEnabled(true); 
  }
}