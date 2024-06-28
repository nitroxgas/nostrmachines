#include <PubSubClient.h>
#include <WiFi.h>
#include "debug.h"

extern const char* mqtt_server;
extern const int mqtt_port;
extern const char* mqtt_user;
extern const char* mqtt_password;
extern const char* client_name;

void mqtt_init();
bool mqtt_publish(const char* topic, const char* payload);
bool mqtt_publish(const char* topic, const char* payload, boolean retained);
//bool mqtt_publish(const char* topic, const char* payload, unsigned int plength);
void mqtt_loop();
