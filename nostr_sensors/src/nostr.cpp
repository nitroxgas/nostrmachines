
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

static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;

#ifdef BOARD_HAS_PSRAM 
  struct SpiRamAllocator {
    void* allocate(size_t size) {
      return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
    }

    void deallocate(void* pointer) {
      heap_caps_free(pointer);
    }

    void* reallocate(void* ptr, size_t new_size) {
      return heap_caps_realloc(ptr, new_size, MALLOC_CAP_SPIRAM);
    }
  };
  using SpiRamJsonDocument = BasicJsonDocument<SpiRamAllocator>;
#endif

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

String master_pubkey = "20f77fb3b0ea02216cd05f38f2784e663db02c3ea2e285eda0ffba402c621ceb";
// String config1_pubkey = "20f77fb3b0ea02216cd05f38f2784e663db02c3ea2e285eda0ffba402c621ceb";
// String config2_pubkey = "39c98bdd84c9f84d0cb085babb3e24d042ca531d6173b6c13cc902a5bc7652e3";

bool config1_rec = false;
bool config2_rec = false;

unsigned long config1_time;
unsigned long config2_time;
unsigned long start_timer = 0;

//String config_relay = "null"; 

// char const *testRecipientPubKeyHex = "20f77fb3b0ea02216cd05f38f2784e663db02c3ea2e285eda0ffba402c621ceb"; //npub1yrmhlvasagpzzmxstuu0y7zwvc7mqtp75t3gtmdql7ayqtrzrn4setw7nt"; // e.g. // sender public key 683211bd155c7b764e4b99ba263a151d81209be7a566a2bb1971dc1bbd3b715e

#ifdef YD_ESP32_S3
 String relayString = "relay.plebstr.com,nos.lol"; //relay.plebstr.com,nostr.bitcoiner.social,nos.lol,offchain.pub,relay.nostr.band,172.29.22.40";
#else
 String relayString = "relay.plebstr.com"; //relay.plebstr.com,nostr.bitcoiner.social,nos.lol,offchain.pub,relay.nostr.band,172.29.22.40";
#endif  

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
void connectToNostrRelays();

unsigned long getUnixTimestamp() {
  time_t now;
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    debugln("Failed to obtain time");
    return 0;
  } else {
   // debugf("Got timestamp of ", String(now));    
  }
  time(&now);
  return now;
}


//free rtos task for control
void MachineControlTask(void *pvParameters) {
  debugln("Starting control task");
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
  
  debugln("Relay connected: ");
  debugln_(String(message.c_str()));
  debugln("Requesting events:");
  //  Nip4 from master_key
  // subscriptionString = "[\"REQ\", \"" + nostrRelayManager.getNewSubscriptionId() + "\", {\"#p\": [\""+master_pubkey+"\"], \"kinds\": [4], \"limit\": 10, \"since\": "+String(start_time)+"}]";  

  // Nip4 from any since 10 days ago
  // String subscriptionString = "[\"REQ\", \"" + nostrRelayManager.getNewSubscriptionId() + "\", {\"kinds\": [4], \"limit\": 100, \"since\": "+String(start_time)+"}]";  
  // String subscriptionString = "[\"REQ\", \"" + nostrRelayManager.getNewSubscriptionId() + "\", {\"kinds\": [4], \"limit\": 100}]";  
  
  // All Nip4 from master_key
  // String subscriptionString = "[\"REQ\", \"" + nostrRelayManager.getNewSubscriptionId() + "\", {\"#p\": [\""+master_pubkey+"\"], \"kinds\": [4,1], \"limit\": 10}]";
  
  // nostrRelayManager.enqueueMessage(subscriptionString.c_str());

  // Not working..
  // subscriptionString = "[\"REQ\", \"" + nostrRelayManager.getNewSubscriptionId() + "\", {\"authors\": [\"20f77fb3b0ea02216cd05f38f2784e663db02c3ea2e285eda0ffba402c621ceb\",\"8ceb5dcd9a7be4cb3f399c073d9dad54acecdebac3176cdb041b36f5be5678e0\",\"kinds\": [1], \"limit\": 10}]";
  // nostrRelayManager.enqueueMessage(subscriptionString.c_str());
  // String subscriptionString = "[\"REQ\", \"" + nostrRelayManager.getNewSubscriptionId() + "\", {\"authors\": [\"20f77fb3b0ea02216cd05f38f2784e663db02c3ea2e285eda0ffba402c621ceb\",\"8ceb5dcd9a7be4cb3f399c073d9dad54acecdebac3176cdb041b36f5be5678e0\",\"kinds\": [1], \"limit\": 1}]";
  // nostrRelayManager.enqueueMessage(subscriptionString.c_str());
 
 // subscriptionString = "[\"REQ\", \"" + nostrRelayManager.getNewSubscriptionId() + "\", {\"#p\": [\"20f77fb3b0ea02216cd05f38f2784e663db02c3ea2e285eda0ffba402c621ceb\"], \"kinds\": [1,4], \"limit\": 1}]";
 // nostrRelayManager.enqueueMessage(subscriptionString.c_str());
 
  NostrRequestOptions* eventRequestOptions = new NostrRequestOptions();
 // Populate #p
  String p[] = {"20f77fb3b0ea02216cd05f38f2784e663db02c3ea2e285eda0ffba402c621ceb"};
  eventRequestOptions->p = p;
  eventRequestOptions->p_count = sizeof(p) / sizeof(p[0]);
  // Populate kinds
  int kinds[] = {4};
  eventRequestOptions->kinds = kinds;
  eventRequestOptions->kinds_count = sizeof(kinds) / sizeof(kinds[0]);
  // Populate other fields
  eventRequestOptions->since = start_time;
  // eventRequestOptions->until = 1640995200;
  eventRequestOptions->limit = 5;
  
  nostrRelayManager.requestEvents(eventRequestOptions);
  delete eventRequestOptions;

 debug("SETUP END:");
 debugln_(ESP.getFreeHeap());
}

