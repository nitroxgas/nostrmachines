
#include "sensors/ultrasonic.h"

#ifndef DHTPIN
    #define DHTPIN 33  
#endif

#ifndef DHTTYPE
  #define DHTTYPE USs
#endif



TUSData USData;

long uspreviousMillis1Minute = 0;

#ifndef SIMPLE_READ
// Vetores para armazenar as leituras
float dist1Minute[vINTERVAL_15_MINUTES];
float dist15Minutes[vINTERVAL_1_HOUR];
float dist1Hour[vINTERVAL_1_DAY];


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

void ultrasonic_init() {  
    dht.begin();
    memset(dist1Minute, 0, sizeof(dist1Minute));    
    memset(dist15Minutes, 0, sizeof(dist15Minutes));    
    memset(dist1Hour, 0, sizeof(dist1Hour));    
    debugln("SETUP ULTRASONIC");   
    USData.disterature = 0;    
    USData.davg15 = 0;
    USData.davg1Hour = 0;
    USData.davg1Day = 0;   
}

bool ultrasonic_PrintJson(bool saveconfig){
    StaticJsonDocument<1024> doc;
    JsonArray arraydist1Minute = doc.createNestedArray("dist_1_minute");    
    JsonArray arraydist15Minutes = doc.createNestedArray("dist_15_minutes");
    JsonArray arraydist1Hour = doc.createNestedArray("dist_1_hour");

    for (int i = 0; i < vINTERVAL_15_MINUTES; i++) {
      arraydist1Minute.add(dist1Minute[i]);
      arrayHumidity1Minute.add(humidity1Minute[i]);
    }

    for (int i = 0; i < vINTERVAL_1_HOUR; i++) {
      arraydist15Minutes.add(dist15Minutes[i]);
      arrayHumidity15Minutes.add(humidity15Minutes[i]);
    }

    for (int i = 0; i < vINTERVAL_1_DAY; i++) {
      arraydist1Hour.add(dist1Hour[i]);
      arrayHumidity1Hour.add(humidity1Hour[i]);
    }
    if (saveconfig){
      if (init_spiffs()){
        File configFile = SPIFFS.open("/ultrasonic.json", "w");
        if (!configFile)
        {
            // Error, file did not open
            debugln("USs: Failed to open config file for writing");
            return false;
        }
        if (serializeJson(doc, configFile) == 0)
        {
            // Error writing file
            debugln(F("USs: Failed to write to file"));
            return false;
        }
        // Close file
        configFile.close();
        debugln(F("USs: Success to write to file"));
        return true;
     }
   } else {    
    serializeJson(doc, Serial);
    debugln(" ");
   }
  return false;
}

void ultrasonic_read(unsigned long ultrasonic_currentMillis) {  
  if (ultrasonic_currentMillis - dpreviousMillis1Minute >= INTERVAL_1_MINUTE) {
    dpreviousMillis1Minute = ultrasonic_currentMillis;
    debugln("US sensor!");
    // Ler os dados do sensor USs
    float dist = dht.readdisterature();
    float humidity = dht.readHumidity();

    if (isnan(dist) || isnan(humidity)) {
      debugln("Failed to read from US sensor!");
      USData.disterature = 0;
      USData.humidity = 0;
      return;
    }

    // Armazenar os dados nos vetores de 1 minuto
    dist1Minute[dindex1Minute] = dist;
    humidity1Minute[dindex1Minute] = humidity;
    dindex1Minute = (dindex1Minute + 1) % vINTERVAL_15_MINUTES;

    // Calcular a média das leituras de 15 minutos
    if (dindex1Minute == 0) {
      float avgdist15 = calculateAverage(dist1Minute, vINTERVAL_15_MINUTES);
      float avgHumidity15 = calculateAverage(humidity1Minute, vINTERVAL_15_MINUTES);
      dist15Minutes[dindex15Minutes] = avgdist15;
      humidity15Minutes[dindex15Minutes] = avgHumidity15;
      dindex15Minutes = (dindex15Minutes + 1) % vINTERVAL_1_HOUR;
      ultrasonic_PrintJson(true);

      // Calcular a média das leituras de 1 hora
      if (dindex15Minutes == 0) {
        float avgdist1Hour = calculateAverage(dist15Minutes, vINTERVAL_1_HOUR);
        float avgHumidity1Hour = calculateAverage(humidity15Minutes, vINTERVAL_1_HOUR);
        dist1Hour[dindex1Hour] = avgdist1Hour;
        humidity1Hour[dindex1Hour] = avgHumidity1Hour;
        dindex1Hour = (dindex1Hour + 1) % vINTERVAL_1_DAY;

        /* // Calcular a média das leituras de 1 dia
        if (dindex1Hour == 0) {
          float avgdist1Day = calculateAverage(dist1Hour, vINTERVAL_1_DAY);
          float avgHumidity1Day = calculateAverage(humidity1Hour, vINTERVAL_1_DAY);                    
        } */
      }
    }
    USData.disterature = dist;    
    USData.davg15 = calculateAverage(dist1Minute, vINTERVAL_15_MINUTES);
    USData.davg1Hour = calculateAverage(dist15Minutes, vINTERVAL_1_HOUR);
    USData.davg1Day = calculateAverage(dist1Hour, vINTERVAL_1_DAY);    
  }
}
#else
void ultrasonic_init() {  
   // dht.begin();       
    debugln("SETUP USs");   
    USData.distance = 0;
    
}

void ultrasonic_read(long ultrasonic_currentMillis) {  
  if (ultrasonic_currentMillis - uspreviousMillis1Minute >= INTERVAL_1_MINUTE) {
    uspreviousMillis1Minute = ultrasonic_currentMillis;
    debugln("DHT sensor!");
    // Ler os dados do sensor USs
    float dist = 0; //dht.readdisterature();
 //   float humidity = dht.readHumidity();

    if (isnan(dist)) {
      debugln("Failed to read from US sensor!");
      USData.distance = 0;       
      return;
    }
    USData.distance = dist;    
  }
}
#endif