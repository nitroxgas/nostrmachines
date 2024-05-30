
#include "sensors/lidar_tfminiplusnolib.h"

// #define RXD2 16
// #define TXD2 17

TLidarData LidarData;

void tfmininl_init() {
  // Start serial port to communicate with the TFMini
  // Default baud rate is 115200    
  #ifdef LIDAR_SERIAL1
    Serial1.begin(115200, SERIAL_8N1, RXD2, TXD2);
  #elif defined(LIDAR_SERIAL2)
    Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);    
  #endif  
}

void tfmininl_read() {  
  // read the data frame sent by the mini
  
      uint8_t buf[9] = {0}; // An array that holds data
      if (Serial2.available() > 0) {
        Serial2.readBytes(buf, 9); // Read 9 bytes of data
      if( buf[0] == 0x59 && buf[1] == 0x59)
      {
        LidarData.distance = buf[2] + buf[3] * 256;
        LidarData.strength = buf[4] + buf[5] * 256;
        LidarData.temperature = (buf[6] + buf[7] * 256)  / 8.0 - 256.0;       
      }        
    }      
}