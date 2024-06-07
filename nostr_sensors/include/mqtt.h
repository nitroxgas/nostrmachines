#include <PubSubClient.h>
#include <WiFi.h>
#include "debug.h"

extern const char* mqtt_server;
extern const int mqtt_port;
extern const char* mqtt_user;
extern const char* mqtt_password;

void mqtt_init();
bool mqtt_publish(const char* topic, const char* payload);
void mqtt_loop();
