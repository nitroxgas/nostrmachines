
#include "sensors/temp_dht22.h"

#ifndef DHTPIN
    #define DHTPIN 33  
#endif
#define DHTTYPE DHT22

// Inicializa o sensor DHT
DHT dht(DHTPIN, DHTTYPE);

TDHTData DhtData;


unsigned long dpreviousMillis1Minute = 0;

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

void dht22_init() {  
    dht.begin();
    memset(temp1Minute, 0, sizeof(temp1Minute));
    memset(humidity1Minute, 0, sizeof(humidity1Minute));
    memset(temp15Minutes, 0, sizeof(temp15Minutes));
    memset(humidity15Minutes, 0, sizeof(humidity15Minutes));
    memset(temp1Hour, 0, sizeof(temp1Hour));
    memset(humidity1Hour, 0, sizeof(humidity1Hour)); 
    Serial.println("SETUP DHT22");   
    DhtData.temperature = 0;
    DhtData.humidity = 0;
    DhtData.tavg15 = 0;
    DhtData.tavg1Hour = 0;
    DhtData.tavg1Day = 0;
    DhtData.havg15 = 0;
    DhtData.havg1Hour = 0;
    DhtData.havg1Day = 0;
}

bool dht22_PrintJson(bool saveconfig){
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
        File configFile = SPIFFS.open("/dht22.json", "w");
        if (!configFile)
        {
            // Error, file did not open
            Serial.println("DHT22: Failed to open config file for writing");
            return false;
        }
        if (serializeJson(doc, configFile) == 0)
        {
            // Error writing file
            Serial.println(F("DHT22: Failed to write to file"));
            return false;
        }
        // Close file
        configFile.close();
        Serial.println(F("DHT22: Success to write to file"));
        return true;
     }
   } else {    
    serializeJson(doc, Serial);
    Serial.println();
   }
  return false;
}

void dht22_read(unsigned long dht22_currentMillis) {  
  if (dht22_currentMillis - dpreviousMillis1Minute >= INTERVAL_1_MINUTE) {
    dpreviousMillis1Minute = dht22_currentMillis;
    
    // Ler os dados do sensor DHT22
    float temp = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temp) || isnan(humidity)) {
      Serial.println("Failed to read from DHT sensor!");
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
      dht22_PrintJson(true);

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
    DhtData.temperature = temp;
    DhtData.humidity = humidity;
    DhtData.tavg15 = calculateAverage(temp1Minute, vINTERVAL_15_MINUTES);
    DhtData.tavg1Hour = calculateAverage(temp15Minutes, vINTERVAL_1_HOUR);
    DhtData.tavg1Day = calculateAverage(temp1Hour, vINTERVAL_1_DAY);
    DhtData.havg15 = calculateAverage(humidity1Minute, vINTERVAL_15_MINUTES);
    DhtData.havg1Hour = calculateAverage(humidity15Minutes, vINTERVAL_1_HOUR);
    DhtData.havg1Day = calculateAverage(humidity1Hour, vINTERVAL_1_DAY);
  }
}