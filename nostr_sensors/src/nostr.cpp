
#include "nostr.h"
/* #include <Arduino.h>
#include "WiFiClientSecure.h"
#include "time.h"
#include <NostrEvent.h>
#include <NostrRelayManager.h>
#include <vector>

#include <NostrRequestOptions.h>
#include <Wire.h>
#include "Bitcoin.h"
#include "Hash.h"
#include <esp_random.h>
#include <math.h>
#include <ArduinoJson.h>

// freertos
#include "freertos/FreeRTOS.h"
#include "freertos/task.h" */

NostrEvent nostr;
NostrRelayManager nostrRelayManager;
NostrQueueProcessor nostrQueue;

bool hasSentEvent = false;

// NTP server to request epoch time
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -10800;
const int   daylightOffset_sec = 0;

bool lastInternetConnectionState = true;

String serialisedEventRequest;

bool hasInternetConnection = false;

NostrRequestOptions* eventRequestOptions;

bool isBuzzerEnabled = false;

int socketDisconnectedCount = 0;
int ledPin = 13; // Pin number where the LED is connected
int buttonPin = 0; // Pin number where the button is connected
int minFlashDelay = 100; // Minimum delay between flashes (in milliseconds)
int maxFlashDelay = 5000; // Maximum delay between flashes (in milliseconds)
int lightBrightness = 20; // The brightness of the LED (0-255)

char const *nsecHex = "d3871668e532102ca5293649d8e9f0e9085dd58e161cf93bee6037abb5b8a3fb"; 
char const *npubHex = "8ceb5dcd9a7be4cb3f399c073d9dad54acecdebac3176cdb041b36f5be5678e0"; 

String config_pubkey = "8ceb5dcd9a7be4cb3f399c073d9dad54acecdebac3176cdb041b36f5be5678e0"; 
String config1_pubkey = "20f77fb3b0ea02216cd05f38f2784e663db02c3ea2e285eda0ffba402c621ceb";
String config2_pubkey = "39c98bdd84c9f84d0cb085babb3e24d042ca531d6173b6c13cc902a5bc7652e3";

bool config1_rec = false;
bool config2_rec = false;

unsigned long config1_time;
unsigned long config2_time;
unsigned long start_timer = 0;

//String config_relay = "null"; 

char const *testRecipientPubKeyHex = "20f77fb3b0ea02216cd05f38f2784e663db02c3ea2e285eda0ffba402c621ceb"; //npub1yrmhlvasagpzzmxstuu0y7zwvc7mqtp75t3gtmdql7ayqtrzrn4setw7nt"; // e.g. // sender public key 683211bd155c7b764e4b99ba263a151d81209be7a566a2bb1971dc1bbd3b715e

SemaphoreHandle_t zapMutex;

// create a vector for storing zap amount for the flash queue
std::vector<int> zapAmountsFlashQueue;

struct KeyValue {
    String key;
    String value;
};

bool hasmessage = false;
unsigned long clockticker = 0;

#define BUTTON_PIN 0 // change this to the pin your button is connected to
#define DOUBLE_TAP_DELAY 250 // delay for double tap in milliseconds
volatile unsigned long lastButtonPress = 0;
volatile bool doubleTapDetected = false;

void IRAM_ATTR handleButtonInterrupt() {
  unsigned long now = millis();
  if (now - lastButtonPress < DOUBLE_TAP_DELAY) {
    doubleTapDetected = true;
  }
  lastButtonPress = now;
}

unsigned long getUnixTimestamp() {
  time_t now;
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return 0;
  } else {
    Serial.println("Got timestamp of " + String(now));    
  }
  time(&now);
  return now;
}


//free rtos task for control
void MachineControlTask(void *pvParameters) {
  Serial.println("Starting control task");
  //attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonInterrupt, FALLING);

  for(;;) {
     if(hasmessage) {
      if ((config1_time>0) && (config2_time>0)) {
        // received both messages
        
        hasmessage = false;
      } else if (config1_time>0) {
        // received 1 
        
       } else if (config2_time>0) {
        // received 2         
             
      }       
    } 
      vTaskDelay(500 / portTICK_PERIOD_MS);    
  }
}

