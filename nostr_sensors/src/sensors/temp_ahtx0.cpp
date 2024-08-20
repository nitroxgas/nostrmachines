
#include "sensors/temp_ahtx0.h"

TAHT0XData AhtData;

Adafruit_AHTX0 aht;
Adafruit_Sensor *aht_humidity, *aht_temp;
bool aht0x_started = false;
long apreviousMillis1Minute = 0;

#ifndef SIMPLE_READ
// Vetores para armazenar as leituras
float temp1Minute[vINTERVAL_15_MINUTES];
float humidity1Minute[vINTERVAL_15_MINUTES];
float temp15Minutes[vINTERVAL_1_HOUR];
float humidity15Minutes[vINTERVAL_1_HOUR];
float temp1Hour[vINTERVAL_1_DAY];
float humidity1Hour[vINTERVAL_1_DAY];

// Índices para controle de inserção nos vetores
int dindex1Minute = 0;
int dindex15Minutes = 0;
int dindex1Hour = 0;

float calculateAverage(float* array, int size) {
  float sum = 0;
  int valids = 0;
  for (int i = 0; i < size; i++) {
    sum += array[i];
    if (array[i]!=0) valids++;
  }
  if (valids==0) return 0;
  return sum / valids;
}

void aht0x_init() {  
    dht.begin();
    memset(temp1Minute, 0, sizeof(temp1Minute));
    memset(humidity1Minute, 0, sizeof(humidity1Minute));
    memset(temp15Minutes, 0, sizeof(temp15Minutes));
    memset(humidity15Minutes, 0, sizeof(humidity15Minutes));
    memset(temp1Hour, 0, sizeof(temp1Hour));
    memset(humidity1Hour, 0, sizeof(humidity1Hour)); 
    debugln("SETUP aht0x");   
    AhtData.temperature = 0;
    AhtData.humidity = 0;
    AhtData.tavg15 = 0;
    AhtData.tavg1Hour = 0;
    AhtData.tavg1Day = 0;
    AhtData.havg15 = 0;
    AhtData.havg1Hour = 0;
    AhtData.havg1Day = 0;
}

bool aht0x_PrintJson(bool saveconfig){
    StaticJsonDocument<1024> doc;
    JsonArray arrayTemp1Minute = doc.createNestedArray("temp_1_minute");    
    JsonArray arrayTemp15Minutes = doc.createNestedArray("temp_15_minutes");
    JsonArray arrayTemp1Hour = doc.createNestedArray("temp_1_hour");
    JsonArray arrayHumidity1Minute = doc.createNestedArray("humidity_1_minute");
    JsonArray arrayHumidity15Minutes = doc.createNestedArray("humidity_15_minutes");    
    JsonArray arrayHumidity1Hour = doc.createNestedArray("humidity_1_hour");

    for (int i = 0; i < vINTERVAL_15_MINUTES; i++) {
      arrayTemp1Minute.add(temp1Minute[i]);
      arrayHumidity1Minute.add(humidity1Minute[i]);
    }

    for (int i = 0; i < vINTERVAL_1_HOUR; i++) {
      arrayTemp15Minutes.add(temp15Minutes[i]);
      arrayHumidity15Minutes.add(humidity15Minutes[i]);
    }

    for (int i = 0; i < vINTERVAL_1_DAY; i++) {
      arrayTemp1Hour.add(temp1Hour[i]);
      arrayHumidity1Hour.add(humidity1Hour[i]);
    }
    if (saveconfig){
      if (init_spiffs()){
        File configFile = SPIFFS.open("/aht0x.json", "w");
        if (!configFile)
        {
            // Error, file did not open
            debugln("aht0x: Failed to open config file for writing");
            return false;
        }
        if (serializeJson(doc, configFile) == 0)
        {
            // Error writing file
            debugln(F("aht0x: Failed to write to file"));
            return false;
        }
        // Close file
        configFile.close();
        debugln(F("aht0x: Success to write to file"));
        return true;
     }
   } else {    
    serializeJson(doc, Serial);
    debugln(" ");
   }
  return false;
}

