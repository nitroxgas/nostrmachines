#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <Arduino.h>

#define DEFAULT_SSID		"NostrSensor"
#define DEFAULT_WIFIPW		"Measures"
#define DEFAULT_PRIV		"d3871668e532102ca5293649d8e9f0e9085dd58e161cf93bee6037abb5b8a3fb"
#define DEFAULT_PUB			"8ceb5dcd9a7be4cb3f399c073d9dad54acecdebac3176cdb041b36f5be5678e0"
#define DEFAULT_RELAYS		"relay.nostrmachines.xyz,nostr.bitcoiner.social,relay.plebstr.com,nos.lol,relay.damus.io"
#define DEFAULT_MASTERPUB	"20f77fb3b0ea02216cd05f38f2784e663db02c3ea2e285eda0ffba402c621ceb"
#define DEFAULT_TIMEZONE	-3
#define DEFAULT_ZAPVALUE	10
#define DEFAULT_NAME		"NOSTRSENSOR"
#define DEFAULT_MAC			"00:00:00:00:00:00"
#ifdef SET_DEEP_SLEEP_SECONDS
#define DEFAULT_SLEEP_SEC	SET_DEEP_SLEEP_SECONDS
#endif

// JSON config file SPIFFS (different for backward compatibility with existing devices)
#define JSON_KEY_PRIV		"nostrPrivKey"
#define JSON_KEY_PUB		"nostrPubKey"
#define JSON_KEY_RELAYS		"nostrRelays"
#define JSON_KEY_MASTERPUB	"nostrMasterPub"
#define JSON_KEY_TIMEZONE	"gmtZone"
#define JSON_KEY_ZAPVALUE	"zapValue"
#define JSON_KEY_NAME		"NOSTRSENSOR"
#define JSON_KEY_SLEEP		"SLEEPSEC"

// JSON config files
#define JSON_CONFIG_FILE	"/config.json"

// settings
struct TSettings
{
	String WifiSSID{ DEFAULT_SSID };
	String WifiPW{ DEFAULT_WIFIPW };
	String privkey{ DEFAULT_PRIV };
	String pubkey{ DEFAULT_PUB };
	String nrelays{ DEFAULT_RELAYS };
	String masterpub{ DEFAULT_MASTERPUB };	
	int Timezone{ DEFAULT_TIMEZONE };	
	long zapvalue{ DEFAULT_ZAPVALUE };
	String name{ DEFAULT_NAME };
	String macaddr{ DEFAULT_MAC };
	#ifdef SET_DEEP_SLEEP_SECONDS
	int sleepsec{ DEFAULT_SLEEP_SEC };	
	#endif
};

extern TSettings Settings;

#endif // _GLOBAL_H_