/**
 * @brief Event callback for when a relay connects
 * 
 * @param key 
 * @param message 
 */
void relayConnectedEvent(const std::string& key, const std::string& message) {
  socketDisconnectedCount = 0;
  Serial.println("Relay connected: ");
  Serial.print(F("Requesting events:"));
}

/**
 * @brief Event callback for when a relay disconnects
 * 
 * @param key 
 * @param message 
 */
void relayDisonnectedEvent(const std::string& key, const std::string& message) {
  Serial.println("Relay disconnected: ");
  socketDisconnectedCount++;
  // reboot after 6 socketDisconnectedCount subsequenet messages
  if(socketDisconnectedCount >= 6) {
    Serial.println("Too many socket disconnections. Restarting");
    // restart device
    ESP.restart();
  }
}

String lastPayload = "";

void okEvent(const std::string& key, const char* payload) {        
    if(lastPayload != payload) { // Prevent duplicate events from multiple relays triggering the same logic
      lastPayload = payload;
      Serial.println("payload is: ");
      Serial.println(payload);
    }
}


/**
 * @brief Get the Bolt11 Invoice From Event object
 * 
 * @param jsonStr 
 * @return String 
 */
String getBolt11InvoiceFromEvent(String jsonStr) {
  // Remove all JSON formatting characters
  String str = jsonStr.substring(1, jsonStr.length()-1); // remove the first and last square brackets
  str.replace("\\", ""); // remove all backslashes

  // Search for the "bolt11" substring
  int index = str.indexOf("bolt11");

  // Extract the value associated with "bolt11"
  String bolt11 = "";
  if (index != -1) {
    int start = index + 9; // the value of "bolt11" starts 9 characters after the substring index
    int end = start; // initialize the end index
    while (str.charAt(end) != '\"') {
      end++; // increment the end index until the closing double-quote is found
    }
    bolt11 = str.substring(start, end); // extract the value of "bolt11"
  }
  return bolt11;
}

String getpubKeyFromEvent(String jsonStr) {
  // Remove all JSON formatting characters
  String str = jsonStr.substring(1, jsonStr.length()-1); // remove the first and last square brackets
  str.replace("\\", ""); // remove all backslashes

  // Search for the "bolt11" substring
  int index = str.indexOf("pubkey");

  // Extract the value associated with "bolt11"
  String bolt11 = "";
  if (index != -1) {
    int start = index+9; // the value of "bolt11" starts 9 characters after the substring index
    int end = start; // initialize the end index
    while (str.charAt(end) != '\"') {
      end++; // increment the end index until the closing double-quote is found
    }
    bolt11 = str.substring(start, end); // extract the value of "bolt11"
  }
  return bolt11;
}

/**
 * @brief Get the Amount In Satoshis from a lightning bol11 invoice
 *
 * @param input
 * @return int64_t
 */
int64_t getAmountInSatoshis(const String &input) {
    int64_t number = -1;
    char multiplier = ' ';

    for (unsigned int i = 0; i < input.length(); ++i) {
        if (isdigit(input[i])) {
            number = 0;
            while (isdigit(input[i])) {
                number = number * 10 + (input[i] - '0');
                ++i;
            }
            for (unsigned int j = i; j < input.length(); ++j) {
                if (isalpha(input[j])) {
                    multiplier = input[j];
                    break;
                }
            }
            break;
        }
    }

    if (number == -1 || multiplier == ' ') {
        return -1;
    }

    int64_t satoshis = number;

    switch (multiplier) {
        case 'm':
            satoshis *= 100000; // 0.001 * 100,000,000
            break;
        case 'u':
            satoshis *= 100; // 0.000001 * 100,000,000
            break;
        case 'n':
            satoshis /= 10; // 0.000000001 * 100,000,000
            break;
        case 'p':
            satoshis /= 10000; // 0.000000000001 * 100,000,000
            break;
        default:
            return -1;
    }

    return satoshis;
}

