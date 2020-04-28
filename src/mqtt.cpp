
#include <PubSubClient.h>
#include <WiFi.h>

#include "creds.h"
#include "display.h"

WiFiClient espClient;
PubSubClient mqtt(espClient);

const size_t topic_len = 100;
char topic[100];
const size_t msg_len = 25;
char msg[25];

unsigned long last_mqtt_attempt = 0;

void format_topic(const char* device, const char* control, const char* meta, bool setter);
bool compare_topic(const char* device, const char* control, const char* topic);

void update_status_block() {
  char stat_s[10];
  uint16_t color = TFT_RED;
  int state = mqtt.state();
  if (state == 0) {
    strcpy(stat_s, "OK");
    color = TFT_GREEN;
  } else if (state == -1) {
    strcpy(stat_s, "DIS");
  } else {
    snprintf(stat_s, 10, "%d", state);
  }
  update_block(2, "MQTT", stat_s, color);
}

void connect_mqtt_if_needed() {
  if (mqtt.connected() || millis() - last_mqtt_attempt < 5000) {
    return;
  }
  if (!mqtt.connect(MQTT_ID, MQTT_LOGIN, MQTT_PASS)) {
      last_mqtt_attempt = millis();
      return;
  }

  format_topic("weather_w1", "External Sensor 1", "", false);
  mqtt.subscribe(topic);
  format_topic("sensor_cabinet", "Temperature", "", false);
  mqtt.subscribe(topic);
  format_topic("sensor_cabinet", "CO2", "", false);
  mqtt.subscribe(topic);
  format_topic("sensor_cabinet", "Illuminance", "", false);
  mqtt.subscribe(topic);
  format_topic("sensor_cabinet", "Max Motion", "", false);
  mqtt.subscribe(topic);
}

const size_t s_len = 10;
char s[s_len];

void mqtt_message_handler(char* rcv_topic, byte* payload, unsigned int length) {
  strncpy(msg, (char*)payload, length);
  msg[length] = '\0';
  Serial.printf("mqtt msg: %s -- %s\n", rcv_topic, msg);

  if (compare_topic("weather_w1", "External Sensor 1", rcv_topic)) {
    float t = atof(msg);
    snprintf(s, s_len, "%.2f", t);
    update_block(16, "W1", s, TFT_YELLOW);
  
  } else if (compare_topic("sensor_cabinet", "Temperature", rcv_topic)) {
    float t = atof(msg);
    snprintf(s, s_len, "%.2f", t);
    update_block(20, "Room", s, TFT_ORANGE);

  } else if (compare_topic("sensor_cabinet", "CO2", rcv_topic)) {
    int t = atoi(msg);
    snprintf(s, s_len, "%d", t);
    update_block(21, "CO2", s, TFT_WHITE);
  
  } else if (compare_topic("sensor_cabinet", "Illuminance", rcv_topic)) {
    float t = atof(msg);
    snprintf(s, s_len, "%.2f", t);
    update_block(22, "Lux", s, TFT_RED);

  } else if (compare_topic("sensor_cabinet", "Max Motion", rcv_topic)) {
    int v = atoi(msg);
    snprintf(s, s_len, "%d", v);
    update_block(23, "Moution", s, TFT_GREEN);

  }
}

// === SERVICE ===

void format_topic(const char* device, const char* control, const char* meta, bool setter) {
  if (*control) {
    if (*meta) {
      snprintf(topic, topic_len, "/devices/%s/controls/%s/meta/%s", device, control, meta);
    } else {
      if (setter) {
        snprintf(topic, topic_len, "/devices/%s/controls/%s/on", device, control);
      } else {
        snprintf(topic, topic_len, "/devices/%s/controls/%s", device, control);
      }
    }
  } else {
    if (*meta) {
      snprintf(topic, topic_len, "/devices/%s/meta/%s", device, meta);
    } else {
      snprintf(topic, topic_len, "/devices/%s", device);
    }
  }
}

bool compare_topic(const char* device, const char* control, const char* rcv_topic) {
  format_topic(device, control, "", false);
  return strcmp(topic, rcv_topic) == 0;
}

// === LOOP ===

void mqtt_setup() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(mqtt_message_handler);
}

void mqtt_loop() {
  update_status_block();
  mqtt.loop();
  connect_mqtt_if_needed();
}