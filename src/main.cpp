
/*
0:        HTTP  MQTT  RSSI
4:  Tw    Tfeel Hum   Press   
8:  Cld   Wnd   Dir   Vis
12: Sun   Set   Hrs-l Uv
16: W1          Tm-mx Ttom-mx
20: Trom  CO2`  Ilum  Mou     

ntp time
temp forecast
button light curtains controls

*/

#include <Arduino.h>
#include <WiFi.h>

#include "creds.h"
#include "display.h"
#include "weather.h"
#include "mqtt.h"

void update_status_blocks() {
  char stat_s[10];
  uint16_t color = TFT_RED;

  switch (WiFi.status()) {
    default: strcpy(stat_s, "?"); break;
    case WL_IDLE_STATUS: strcpy(stat_s, "IDLE"); break;
    case WL_NO_SSID_AVAIL: strcpy(stat_s, "N/A"); break;
    case WL_CONNECT_FAILED: strcpy(stat_s, "FAIL"); break;
    case WL_CONNECTION_LOST: strcpy(stat_s, "LOST"); break;
    case WL_DISCONNECTED: strcpy(stat_s, "DIS"); break;
    case WL_CONNECTED: 
      snprintf(stat_s, 10, "%3.d", WiFi.RSSI());
      color = TFT_GREEN;
      break;
  }
  update_block(3, "WiFi", stat_s, color, TFT_BLACK, true);
}

// ============= Main ==============

void setup() {
  Serial.begin(115200);
  M5.begin();
  display_setup();
  
  WiFi.begin(SSID, WIFI_PASS);
  mqtt_setup();
}

void loop() {
  update_status_blocks();
  update_weather_status_block();
  fetch_weather_on_interval();
  mqtt_loop();
  display_loop();

  delay(50);
}