/**
 * @brief Event callback for when a relay disconnects
 * 
 * @param key 
 * @param message 
 */
void relayDisonnectedEvent(const std::string& key, const std::string& message) {
  debug("Relay disconnected: Heap: ");  
  debugln_(ESP.getFreeHeap());
  /* debugln_("Message:"+String(message.c_str()));
  debugln_("Key:"+String(key.c_str())); */
  socketDisconnectedCount++;
  // reboot after 6 socketDisconnectedCount subsequenet messages
  /* if(socketDisconnectedCount >= 10) {
    debugln("Too many socket disconnections. Change Relay");
    if (relayString == "relay.plebstr.com") {
      relayString = "nos.lol";
    } else {
      relayString = "relay.plebstr.com";
    }    
    // connectToNostrRelays();
    // restart device
    // ESP.restart();
  } */
}

String lastPayload = "";

void okEvent(const std::string& key, const char* payload) {        
    if(lastPayload != payload) { // Prevent duplicate events from multiple relays triggering the same logic
      lastPayload = payload;
      debugln("payload is: ");
      debugln_(payload);
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
      debug("-----> Key: ");
      debugln_(String(key.c_str()));
      debug("-----> Payload: ");
      debugln_(payload);

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
      debugf("BOLT11: " , bolt11);
      uint64_t amountInSatoshis = getAmountInSatoshis(bolt11);
      // Choose a random phrase from the array
      int randomPhraseIndex = getRandomNum(0, sizeof(zapPhrases) / sizeof(zapPhrases[0]) - 1);
      debugln_(zapPhrases[randomPhraseIndex] + " " + String(amountInSatoshis) + " sats");
      // writeToDisplay(zapPhrases[randomPhraseIndex] + " " + String(amountInSatoshis) + " sats");
      //flashLightning(amountInSatoshis);
    }
}

void nip01Event(const std::string& key, const char* payload) {
    debugln("NIP01 event");
    debugln("payload is: ");
    debugln_(payload);  
    String temp = "";
    temp = getpubKeyFromEvent(payload);
    debugln_(temp);   
    // delay(1000);
    // writeToDisplay(payload);
}

const char* previousPayload = "";

String _decryptData(byte key[32], byte iv[16], String messageHex) {  
  int byteSize =  messageHex.length() / 2;
  byte messageBin[byteSize];
  fromHex(messageHex, messageBin, byteSize);

  AES_ctx ctx;
  AES_init_ctx_iv(&ctx, key, iv);
  AES_CBC_decrypt_buffer(&ctx, messageBin, sizeof(messageBin));

  return String((char *)messageBin).substring(0, byteSize);
}