void aht0x_read(unsigned long aht0x_currentMillis) {  
  if (aht0x_currentMillis - apreviousMillis1Minute >= INTERVAL_1_MINUTE) {
    apreviousMillis1Minute = aht0x_currentMillis;
    debugln("DHT sensor!");
    // Ler os dados do sensor aht0x
    float temp = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temp) || isnan(humidity)) {
      debugln("Failed to read from DHT sensor!");
      AhtData.temperature = 0;
      AhtData.humidity = 0;
      return;
    }

    // Armazenar os dados nos vetores de 1 minuto
    temp1Minute[dindex1Minute] = temp;
    humidity1Minute[dindex1Minute] = humidity;
    dindex1Minute = (dindex1Minute + 1) % vINTERVAL_15_MINUTES;

    // Calcular a média das leituras de 15 minutos
    if (dindex1Minute == 0) {
      float avgTemp15 = calculateAverage(temp1Minute, vINTERVAL_15_MINUTES);
      float avgHumidity15 = calculateAverage(humidity1Minute, vINTERVAL_15_MINUTES);
      temp15Minutes[dindex15Minutes] = avgTemp15;
      humidity15Minutes[dindex15Minutes] = avgHumidity15;
      dindex15Minutes = (dindex15Minutes + 1) % vINTERVAL_1_HOUR;
      aht0x_PrintJson(true);

      // Calcular a média das leituras de 1 hora
      if (dindex15Minutes == 0) {
        float avgTemp1Hour = calculateAverage(temp15Minutes, vINTERVAL_1_HOUR);
        float avgHumidity1Hour = calculateAverage(humidity15Minutes, vINTERVAL_1_HOUR);
        temp1Hour[dindex1Hour] = avgTemp1Hour;
        humidity1Hour[dindex1Hour] = avgHumidity1Hour;
        dindex1Hour = (dindex1Hour + 1) % vINTERVAL_1_DAY;

        /* // Calcular a média das leituras de 1 dia
        if (dindex1Hour == 0) {
          float avgTemp1Day = calculateAverage(temp1Hour, vINTERVAL_1_DAY);
          float avgHumidity1Day = calculateAverage(humidity1Hour, vINTERVAL_1_DAY);                    
        } */
      }
    }
    AhtData.temperature = temp;
    AhtData.humidity = humidity;
    AhtData.tavg15 = calculateAverage(temp1Minute, vINTERVAL_15_MINUTES);
    AhtData.tavg1Hour = calculateAverage(temp15Minutes, vINTERVAL_1_HOUR);
    AhtData.tavg1Day = calculateAverage(temp1Hour, vINTERVAL_1_DAY);
    AhtData.havg15 = calculateAverage(humidity1Minute, vINTERVAL_15_MINUTES);
    AhtData.havg1Hour = calculateAverage(humidity15Minutes, vINTERVAL_1_HOUR);
    AhtData.havg1Day = calculateAverage(humidity1Hour, vINTERVAL_1_DAY);
  }
}
#else
void aht0x_init() {  
    debugln("SETUP AHT");   
    // Initialize AHT sensor
    if (aht.begin()) {
        debugln("AHT10/AHT20 Found!");
        aht_temp = aht.getTemperatureSensor();
        aht_temp->printSensorDetails();
        aht_humidity = aht.getHumiditySensor();
        aht_humidity->printSensorDetails();
        aht0x_started = true;
    } else {
        debugln("Failed to find AHT10/AHT20 chip");        
    }    
    AhtData.temperature = 0;
    AhtData.humidity = 0;    
}

void aht0x_read(long aht0x_currentMillis) {  
  if ((aht0x_currentMillis - apreviousMillis1Minute >= INTERVAL_1_MINUTE)&&(aht0x_started==true)) {
    apreviousMillis1Minute = aht0x_currentMillis;
    debugln("AHT sensor!");
    // Ler os dados do sensor aht0x
    sensors_event_t humidity;
    sensors_event_t temp;
    aht_humidity->getEvent(&humidity);
    aht_temp->getEvent(&temp);

    float temp_tmp = temp.temperature;
    float humidity_tmp = humidity.relative_humidity;

    if (isnan(temp_tmp) || isnan(humidity_tmp)) {
      debugln("Failed to read from AHT sensor!");
      AhtData.temperature = 0; 
      AhtData.humidity = 0;
      return;
    }
    AhtData.temperature = temp_tmp;
    AhtData.humidity = humidity_tmp;
  }
}
#endif