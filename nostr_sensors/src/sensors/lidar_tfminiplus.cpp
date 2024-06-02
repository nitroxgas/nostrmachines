
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
  Serial.println("SETUP LIDAR END");

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

        #ifdef DEBUG_PRINT_SENSOR
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
          Serial.print("Dados Rio:");
          tfmini_PrintJson();
        #endif

      }
    }     
  // Disable readings, reduces temperature and readings buffer  
  tfmini.setEnabled(true); 
}