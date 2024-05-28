#define ESP_DRD_USE_SPIFFS true

// Include Libraries
//#include ".h"

#include <WiFi.h>
#include <WiFiManager.h>
#include <SPIFFS.h>
#include <FS.h>
#include <ArduinoJson.h>

#include "wManager.h"
#include "global.h"

// Flag for saving data
bool shouldSaveConfig = false;

// Variables to hold data from custom textboxes
TSettings Settings;

// Define WiFiManager Object
WiFiManager wm;

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

/// @brief Load settings from config file located in SPIFFS.
/// @param TSettings* Struct to update with new settings.
/// @return true on success
bool loadConfig(TSettings* Settings)
{
    // Uncomment if we need to format filesystem
    // SPIFFS.format();

    // Load existing configuration file
    // Read configuration from FS json

    if (init_spiffs())
    {
        if (SPIFFS.exists(JSON_CONFIG_FILE))
        {
            // The file exists, reading and loading
            File configFile = SPIFFS.open(JSON_CONFIG_FILE, "r");
            if (configFile)
            {
                Serial.println("SPIFS: Loading config file");
                StaticJsonDocument<512> json;
                DeserializationError error = deserializeJson(json, configFile);
                configFile.close();
                serializeJsonPretty(json, Serial);
                Serial.print('\n');
                if (!error)
                {
                    Settings->privkey       = json[JSON_KEY_PRIV]     | Settings->privkey;
                    Settings->pubkey        = json[JSON_KEY_PUB]      | Settings->pubkey;
                    Settings->nrelays       = json[JSON_KEY_RELAYS]   | Settings->nrelays;
                    Settings->masterpub     = json[JSON_KEY_MASTERPUB]| Settings->masterpub;                    
                    if (json.containsKey(JSON_KEY_TIMEZONE))
                        Settings->Timezone = json[JSON_KEY_TIMEZONE].as<int>();
                    if (json.containsKey(JSON_KEY_ZAPVALUE))
                        Settings->zapvalue = json[JSON_KEY_ZAPVALUE].as<long>();                    
                    return true;
                }
                else
                {
                    // Error loading JSON data
                    Serial.println("SPIFS: Error parsing config file!");
                }
            }
            else
            {
                Serial.println("SPIFS: Error opening config file!");
            }
        }
        else
        {
            Serial.println("SPIFS: No config file available!");
        }
    }
    return false;
}

/// @brief Delete config file from SPIFFS
/// @return true on successs
bool deleteConfig()
{
    Serial.println("SPIFS: Erasing config file..");
    return SPIFFS.remove(JSON_CONFIG_FILE); //Borramos fichero
}

/// @brief Save settings to config file on SPIFFS
/// @param TSettings* Settings to be saved.
/// @return true on success
bool saveConfig(TSettings* Settings)
{
    if (init_spiffs())
    {
        // Save Config in JSON format
        Serial.println(F("SPIFS: Saving configuration."));

        // Create a JSON document
        StaticJsonDocument<512> json;
        json[JSON_KEY_PRIV] = Settings->privkey;
        json[JSON_KEY_PUB] = Settings->pubkey;
        json[JSON_KEY_RELAYS] = Settings->nrelays;
        json[JSON_KEY_MASTERPUB] = Settings->masterpub;
        json[JSON_KEY_TIMEZONE] = Settings->Timezone;
        json[JSON_KEY_ZAPVALUE] = Settings->zapvalue;
        
        // Open config file
        File configFile = SPIFFS.open(JSON_CONFIG_FILE, "w");
        if (!configFile)
        {
            // Error, file did not open
            Serial.println("SPIFS: Failed to open config file for writing");
            return false;
        }

        // Serialize JSON data to write to file
        serializeJsonPretty(json, Serial);
        Serial.print('\n');
        if (serializeJson(json, configFile) == 0)
        {
            // Error writing file
            Serial.println(F("SPIFS: Failed to write to file"));
            return false;
        }
        // Close file
        configFile.close();
        return true;
    };
    return false;
}