void nip04Event(const std::string& key, const char* payload) {
  
  // if ((previousPayload!=payload)&&(millis()>INTERVAL_1_MINUTE)){
  //  if (millis()>60000){
    previousPayload = payload;
    debug("NIP04 event: HEAP:"); 
    debugln_(ESP.getMinFreeHeap());   
    /* String temp = "" ; // nostr.decryptDm(nsecHex,payload);
    // debugln_(temp);
    temp = getpubKeyFromEvent(payload);
    debugln_(temp);    
    if (temp==master_pubkey) {
      // writeToDisplay("NIP04 from: George");
      debugln(" George");
      config1_time = getUnixTimestamp();
      hasmessage = true;
    } 
    previousPayload=payload;  
  } else {
    debug("NIP04 event: REPLAY");
    debugln_(millis());
    debug("Previus Payload:");
    debugln_(previousPayload);
    debug("Payload:");
    debugln_(payload);
  } 
  */
    
 //  String dmMessage = nostr.decryptDm(nsecHex, payload);

// ------------------------------    
    #ifdef BOARD_HAS_PSRAM
      SpiRamJsonDocument doc(1048576);
    #else
      StaticJsonDocument<1052> doc;
    #endif  
    
    deserializeJson(doc, payload);
    String serialisedTest;
    serializeJson(doc, serialisedTest);
    /* debugf("serialisedTest %S\n", serialisedTest);
    debug("Nostr Antes decrypt:");
    debugln_(ESP.getFreeHeap());
    debugln_(sizeof(doc)); */
    
    portENTER_CRITICAL_ISR(&spinlock);
    
    String content = doc[2]["content"];

    String encryptedMessage = content.substring(0, content.indexOf("?iv="));
    // debugln_(encryptedMessage);

    String encryptedMessageHex = base64ToHex(encryptedMessage);
    int encryptedMessageSize =  encryptedMessageHex.length() / 2;
    byte encryptedMessageBin[encryptedMessageSize];
    fromHex(encryptedMessageHex, encryptedMessageBin, encryptedMessageSize);
    
    //debugf("encryptedMessageHex %s\n", encryptedMessageHex);

    String iv = content.substring(content.indexOf("?iv=") + 4);
    // debugln_(iv);
    
    String ivHex = base64ToHex(iv);
    int ivSize =  16;
    byte ivBin[ivSize];
    fromHex(ivHex, ivBin, ivSize);
    //debugf("iv %s\n", iv);
    //debugf("ivHex %s\n", ivHex);

    int byteSize =  32;
    byte privateKeyBytes[byteSize];
    fromHex(nsecHex, privateKeyBytes, byteSize);
    PrivateKey privateKey(privateKeyBytes);

    // String senderPubKeyHex = doc[2]["pubkey"];
    String tags = doc[2]["tags"];
    String senderPubKeyHex = tags.substring(tags.indexOf("[[") + 7, 71);
    
    //debug("PubKey:");
    //debugln_(senderPubKeyHex);
    byte senderPublicKeyBin[64];
    fromHex("02" + String(senderPubKeyHex), senderPublicKeyBin, 64);
    PublicKey senderPublicKey(senderPublicKeyBin);
    //debugf("senderPublicKey.toString() is %s\n", senderPublicKey.toString());

    byte sharedPointX[32];
    privateKey.ecdh(senderPublicKey, sharedPointX, false);
    String sharedPointXHex = toHex(sharedPointX, sizeof(sharedPointX));
    //debugf("sharedPointXHex is %s \n", sharedPointXHex);

    String message = _decryptData(sharedPointX, ivBin, encryptedMessageHex);
    debugln("---------------");
    debug("Mensagem:");   
    message.trim();
    debugln_(message);
    debugln("---------------");
    portEXIT_CRITICAL_ISR(&spinlock);
// -----------------    
  /* }  else {
    debugln("Old event nip04");
  } */
}

