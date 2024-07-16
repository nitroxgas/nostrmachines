// Include Libraries
//#include ".h"

#include <WiFi.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include "storage.h"
#include "wManager.h"
#include "global.h"

// Flag for saving data
bool shouldSaveConfig = false;

// Variables to hold data from custom textboxes
TSettings Settings;

// Define WiFiManager Object
WiFiManager wm;


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
                debugln("SPIFS: Loading config file");
                StaticJsonDocument<512> json;
                DeserializationError error = deserializeJson(json, configFile);
                configFile.close();
                // serializeJsonPretty(json, Serial);
                debugln(" ");
                if (!error)
                {
                    Settings->privkey       = json[JSON_KEY_PRIV]     | Settings->privkey;
                    Settings->pubkey        = json[JSON_KEY_PUB]      | Settings->pubkey;
                    Settings->nrelays       = json[JSON_KEY_RELAYS]   | Settings->nrelays;
                    Settings->masterpub     = json[JSON_KEY_MASTERPUB]| Settings->masterpub;
                    Settings->name          = json[JSON_KEY_NAME]     | Settings->name;               
                    if (json.containsKey(JSON_KEY_TIMEZONE))
                        Settings->Timezone = json[JSON_KEY_TIMEZONE].as<int>();
                    if (json.containsKey(JSON_KEY_ZAPVALUE))
                        Settings->zapvalue = json[JSON_KEY_ZAPVALUE].as<long>();  
                    #ifdef SET_DEEP_SLEEP_SECONDS
                    if (json.containsKey(JSON_KEY_SLEEP))
                        Settings->sleepsec = json[JSON_KEY_SLEEP].as<int>(); 
                    #endif                                      
                    return true;
                }
                else
                {
                    // Error loading JSON data
                    debugln("SPIFS: Error parsing config file!");
                }
                json.clear();
            }
            else
            {
                debugln("SPIFS: Error opening config file!");
            }
        }
        else
        {
            debugln("SPIFS: No config file available!");
        }
    }
    return false;
}

/// @brief Delete config file from SPIFFS
/// @return true on successs
bool deleteConfig()
{
    debugln("SPIFS: Erasing config file..");
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
        debugln(F("SPIFS: Saving configuration."));

        // Create a JSON document
        StaticJsonDocument<512> json;
        json[JSON_KEY_PRIV] = Settings->privkey;
        json[JSON_KEY_PUB] = Settings->pubkey;
        json[JSON_KEY_RELAYS] = Settings->nrelays;
        json[JSON_KEY_MASTERPUB] = Settings->masterpub;
        json[JSON_KEY_TIMEZONE] = Settings->Timezone;
        json[JSON_KEY_ZAPVALUE] = Settings->zapvalue;
        json[JSON_KEY_NAME] = Settings->name;
        #ifdef SET_DEEP_SLEEP_SECONDS        
        json[JSON_KEY_SLEEP] = Settings->sleepsec;
        #endif 

        
        // Open config file
        File configFile = SPIFFS.open(JSON_CONFIG_FILE, "w");
        if (!configFile)
        {
            // Error, file did not open
            debugln("SPIFS: Failed to open config file for writing");
            return false;
        }

        // Serialize JSON data to write to file
        // serializeJsonPretty(json, Serial);
        debugln(" ");
        if (serializeJson(json, configFile) == 0)
        {
            // Error writing file
            debugln("SPIFS: Failed to write to file");
            return false;
        }
        // Close file
        configFile.close();
        return true;
    }
    return false;
}

void saveConfigCallback()
// Callback notifying us of the need to save configuration
{
    debugln("Should save config");
    shouldSaveConfig = true;    
    //wm.setConfigPortalBlocking(false);
}

/* void saveParamsCallback()
// Callback notifying us of the need to save configuration
{
    debugln("Should save config");
    shouldSaveConfig = true;
    nvMem.saveConfig(&Settings);
} */

void configModeCallback(WiFiManager* myWiFiManager)
// Called when config mode launched
{
    debugln("Entered Configuration Mode");    
    debug("Config SSID: ");
    debugf("%c", myWiFiManager->getConfigPortalSSID());
    debug("Config IP Address: ");
    debugf("%c",WiFi.softAPIP());
}

void reset_configuration()
{
    debugln("Erasing Config, restarting");
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
        debugln("Button pressed to force start config mode");
        forceConfig = true;
        wm.setBreakAfterConfig(true); //Set to detect config edition and save
    }
