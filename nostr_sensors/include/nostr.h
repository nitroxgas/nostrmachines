#ifndef _NOSTR_H_
#define _NOSTR_H_
#include <Arduino.h>
#include "WiFiClientSecure.h"
#include "time.h"
#include "debug.h"
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
#include "timers.h"
#include "global.h"

// freertos
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern NostrEvent nostr;
extern NostrRelayManager nostrRelayManager;
extern NostrQueueProcessor nostrQueue;
extern char const *nsecHex;
extern char const *npubHex;
extern String relayString;
extern String master_pubkey;
// extern unsigned long start_timer = 0;
extern unsigned long start_time;

void setup_machine();
void sendPublicMessage(String message_to_send);
unsigned long getUnixTimestamp();

#endif // _NOSTR_H_