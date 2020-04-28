
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <time.h>
#include "display.h"

HTTPClient http;
DynamicJsonDocument json(2048);

unsigned long last_fetch = 0;
int err_code = 0;

bool load_url(const char *url);
bool parse_weather();
bool parse_uvi();

void update_weather_status_block() {
  if (err_code == 0) {
    update_block(1, "HTTP", "OK", TFT_GREEN);
  } else {
    char s[10];
    snprintf(s, 10, "%d", err_code);
    update_block(1, "HTTP", s, TFT_RED);
  }
}

void deg_to_string(int deg, char* out_str) {
  if (deg >= 348 || deg < 11) { strcpy(out_str, "N"); }
  else if (deg < 33) {          strcpy(out_str, "NNE"); }
  else if (deg < 56) {          strcpy(out_str, "NE"); }
  else if (deg < 78) {          strcpy(out_str, "ENE"); }
  else if (deg < 101) {         strcpy(out_str, "E"); }
  else if (deg < 123) {         strcpy(out_str, "ESE"); }
  else if (deg < 146) {         strcpy(out_str, "SE"); }
  else if (deg < 168) {         strcpy(out_str, "SSE"); }
  else if (deg < 191) {         strcpy(out_str, "S"); }
  else if (deg < 213) {         strcpy(out_str, "SSW"); }
  else if (deg < 236) {         strcpy(out_str, "SW"); }
  else if (deg < 258) {         strcpy(out_str, "WSW"); }
  else if (deg < 281) {         strcpy(out_str, "W"); }
  else if (deg < 303) {         strcpy(out_str, "WNW"); }
  else if (deg < 326) {         strcpy(out_str, "NW"); }
  else if (deg < 348) {         strcpy(out_str, "NNW"); }
}

void fetch_weather_on_interval() {
  if (!WiFi.isConnected()) return;
  if (last_fetch > 0 && millis() - last_fetch < 600000) return;
  last_fetch = millis();

  if (
    load_url("http://api.openweathermap.org/data/2.5/weather?appid=e3f3f0e149c736248b95119d5cb66a61&q=Kiev,UA&units=metric") &&
    parse_weather() &&
    load_url("http://api.openweathermap.org/data/2.5/uvi?appid=e3f3f0e149c736248b95119d5cb66a61&lat=50.43&lon=30.52") &&
    parse_uvi()
  ) { }
}

bool load_url(const char *url) {  
  Serial.printf("loading %s ... ", url);

  WiFiClient client;
  HTTPClient http;

  if (!http.begin(client, url)) {
    Serial.printf("http unable to connect\n");
    err_code = -1000;
    return false;
  }

  int http_code = http.GET();
  if (http_code != HTTP_CODE_OK) {
    Serial.printf("http failed, error: %s\n", http.errorToString(http_code).c_str());
    err_code = http_code;
    return false;
  }
  
  DeserializationError error = deserializeJson(json, http.getStream());
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    err_code = -1001;
    return false;
  }

  http.end();
  Serial.print("DONE\n");

  err_code = 0;
  return true;
}

const size_t ws_len = 20;
char ws[ws_len];

bool parse_weather() {
  float temp = json["main"]["temp"];
  float feels = json["main"]["feels_like"];
  int press = json["main"]["pressure"];
  int hum = json["main"]["humidity"];
  int vis = json["visibility"];
  int wind = json["wind"]["speed"];
  int wind_deg = json["wind"]["deg"];
  int clouds = json["clouds"]["all"];

  snprintf(ws, ws_len, "%.2f", temp);
  update_block(4, "Temp", ws, TFT_YELLOW);

  snprintf(ws, ws_len, "%.2f", feels);
  update_block(5, "Feels", ws, TFT_YELLOW);

  snprintf(ws, ws_len, "%d", press);
  update_block(7, "Press", ws, TFT_BLUE);

  snprintf(ws, ws_len, "%d%%", hum);
  update_block(6, "Humidity", ws, TFT_CYAN);

  snprintf(ws, ws_len, "%.1fkm", vis / 1000.0);
  update_block(11, "Vis", ws, TFT_WHITE);

  snprintf(ws, ws_len, "%dm/s", wind);
  update_block(9, "Wind", ws, TFT_GREENYELLOW);

  deg_to_string(wind_deg, ws);
  update_block(10, "Dir", ws, TFT_GREENYELLOW);

  snprintf(ws, ws_len, "%d%%", clouds);
  update_block(8, "Clouds", ws, TFT_WHITE);

  // TIME

  int tz = json["timezone"];
  int dt = json["dt"];
  int sunrise = json["sys"]["sunrise"];
  int sunset = json["sys"]["sunset"];
  dt += tz;
  sunrise += tz;
  sunset += tz;


  struct tm dt_tm = *localtime(((time_t*)&dt));
  struct tm sunrise_tm = *localtime(((time_t*)&sunrise));
  struct tm sunset_tm = *localtime((time_t*)&sunset);

  strftime(ws, ws_len, "%H:%M", &sunrise_tm);
  update_block(12, "Sunrise", ws, TFT_PINK);

  strftime(ws, ws_len, "%H:%M", &sunset_tm);
  update_block(13, "Sunset", ws, TFT_PINK);

  int sunrise_min = sunrise_tm.tm_hour * 60 + sunrise_tm.tm_min;
  int sunset_min = sunset_tm.tm_hour * 60 + sunset_tm.tm_min;
  int dt_min = dt_tm.tm_hour * 60 + dt_tm.tm_min;

  int min;
  bool sun_up = false;
  if (dt_min < sunrise_min) {
    min = sunrise_min - dt_min;
  } else if (dt_min < sunset_min) {
    sun_up = true;
    min = sunset_min - dt_min;
  } else {
    min = sunrise_min + (60*24 - dt_min);
  }
  
  int hr = min / 60;
  min = min % 60;

  snprintf(ws, ws_len, "-%d:%02d", hr, min);
  update_block(14, sun_up ? "To Set" : "To Rise", ws, TFT_MAGENTA);

  return true;
}

bool parse_uvi() {
  float value = json["value"];

  char s[10];
  snprintf(s, 10, "%.2f", value);
  update_block(15, "UVI", s, TFT_RED);

  return true;
}