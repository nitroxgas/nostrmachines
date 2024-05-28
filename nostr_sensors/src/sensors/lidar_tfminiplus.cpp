
#include "sensors/lidar_tfminiplus.h"

#include <TFMiniPlusConstants.h>
// #define RXD2 16
// #define TXD2 17

TFMiniPlus tfmini;

void tfmini_init() {
  // Start serial port to communicate with the TFMini
  // Default baud rate is 115200  

  // Pass the Serial class initialized to the tfmini
  #ifdef LIDAR_SERIAL1
    Serial1.begin(115200);
    tfmini.begin(&Serial1);
  #elif defined(LIDAR_SERIAL2)
    Serial2.begin(115200);
    tfmini.begin(&Serial2); 
  #elif
    tfmini.begin(&Serial);
  #endif
  Serial.println("SETUP LIDAR START");
  // Get firmware version
  Serial.println("Versão: "+tfmini.getVersion());

  // System Reset
  // tfmini.systemReset();

  // Set the data frame rate
  tfmini.setFrameRate(1);

  // If frame rate is 0 use this method to trigger a data frame
  // tfmini.triggerDetection();

  // Set ouput distance format to Centimiters
  // tfmini.setMeasurementTo(TFMINI_MEASUREMENT_CM);

  // Set output distance format to Milimeters
  // tfmini.setMeasurementTo(TFMINI_MEASUREMENT_MM);

  // Set baud rate "Only standard baud rates are supported"
  //tfmini.setBaudRate(115200);


  // Disable the device
  // tfmini.setEnabled(false);
 
  // Enable the device
  // tfmini.setEnabled(true);

  // Restore to factory settings
  // tfmini.restoreFactorySettings();

  // Persist configuration into the device otherwise will be reset with the next
  // power cyle
//  tfmini.saveSettings();
  Serial.println("SETUP LIDAR END");
}

String tfmini_read() {  
  // read the data frame sent by the mini
  try {
    if (tfmini.readData()) {
      char values[20];      
      // Distance "default in CM":Sensor tempreture in celsius:Signal Strength
      sprintf(values, "D:%d T:%.2f S:%d",tfmini.getDistance(), tfmini.getSensorTemperature(), tfmini.getSignalStrength());
      return values;
    } else {
      return "NO DATA";
    }
  }
   catch(...)
   {    
    return "LIDAR ERROR";
   }   
}