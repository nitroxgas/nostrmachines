#define ESP_DRD_USE_SPIFFS true

#include "storage.h"

bool SPIFFS_Initialized_;

/// @brief Prepare and mount SPIFFS
/// @return true on success
bool init_spiffs()
{
    if (!SPIFFS_Initialized_)
    {
        debugln("SPIFS: Mounting File System...");
        // May need to make it begin(true) first time you are using SPIFFS
        SPIFFS_Initialized_ = SPIFFS.begin(false) || SPIFFS.begin(true);
        // SPIFFS_Initialized_ ? debugln("SPIFS: Mounted") : debugln("SPIFS: Mounting failed.");
    }
    else
    {
        debugln("SPIFS: Already Mounted");
    }
    return SPIFFS_Initialized_;
};