// Split string to vector
std::vector<String> split(const String &str, char delimiter) {
  std::vector<String> tokens;
  int start = 0;
  int end = str.indexOf(delimiter);
  
  while (end != -1) {
    tokens.push_back(str.substring(start, end));
    start = end + 1;
    end = str.indexOf(delimiter, start);
  }
  
  tokens.push_back(str.substring(start));
  
  return tokens;
}

/**
 * @brief Connect to the Nostr relays
 *
 */
void connectToNostrRelays() {
  // first disconnect from all relays
  nostrRelayManager.disconnect();
  
  // split relays by comma into vector
  std::vector<String> relays = split(relayString, ',');
     
  // no need to convert to char* anymore
  nostr.setLogging(false);
  nostrRelayManager.setRelays(relays);
  nostrRelayManager.setMinRelaysAndTimeout(1,INTERVAL_1_MINUTE);

  // Set some event specific callbacks here
  debugln("Setting callbacks");
  nostrRelayManager.setEventCallback("ok", okEvent);
  nostrRelayManager.setEventCallback("connected", relayConnectedEvent);
  nostrRelayManager.setEventCallback("disconnected", relayDisonnectedEvent);
  //nostrRelayManager.setEventCallback(9735, zapReceiptEvent);
  //nostrRelayManager.setEventCallback(9734, zapReceiptEvent);
  // nostrRelayManager.setEventCallback(1, nip01Event);
  nostrRelayManager.setEventCallback(4, nip04Event);  
  // nostrRelayManager.setEventCallback(14, nip04Event);

  debugln("connecting");
  nostrRelayManager.connect();
}

long start_time = 0;
/**
 * @brief Initialise the nostr machine task
 * 
 */
/*
void initMachine() {
  // Set the LED pin as OUTPUT
  // pinMode(ledPin, OUTPUT);   
 // start lamp control task
  xTaskCreatePinnedToCore(
    MachineControlTask,   // Task function. 
    "MachineControlTask",     // String with name of task. 
    5000,             // Stack size in bytes. 
    NULL,             // Parameter passed as input of the task 
    2,                // Priority of the task.
    NULL,             // Task handle. 
    1);               // Core where the task should run 
} 
*/

void setup_machine() {
  // pinMode(buttonPin, INPUT_PULLUP); 
  // WiFi.begin(ssid, password);

  // Change defaults to wManager configured parameters;


  int wifiConnectTimer = 0;
    while (WiFi.status() != WL_CONNECTED && wifiConnectTimer < 15000) {
      delay(100);
      debug(".");
      wifiConnectTimer = wifiConnectTimer + 100;
      hasInternetConnection = false;
    }
    if(WiFi.status() == WL_CONNECTED) {
      hasInternetConnection = true;      
    }
    else {
      hasInternetConnection = false;      
    }
  
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  start_time = getUnixTimestamp() - (86400);
  
  // initMachine();

  // zapMutex = xSemaphoreCreateMutex();

  // randomSeed(analogRead(25)); // Seed the random number generator 
  
  connectToNostrRelays();
  
  // createZapEventRequest();

  /* subscriptionString = "[\"REQ\", \"" + nostrRelayManager.getNewSubscriptionId() + "\", {\"#p\": [\"8ceb5dcd9a7be4cb3f399c073d9dad54acecdebac3176cdb041b36f5be5678e0\"], \"kinds\": [4], \"limit\": 1}]";
  nostrRelayManager.enqueueMessage(subscriptionString.c_str());
 */
  /* subscriptionString = "[\"REQ\", \"" + nostrRelayManager.getNewSubscriptionId() + "\", {\"#p\": [\"39c98bdd84c9f84d0cb085babb3e24d042ca531d6173b6c13cc902a5bc7652e3\"], \"kinds\": [4], \"limit\": 1}]";
  nostrRelayManager.enqueueMessage(subscriptionString.c_str()); */

 /*  subscriptionString = nostr.getEncryptedDm(nsecHex, npubHex, testRecipientPubKeyHex, timestamp, "Running NIP04!");
  nostrRelayManager.enqueueMessage(subscriptionString.c_str()); */

}

void sendPublicMessage(String message_to_send){
  long timestamp = getUnixTimestamp();
  String noteString = nostr.getNote(nsecHex, npubHex, timestamp, message_to_send);
  nostrRelayManager.enqueueMessage(noteString.c_str());
}