/**
 * @brief Get the Random Num object
 * 
 * @param min 
 * @param max 
 * @return uint16_t 
 */
uint16_t getRandomNum(uint16_t min, uint16_t max) {
  uint16_t rand  = (esp_random() % (max - min + 1)) + min;
  return rand;
}

/**
 * @brief Event callback for when a relay sends a zap receipt event
 * 
 * @param key 
 * @param payload 
 */
void zapReceiptEvent(const std::string& key, const char* payload) {
    if(lastPayload != payload) { // Prevent duplicate events from multiple relays triggering the same logic, as we are using multiple relays, this is likely to happen
      // define an array of phrases to use when a zap is a received
      Serial.print("-----> Key: ");
      Serial.println(String(key.c_str()));
      Serial.print("-----> Payload: ");
      Serial.println(payload);

      /* StaticJsonDocument<1024> doc;
      deserializeJson(doc, payload);

      serializeJsonPretty(doc, Serial); */

      String zapPhrases[] = {
        "Zap!",
        "A zap happened! ",
        "The zappenning!",
        "Zap! Zap!",
        "Pew pew!",
        "Zap! Zap! Zap!"
      };

      lastPayload = payload;
      String bolt11 = getBolt11InvoiceFromEvent(payload);
      Serial.println("BOLT11: " + bolt11);
      uint64_t amountInSatoshis = getAmountInSatoshis(bolt11);
      // Choose a random phrase from the array
      int randomPhraseIndex = getRandomNum(0, sizeof(zapPhrases) / sizeof(zapPhrases[0]) - 1);
      Serial.println(zapPhrases[randomPhraseIndex] + " " + String(amountInSatoshis) + " sats");
      // writeToDisplay(zapPhrases[randomPhraseIndex] + " " + String(amountInSatoshis) + " sats");
      //flashLightning(amountInSatoshis);
    }
}

void nip01Event(const std::string& key, const char* payload) {
    Serial.println("NIP01 event");
    Serial.println("payload is: ");
    Serial.println(payload);
    
    delay(1000);
    // writeToDisplay(payload);
}

const char* previousPayload = "";

void nip04Event(const std::string& key, const char* payload) {
  //if ((previousPayload!=payload)&&(millis()>60000)){
    if (millis()>60000){
    Serial.print("NIP04 event: ");
    String temp = "";
    temp = getpubKeyFromEvent(payload);
    Serial.println(temp);
    if (temp==config2_pubkey) {
      // writeToDisplay("NIP04 from: Mihai");
      Serial.println(" Mihai");
      config2_time = getUnixTimestamp();
      hasmessage = true;
    } else
    if (temp==config1_pubkey) {
      // writeToDisplay("NIP04 from: George");
      Serial.println(" George");
      config1_time = getUnixTimestamp();
      hasmessage = true;
    } 
    previousPayload=payload;    
  } else {
    Serial.println(millis());
    Serial.print("Previus Payload:");
    Serial.println(previousPayload);
    Serial.print("Payload:");
    Serial.println(payload);
  }
   delay(2000); //writeToDisplay(dmMessage);
}


/**
 * @brief Connect to the Nostr relays
 *
 */
void connectToNostrRelays() {
  // first disconnect from all relays
  nostrRelayManager.disconnect();
  Serial.println("Requesting Zap notifications");

  // split relays by comma into vector
  std::vector<String> relays = {
    "nostr.bitcoiner.social",
    "relay.plebstr.com",
    "nos.lol",
    "relay.damus.io"
  //  "relay.current.fyi",
  //  "relay.nostr.bg",
  //  "offchain.pub"
  };
    
  // no need to convert to char* anymore
  nostr.setLogging(true);
  nostrRelayManager.setRelays(relays);
  nostrRelayManager.setMinRelaysAndTimeout(1,10000);

  // Set some event specific callbacks here
  Serial.println("Setting callbacks");
  nostrRelayManager.setEventCallback("ok", okEvent);
  //nostrRelayManager.setEventCallback("connected", relayConnectedEvent);
  nostrRelayManager.setEventCallback("disconnected", relayDisonnectedEvent);
  nostrRelayManager.setEventCallback(9735, zapReceiptEvent);
  nostrRelayManager.setEventCallback(9734, zapReceiptEvent);
  nostrRelayManager.setEventCallback("nip01", nip01Event);
  nostrRelayManager.setEventCallback("nip04", nip04Event);
  nostrRelayManager.setEventCallback(4, nip04Event);

  Serial.println("connecting");
  nostrRelayManager.connect();
}


