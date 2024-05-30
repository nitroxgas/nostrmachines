
#include "sensors/lidar_tfminiplus.h"

#include <TFMiniPlusConstants.h>
// #define RXD2 16
// #define TXD2 17

TFMiniPlus tfmini;

float sum15 = 0;
float sum1Hour = 0;
float sum1Day = 0;

int count15 = 0;
int count1Hour = 0;
int count1Day = 0;

unsigned long previousMillis15 = 0;
unsigned long previousMillis1Hour = 0;
unsigned long previousMillis1Day = 0;

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
  // power cyle - And make changes valid
  tfmini.saveSettings();
  Serial.println("SETUP LIDAR END");
}

void tfmini_read(unsigned long Lidar_currentMillis) {  
  // read the data frame sent by the mini
  // Enable readings
  tfmini.setEnabled(false); 
    if (tfmini.readData()) {
      LidarData.distance = tfmini.getDistance();
      LidarData.strength = tfmini.getSignalStrength();
      LidarData.temperature = tfmini.getSensorTemperature();   
      sum15 += LidarData.distance;
      count15++;
      sum1Hour += LidarData.distance;
      count1Hour++;
      sum1Day += LidarData.distance;
      count1Day++;

       // Verifica o intervalo de 15 minutos
      if (Lidar_currentMillis - previousMillis15 >= INTERVAL_15_MINUTES) {
        previousMillis15 = Lidar_currentMillis;
        LidarData.avg15 = round(sum15 / count15);
        sum15 = 0;
        count15 = 0;
      }
      
      // Verifica o intervalo de 1 hora
      if (Lidar_currentMillis - previousMillis1Hour >= INTERVAL_1_HOUR) {
        previousMillis1Hour = Lidar_currentMillis;
        LidarData.avg1Hour = round(sum1Hour / count1Hour);
        sum1Hour = 0;
        count1Hour = 0;
      }
      
      // Verifica o intervalo de 1 dia
      if (Lidar_currentMillis - previousMillis1Day >= INTERVAL_1_DAY) {
        previousMillis1Day = Lidar_currentMillis;
        LidarData.avg1Day = round(sum1Day / count1Day);
        sum1Day = 0;
        count1Day = 0;
      }
    }     
  // Disable readings, reduces temperature and readings buffer  
  tfmini.setEnabled(true); 
}