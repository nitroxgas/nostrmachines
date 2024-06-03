#define ESP_DRD_USE_SPIFFS true

#include "storage.h"

bool SPIFFS_Initialized_;

/// @brief Prepare and mount SPIFFS
/// @return true on success
bool init_spiffs()
{
    if (!SPIFFS_Initialized_)
    {
        Serial.println("SPIFS: Mounting File System...");
        // May need to make it begin(true) first time you are using SPIFFS
        SPIFFS_Initialized_ = SPIFFS.begin(false) || SPIFFS.begin(true);
        SPIFFS_Initialized_ ? Serial.println("SPIFS: Mounted") : Serial.println("SPIFS: Mounting failed.");
    }
    else
    {
        Serial.println("SPIFS: Already Mounted");
    }
    return SPIFFS_Initialized_;
};