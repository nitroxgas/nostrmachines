#include "sensors/pluviometer_bascar.h"

TPluvData PluvData;

//RTC_DATA_ATTR 
static volatile int readings = 0;
// static volatile int flux_adjust = 0;
static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;

volatile unsigned long DebounceTimer = 0;
volatile unsigned int delayTime = 250;
unsigned long pl_previousMillis1Minute = 0;

#ifndef SIMPLE_READ
// Intervalos de tempo em milissegundos
// const int INTERVAL_1_MINUTE = 60 * 1000;
// const int pl_vINTERVAL_1_MINUTE = (60 / (LIDAR_TIME/1000)); 
const int pl_vINTERVAL_15_MINUTES = 15;
const int pl_vINTERVAL_1_HOUR = 4;
const int pl_vINTERVAL_1_DAY = 24;

// Vetores para armazenar as leituras
// int volumeSeconds[pl_vINTERVAL_1_MINUTE];
float volume1Minute[pl_vINTERVAL_15_MINUTES];
float volume15Minutes[pl_vINTERVAL_1_HOUR];
float volume1Hour[pl_vINTERVAL_1_DAY];

// Índices para controle de inserção nos vetores
RTC_DATA_ATTR int pindexSeconds = 0;
RTC_DATA_ATTR int pindex1Minute = 0;
RTC_DATA_ATTR int pindex15Minutes = 0;
RTC_DATA_ATTR int pindex1Hour = 0;

// Função para calcular a soma de um vetor
float calculateSum(float* array, int size) {
  float sum = 0;
  for (int i = 0; i < size; i++) {
    sum += array[i];
  }
  return sum;
}

void pluviometer_PrintJson(){
   // Serializar e imprimir os dados em formato JSON
    StaticJsonDocument<1024> doc;
    JsonArray array1Minute = doc.createNestedArray("1_minute");
    JsonArray array15Minutes = doc.createNestedArray("15_minutes");
    JsonArray array1Hour = doc.createNestedArray("1_hour");

    for (int i = 0; i < pl_vINTERVAL_15_MINUTES; i++) {
      array1Minute.add(volume1Minute[i]);
    }

    for (int i = 0; i < pl_vINTERVAL_1_HOUR; i++) {
      array15Minutes.add(volume15Minutes[i]);
    }

    for (int i = 0; i < pl_vINTERVAL_1_DAY; i++) {
      array1Hour.add(volume1Hour[i]);
    }

    serializeJson(doc, Serial);
    // serializeJsonPretty(doc, Serial);    
    debugln(" "); // Para separar cada conjunto de dados    
}

/// @brief Load settings from config file located in SPIFFS.
/// @return true on success
bool pluviometer_loadConfig()
{
    // Load existing configuration file
    // Read configuration from FS json
    if (init_spiffs())
    {
        if (SPIFFS.exists("/pluviometer.json"))
        {
            // The file exists, reading and loading
            File configFile = SPIFFS.open("/pluviometer.json", "r");
            if (configFile)
            {
                debugln("PLUV: Loading Pluviometer data file");
                StaticJsonDocument<1024> json;
                DeserializationError error = deserializeJson(json, configFile);
                configFile.close();
                serializeJson(json, Serial);
                // debugln_(' ');
                if (!error)
                {
                  if (json.containsKey("1_minute")) {
                        JsonArray arraySeconds = json["1_minute"];                        
                        for (int i = 0; i < pl_vINTERVAL_15_MINUTES; i++){
                          volume1Minute[i] = arraySeconds[i];
                        }
                  }                  
                  if (json.containsKey("15_minutes")) {
                        JsonArray arrayJSON = json["15_minutes"];                        
                        for (int i = 0; i < vINTERVAL_1_HOUR; i++){
                          volume15Minutes[i] = arrayJSON[i];
                        }
                  }
                  if (json.containsKey("1_hour")) {
                        JsonArray arrayJSON = json["1_hour"];                        
                        for (int i = 0; i < vINTERVAL_1_DAY; i++){
                          volume1Hour[i] = arrayJSON[i];
                        }
                  }        
                    return true;
                }
                else
                {
                    // Error loading JSON data
                    debugln("PLUV: Error parsing config file!");
                }
            }
            else
            {
                debugln("PLUV: Error opening config file!");
            }
        }
        else
        {
            debugln("PLUV: No config file available! Starting with zeros!"); 
                       
        }
    }
    debugln("PLUV: Starting with zeros!");
    memset(volume1Minute, 0, sizeof(volume1Minute));
    memset(volume15Minutes, 0, sizeof(volume15Minutes));
    memset(volume1Hour, 0, sizeof(volume1Hour));
    PluvData.sum15 = 0;
    PluvData.sum1Day = 0;
    PluvData.sum1Hour = 0;
    PluvData.volume = 0;                                 
    return false;
}

