#include <Arduino.h>
#include <WiFi.h>

// This file is excluded from the git working tree for secuity reasons
#include "src/network_credentials.h"

#define PORT 8000
#define UDP_MAX 1478
#define BAUD 115200

void setup() {
  Serial.begin(BAUD);

  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.printf("WiFi attempting to connect. status=%d\n", WiFi.status());
  }
  Serial.printf("WiFi connected. IP address: %s\n", WiFi.localIP().toString().c_str());
}

void loop() {
  delay(1000);
  Serial.printf("HeartBeat. status=%d\n", WiFi.status());
  Serial.printf("WiFi connected. IP address: %s\n", WiFi.localIP().toString().c_str());
}
