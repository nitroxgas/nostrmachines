#ifndef _NOSTR_H_
#define _NOSTR_H_
#include <Arduino.h>
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
#include "freertos/task.h"

extern NostrEvent nostr;
extern NostrRelayManager nostrRelayManager;
extern NostrQueueProcessor nostrQueue;

#endif // _NOSTR_H_