#include "mqtt.h"

WiFiClient espClient;
PubSubClient client(espClient);

const char* mqtt_server = "192.168.1.11";
const int mqtt_port = 1883;
const char* mqtt_user = "xxxx";
const char* mqtt_password = "xxxxx";

void mqtt_reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("NostrMachines", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

bool mqtt_publish(const char* topic, const char* payload){
    return client.publish(topic, payload);
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