/**
 * @brief Initialise the nostr machine task
 * 
 */
void initMachine() {
  // Set the LED pin as OUTPUT
  pinMode(ledPin, OUTPUT);

  // start lamp control task
  xTaskCreatePinnedToCore(
    MachineControlTask,   /* Task function. */
    "MachineControlTask",     /* String with name of task. */
    5000,            /* Stack size in bytes. */
    NULL,             /* Parameter passed as input of the task */
    2,                /* Priority of the task. */
    NULL,             /* Task handle. */
    1);               /* Core where the task should run */
}

void setup_machine() {
  pinMode(buttonPin, INPUT_PULLUP); 
  // WiFi.begin(ssid, password);
  int wifiConnectTimer = 0;
    while (WiFi.status() != WL_CONNECTED && wifiConnectTimer < 15000) {
      delay(100);
      Serial.print(".");
      wifiConnectTimer = wifiConnectTimer + 100;
      hasInternetConnection = false;
    }
    if(WiFi.status() == WL_CONNECTED) {
      hasInternetConnection = true;
    }
    else {
      hasInternetConnection = false;      
    }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  long timestamp = getUnixTimestamp();
  
  initMachine();

  zapMutex = xSemaphoreCreateMutex();

  //randomSeed(analogRead(0)); // Seed the random number generator 
  
  connectToNostrRelays();
  
  // createZapEventRequest();

 

  String subscriptionString = "[\"REQ\", \"" + nostrRelayManager.getNewSubscriptionId() + "\", {\"authors\": [\"20f77fb3b0ea02216cd05f38f2784e663db02c3ea2e285eda0ffba402c621ceb\",\"8ceb5dcd9a7be4cb3f399c073d9dad54acecdebac3176cdb041b36f5be5678e0\",\"39c98bdd84c9f84d0cb085babb3e24d042ca531d6173b6c13cc902a5bc7652e3\"], \"kinds\": [1], \"limit\": 1}]";
  nostrRelayManager.enqueueMessage(subscriptionString.c_str());

  subscriptionString = "[\"REQ\", \"" + nostrRelayManager.getNewSubscriptionId() + "\", {\"#p\": [\"20f77fb3b0ea02216cd05f38f2784e663db02c3ea2e285eda0ffba402c621ceb\"], \"kinds\": [4], \"limit\": 1}]";
  nostrRelayManager.enqueueMessage(subscriptionString.c_str());

  subscriptionString = "[\"REQ\", \"" + nostrRelayManager.getNewSubscriptionId() + "\", {\"#p\": [\"8ceb5dcd9a7be4cb3f399c073d9dad54acecdebac3176cdb041b36f5be5678e0\"], \"kinds\": [4], \"limit\": 1}]";
  nostrRelayManager.enqueueMessage(subscriptionString.c_str());

  subscriptionString = "[\"REQ\", \"" + nostrRelayManager.getNewSubscriptionId() + "\", {\"#p\": [\"39c98bdd84c9f84d0cb085babb3e24d042ca531d6173b6c13cc902a5bc7652e3\"], \"kinds\": [4], \"limit\": 1}]";
  nostrRelayManager.enqueueMessage(subscriptionString.c_str());

 /*  subscriptionString = nostr.getEncryptedDm(nsecHex, npubHex, testRecipientPubKeyHex, timestamp, "Running NIP04!");
  nostrRelayManager.enqueueMessage(subscriptionString.c_str()); */

}