void saveConfigCallback()
// Callback notifying us of the need to save configuration
{
    Serial.println("Should save config");
    shouldSaveConfig = true;    
    //wm.setConfigPortalBlocking(false);
}

/* void saveParamsCallback()
// Callback notifying us of the need to save configuration
{
    Serial.println("Should save config");
    shouldSaveConfig = true;
    nvMem.saveConfig(&Settings);
} */

void configModeCallback(WiFiManager* myWiFiManager)
// Called when config mode launched
{
    Serial.println("Entered Configuration Mode");    
    Serial.print("Config SSID: ");
    Serial.println(myWiFiManager->getConfigPortalSSID());
    Serial.print("Config IP Address: ");
    Serial.println(WiFi.softAPIP());
}

void reset_configuration()
{
    Serial.println("Erasing Config, restarting");
    deleteConfig();
    wm.resetSettings();
    ESP.restart();
}

void init_WifiManager()
{
#ifdef MONITOR_SPEED
    Serial.begin(MONITOR_SPEED);
#else
    Serial.begin(115200);
#endif //MONITOR_SPEED
    //Serial.setTxTimeoutMs(10);

    //Init pin 15 to eneble 5V external power (LilyGo bug)
#ifdef PIN_ENABLE5V
    pinMode(PIN_ENABLE5V, OUTPUT);
    digitalWrite(PIN_ENABLE5V, HIGH);
#endif

    // Change to true when testing to force configuration every time we run
    bool forceConfig = false;

#if defined(PIN_BUTTON_2)
    // Check if button2 is pressed to enter configMode with actual configuration
    if (!digitalRead(PIN_BUTTON_2)) {
        Serial.println(F("Button pressed to force start config mode"));
        forceConfig = true;
        wm.setBreakAfterConfig(true); //Set to detect config edition and save
    }
#endif
    // Explicitly set WiFi mode
    WiFi.mode(WIFI_STA);

    // Reset settings (only for development)
    //wm.resetSettings();

    //Set dark theme
    //wm.setClass("invert"); // dark theme

    // Set config save notify callback
    wm.setSaveConfigCallback(saveConfigCallback);
    wm.setSaveParamsCallback(saveConfigCallback);

    // Set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
    wm.setAPCallback(configModeCallback);    

    //Advanced settings
    wm.setConfigPortalBlocking(false); //Hacemos que el portal no bloquee el firmware
    wm.setConnectTimeout(40); // how long to try to connect for before continuing
    wm.setConfigPortalTimeout(180); // auto close configportal after n seconds
    // wm.setCaptivePortalEnable(false); // disable captive portal redirection
    // wm.setAPClientCheck(true); // avoid timeout if client connected to softap
    //wm.setTimeout(120);
    //wm.setConfigPortalTimeout(120); //seconds

    // Custom elements

    // Text box (String) - 80 characters maximum
    WiFiManagerParameter privkey_text_box("nstpPriv", "Nostr Private Key", Settings.privkey.c_str(), 80);
    WiFiManagerParameter pubkey_text_box("nstpPub", "Nostr Public Key", Settings.pubkey.c_str(), 80);
    WiFiManagerParameter nrelays_text_box("nstpRalays", "Nostr Relays", Settings.nrelays.c_str(), 150);
    WiFiManagerParameter master_text_box("nstpMaster", "Nostr Master Pub Key", Settings.privkey.c_str(), 80);
    // Text box (Number) - 2 characters maximum
    char charZap[8];
    sprintf(charZap, "%d", Settings.zapvalue);
    WiFiManagerParameter zap_text_box_num("zapValue", "Zap Value", charZap, 8);

    // Text box (Number) - 2 characters maximum
    char charZone[6];
    sprintf(charZone, "%d", Settings.Timezone);
    WiFiManagerParameter time_text_box_num("TimeZone", "TimeZone fromUTC (-12/+12)", charZone, 3);

    // Add all defined parameters
    wm.addParameter(&privkey_text_box);
    wm.addParameter(&pubkey_text_box);
    wm.addParameter(&nrelays_text_box);
    wm.addParameter(&master_text_box);
    wm.addParameter(&zap_text_box_num);
    wm.addParameter(&time_text_box_num);

    Serial.println("AllDone: ");
    if (forceConfig)    
    {
        // Run if we need a configuration
        //No configuramos timeout al modulo
        wm.setConfigPortalBlocking(true); //Hacemos que el portal SI bloquee el firmware        
        if (!wm.startConfigPortal(DEFAULT_SSID, DEFAULT_WIFIPW))
        {
            //Could be break forced after edditing, so save new config
            Serial.println("failed to connect and hit timeout");
            Settings.privkey = privkey_text_box.getValue();            
            Settings.privkey = privkey_text_box.getValue();
            Settings.pubkey  = pubkey_text_box.getValue();
            Settings.nrelays = nrelays_text_box.getValue();
            Settings.Timezone = atoi(time_text_box_num.getValue());
            Settings.zapvalue = atoi(zap_text_box_num.getValue());            
            saveConfig(&Settings);
            delay(3000);
            //reset and try again, or maybe put it to deep sleep
            ESP.restart();            
        };
    }
    else
    {        
        // disable captive portal redirection
        wm.setCaptivePortalEnable(true); 
        wm.setConfigPortalBlocking(true);
        wm.setEnableConfigPortal(true);
        // if (!wm.autoConnect(Settings.WifiSSID.c_str(), Settings.WifiPW.c_str()))
        if (!wm.autoConnect(DEFAULT_SSID, DEFAULT_WIFIPW))
        {
            Serial.println("Failed to connect to configured WIFI, and hit timeout");
            if (shouldSaveConfig) {
                // Save new config            
                Settings.privkey = privkey_text_box.getValue();            
                Settings.privkey = privkey_text_box.getValue();
                Settings.pubkey  = pubkey_text_box.getValue();
                Settings.nrelays = nrelays_text_box.getValue();
                Settings.Timezone = atoi(time_text_box_num.getValue());
                Settings.zapvalue = atoi(zap_text_box_num.getValue()); 
                saveConfig(&Settings);
                vTaskDelay(2000 / portTICK_PERIOD_MS);      
            }        
            ESP.restart();                            
        } 
    }
    
    //Conectado a la red Wifi
    if (WiFi.status() == WL_CONNECTED) {
        //tft.pushImage(0, 0, MinerWidth, MinerHeight, MinerScreen);
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());

        // Lets deal with the user config values
        // Copy the string value
        Settings.privkey = privkey_text_box.getValue();            
        Settings.privkey = privkey_text_box.getValue();
        Settings.pubkey  = pubkey_text_box.getValue();
        Settings.nrelays = nrelays_text_box.getValue();
        Settings.Timezone = atoi(time_text_box_num.getValue());
        Settings.zapvalue = atol(zap_text_box_num.getValue());

        Serial.print("Private Key: ");
        Serial.println(Settings.privkey);
                
        Serial.print("Public Key: ");
        Serial.println(Settings.pubkey);
        
        Serial.print("Relays: ");
        Serial.println(Settings.nrelays);
        
        Serial.print("Master Pub Key: ");
        Serial.println(Settings.masterpub);
                
        Serial.print("TimeZone fromUTC: ");
        Serial.println(Settings.Timezone);
    }

    // Save the custom parameters to FS
    if (shouldSaveConfig)
    {
        saveConfig(&Settings);        
    }
}

//----------------- MAIN PROCESS WIFI MANAGER --------------
int oldStatus = 0;

void wifiManagerProcess() {

    wm.process(); // avoid delays() in loop when non-blocking and other long running code

    int newStatus = WiFi.status();
    if (newStatus != oldStatus) {
        if (newStatus == WL_CONNECTED) {
            Serial.println("CONNECTED - Current ip: " + WiFi.localIP().toString());
        } else {
            Serial.print("[Error] - current status: ");
            Serial.println(newStatus);
        }
        oldStatus = newStatus;
    }
}