bool pluviometer_saveConfig(){
  if (init_spiffs())
    {
      // Serializar e imprimir os dados em formato JSON
      StaticJsonDocument<1024> doc;
      JsonArray array1Minute = doc.createNestedArray("1_minute");
      JsonArray array15Minutes = doc.createNestedArray("15_minutes");
      JsonArray array1Hour = doc.createNestedArray("1_hour");

      for (int i = 0; i < pl_vINTERVAL_15_MINUTES; i++) {
        array1Minute.add(volume1Minute[i]);
      }

      for (int i = 0; i < pl_vINTERVAL_1_HOUR; i++) {
        array15Minutes.add(volume15Minutes[i]);
      }

      for (int i = 0; i < pl_vINTERVAL_1_DAY; i++) {
        array1Hour.add(volume1Hour[i]);
      }

      File configFile = SPIFFS.open("/pluviometer.json", "w");
      if (!configFile)
      {
          // Error, file did not open
          debugln("PLUV: Failed to open config file for writing");
          return false;
      }
      if (serializeJson(doc, configFile) == 0)
      {
          // Error writing file
          debugln("PLUV: Failed to write to file");
          return false;
      }
      // Close file
      configFile.close();
      debugln("PLUV: Success to write to file");
      serializeJson(doc, Serial);
      doc.clear();
      return true;
  }
  return false;
}

void pluviometer_read(unsigned long pl_currentMillis){
    if (pl_currentMillis - pl_previousMillis1Minute >= INTERVAL_1_MINUTE) {
        pl_previousMillis1Minute = pl_currentMillis;
        int reading_tmp;
        portENTER_CRITICAL_ISR(&spinlock);  
        reading_tmp = readings;
        readings = 0;
        // Calcular o volume atual com base nos pulsos
        PluvData.volume = reading_tmp * PLUV_VOL;
        portEXIT_CRITICAL_ISR(&spinlock);
        
        // Armazenar o volume no vetor de 1 minuto
        volume1Minute[pindex1Minute] = PluvData.volume;
        pindex1Minute = (pindex1Minute + 1) % pl_vINTERVAL_15_MINUTES;
        debug("Leituras:");
        debugln_(reading_tmp);
        //debug("INDICE:");
        //debugln_(pindex1Minute);
        // Calcular a soma dos volumes dos últimos 15 minutos
        if (pindex1Minute == 0) {      
            float sum15Minutes = calculateSum(volume1Minute, pl_vINTERVAL_15_MINUTES);
            volume15Minutes[pindex15Minutes] = sum15Minutes;    
            pindex15Minutes = (pindex15Minutes + 1) % pl_vINTERVAL_1_HOUR;            

            // Calcular a soma dos volumes da última 1 hora
            if (pindex15Minutes == 0) {
                float sum1Hour = calculateSum(volume15Minutes, pl_vINTERVAL_1_HOUR);
                volume1Hour[pindex1Hour] = sum1Hour; 
                pindex1Hour = (pindex1Hour + 1) % pl_vINTERVAL_1_DAY;

             /* // Calcular a soma dos volumes das últimas 24 horas
                if (pindex1Hour == 0) {
                PluvData.sum1Day = calculateSum(volume1Hour, pl_vINTERVAL_1_DAY);          
                } */
            }
        }
        portENTER_CRITICAL_ISR(&spinlock);
        PluvData.sum15 = calculateSum(volume1Minute, pl_vINTERVAL_15_MINUTES);
        // volume15Minutes[pindex15Minutes] = PluvData.sum15;
        PluvData.sum1Hour = calculateSum(volume15Minutes, pl_vINTERVAL_1_HOUR);
        // volume1Hour[pindex1Hour] = PluvData.sum1Hour;
        PluvData.sum1Day = calculateSum(volume1Hour, pl_vINTERVAL_1_DAY); 
        portEXIT_CRITICAL_ISR(&spinlock);
        pluviometer_saveConfig();
    }
}

#else

// TODO: Verify behave without deepsleep
void pluviometer_read(long pl_currentMillis){    
        /* #ifndef SET_DEEP_SLEEP_SECONDS_
        if (pl_currentMillis - pl_previousMillis1Minute >= INTERVAL_1_MINUTE) {
          pl_previousMillis1Minute = pl_currentMillis;      
          PluvData.volume = 0;
        }
        #endif */
        int reading_tmp;
       // debugln("Lendo Pluv...");
        portENTER_CRITICAL_ISR(&spinlock);  
        reading_tmp = readings;
        readings = 0;
        portEXIT_CRITICAL_ISR(&spinlock);         
        // Calcular o volume atual com base nos pulsos
        PluvData.volume += reading_tmp * PLUV_VOL; 
        // debugln_(PluvData.volume);
}
#endif


void pluviometer_AddReaging(){
    portENTER_CRITICAL_ISR(&spinlock);      
        readings++;
        DebounceTimer = millis();
    portEXIT_CRITICAL_ISR(&spinlock); 
}

// void RTC_IRAM_ATTR pulseCounter()
void IRAM_ATTR pulseCounter()
{
    portENTER_CRITICAL_ISR(&spinlock);  
    if ((millis() - DebounceTimer) >= delayTime ) {
        readings++;
        DebounceTimer = millis();
     }
    // debugln_(readings);    
    portEXIT_CRITICAL_ISR(&spinlock);    
}

void pluviometer_init(){
    // disableCore0WDT();
    //pinMode(PLUV_PIN, INPUT);
    pinMode(PLUV_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PLUV_PIN), pulseCounter, HIGH);
    // attachInterrupt(digitalPinToInterrupt(PLUV_PIN), pulseCounter, FALLING);
    #ifndef SIMPLE_READ
      pluviometer_loadConfig();
    #else
      PluvData.volume = 0;
    #endif
    debugln("SETUP PLUVIOMETER");
}