#endif
    // Explicitly set WiFi mode
    WiFi.mode(WIFI_STA);

    if (!loadConfig(&Settings))
    {            
        //No config file. Starting wifi config server.
        forceConfig = true;            
    }

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
    WiFiManagerParameter name_text_box("nameStr", "Nostr Sensor Name", Settings.name.c_str(), 80);
    WiFiManagerParameter privkey_text_box("nstpPriv", "Nostr Private Key", Settings.privkey.c_str(), 80);
    WiFiManagerParameter pubkey_text_box("nstpPub", "Nostr Public Key", Settings.pubkey.c_str(), 80);
    WiFiManagerParameter nrelays_text_box("nstpRelays", "Nostr Relays", Settings.nrelays.c_str(), 150);
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
    wm.addParameter(&name_text_box);
    wm.addParameter(&privkey_text_box);
    wm.addParameter(&pubkey_text_box);
    wm.addParameter(&nrelays_text_box);
    wm.addParameter(&master_text_box);
    wm.addParameter(&zap_text_box_num);
    wm.addParameter(&time_text_box_num);
    
    #ifdef SET_DEEP_SLEEP_SECONDS
    sprintf(charZone, "%d", Settings.sleepsec);
    WiFiManagerParameter sleep_text_box_num("SleepSec", "Seconds to sleep", charZone, 3);
    wm.addParameter(&sleep_text_box_num);
    #endif

    debugln("AllDone: ");
    if (forceConfig)    
    {
        // Run if we need a configuration
        //No configuramos timeout al modulo
        wm.setConfigPortalBlocking(true); //Hacemos que el portal SI bloquee el firmware        
        if (!wm.startConfigPortal(DEFAULT_SSID, DEFAULT_WIFIPW))
        {
            //Could be break forced after edditing, so save new config
            debugln("failed to connect and hit timeout");
            Settings.name    = name_text_box.getValue();
            Settings.privkey = privkey_text_box.getValue();            
            Settings.privkey = privkey_text_box.getValue();
            Settings.pubkey  = pubkey_text_box.getValue();
            Settings.nrelays = nrelays_text_box.getValue();
            Settings.Timezone = atoi(time_text_box_num.getValue());
            Settings.zapvalue = atoi(zap_text_box_num.getValue());  
            #ifdef SET_DEEP_SLEEP_SECONDS
            Settings.sleepsec = atoi(sleep_text_box_num.getValue());
            #endif          
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
            debugln("Failed to connect to configured WIFI, and hit timeout");
            if (shouldSaveConfig) {
                // Save new config            
                Settings.name    = name_text_box.getValue();
                Settings.privkey = privkey_text_box.getValue();            
                Settings.privkey = privkey_text_box.getValue();
                Settings.pubkey  = pubkey_text_box.getValue();
                Settings.nrelays = nrelays_text_box.getValue();
                Settings.Timezone = atoi(time_text_box_num.getValue());
                #ifdef SET_DEEP_SLEEP_SECONDS
                Settings.sleepsec = atoi(sleep_text_box_num.getValue());
                #endif
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
        debugln("");
        debugln("WiFi connected");
        debug("IP address: ");
        debugln_(WiFi.localIP().toString());
        debugln_(WiFi.macAddress());

        // Lets deal with the user config values
        // Copy the string value
        Settings.name    = name_text_box.getValue();
        Settings.privkey = privkey_text_box.getValue();            
        Settings.privkey = privkey_text_box.getValue();
        Settings.pubkey  = pubkey_text_box.getValue();
        Settings.nrelays = nrelays_text_box.getValue();
        Settings.Timezone = atoi(time_text_box_num.getValue());
        Settings.zapvalue = atol(zap_text_box_num.getValue());
        Settings.macaddr = WiFi.macAddress();
        #ifdef SET_DEEP_SLEEP_SECONDS
        Settings.sleepsec = atoi(sleep_text_box_num.getValue());
        #endif

        debug("Sensor Name: ");
        debugln_(Settings.name);

        debug("Private Key: ");
        debugln_(Settings.privkey);
                
        debug("Public Key: ");
        debugln_(Settings.pubkey);
        
        debug("Relays: ");
        debugln_(Settings.nrelays);
        
        debug("Master Pub Key: ");
        debugln_(Settings.masterpub);
                
        debug("TimeZone fromUTC: ");
        debugln_(Settings.Timezone);

        #ifdef SET_DEEP_SLEEP_SECONDS
        debug("Sleep Seconds: ");
        debugln_(Settings.sleepsec);
        #endif
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
            debugf("CONNECTED - Current ip: %c" , WiFi.localIP().toString());
        } else {
            debug("[Error] - current status: ");
            debugf("%d", newStatus);
        }
        oldStatus = newStatus;
    }
}
