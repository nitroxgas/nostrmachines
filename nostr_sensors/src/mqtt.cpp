#include "mqtt.h"

WiFiClient espClient;
PubSubClient client(espClient);

const char* mqtt_server = "192.168.1.11";
const int mqtt_port = 1883;
const char* mqtt_user = "nitroxgas";
const char* mqtt_password = "Cz1mwyh.";
char mqtt_monitor_2 = 0;
const char* client_name;

void mqtt_reconnect() {
  while (!client.connected()) {
    debug("Attempting MQTT connection...");
    if (client.connect(client_name, mqtt_user, mqtt_password)) {
      debugln("connected");
      mqtt_monitor_2 = 0;
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

}

void mqtt_loop(){
    if (!client.connected()) {
    mqtt_reconnect();
  }
  client.loop();  
}