#include <Arduino.h>
#include <WiFi.h>

#include "src/network_credentials.h"

#define PORT 8000
#define BAUD 115200

#define LED 21

void setup() {
  Serial.begin(BAUD);
  pinMode(LED, OUTPUT);

  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    digitalWrite(LED, !digitalRead(LED));
    Serial.printf("WiFi attempting to connect. status=%d\n", WiFi.status());
  }
  Serial.printf("WiFi connected. IP address: %s\n",
                WiFi.localIP().toString().c_str());
}

void loop() {
  delay(1000);
  digitalWrite(LED, !digitalRead(LED));

  Serial.printf("HeartBeat\n");
}
