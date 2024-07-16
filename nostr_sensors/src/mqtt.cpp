#include "mqtt.h"

WiFiClient espClient;
PubSubClient client(espClient);

const char* mqtt_server = "192.168.1.11";
const int mqtt_port = 1883;
const char* mqtt_user = "nitroxgas";
const char* mqtt_password = "Cz1mwyh.";
char mqtt_monitor_2 = 0;
const char* client_name;
String tag_name;
#ifdef MQTT_READ
bool has_mqtt_message = false;
String tag_read;
char read_mqtt_topic;
String read_mqtt_message;
#endif

#ifdef MQTT_READ
void callback(char* topic, byte* message, unsigned int length) {  
  debug("Message arrived on topic: ");
  debug_(topic);
  debug(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    debug_((char)message[i]);
    messageTemp += (char)message[i];
  }
  debugln("");

  tag_read = tag_name+"/cmd/sleep_sec";  
  if (strcmp(topic,tag_read.c_str()) == 0 ) {
    // debugln("Change sleep timer");
    has_mqtt_message = true;
    read_mqtt_topic = 1; // Sleep timer
    read_mqtt_message = messageTemp;
    return;
  }

  tag_read = tag_name+"/cmd/wait_ota";  
  if (strcmp(topic,tag_read.c_str()) == 0 ) {
    // debugln("Wait OTA");
    has_mqtt_message = true;
    read_mqtt_topic = 2; // Wait for OTA
    read_mqtt_message = messageTemp;
    return;
  }

}
#endif

void mqtt_reconnect() {
  while (!client.connected()) {
    debug("Attempting MQTT connection...");
    if (client.connect(client_name, mqtt_user, mqtt_password)) {
      debugln("connected");
      mqtt_monitor_2 = 0;
      #ifdef MQTT_READ
        String tag_read = tag_name+"/cmd/#";
        client.subscribe(tag_read.c_str());
      #endif
    } else {
      debugf("failed, rc=%d",client.state());     
      mqtt_monitor_2++;
      if (mqtt_monitor_2>10) ESP.restart(); 
      debugln(" try again in 5 seconds");
      vTaskDelay(5000/ portTICK_PERIOD_MS);
    }
  }
}

bool mqtt_publish(const char* topic, const char* payload){    
    return client.publish(topic, payload);
}

bool mqtt_publish(const char* topic, const char* payload, boolean retained){    
    return client.publish(topic, payload, retained);
}

bool mqtt_publish(const char* topic, const char* payload, unsigned int plength){    
    return client.publish(topic, payload, plength);
}

void mqtt_init(){
    client.setServer(mqtt_server, mqtt_port);
    #ifdef MQTT_READ
    client.setCallback(callback);
    #endif
}

void mqtt_loop(){
    if (!client.connected()) {
    mqtt_reconnect();
  }
  client.loop();  
}