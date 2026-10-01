#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include "src/waveshare/10in2g/ESP32/DEV_Config.h"
#include "src/waveshare/10in2g/ESP32/EPD_10in2g.h"
#include "src/waveshare/10in2g/ESP32/GUI_Paint.h"
#include "src/waveshare/10in2g/ESP32/ImageData.h"

#include "src/network_credentials.h"

#define ROTATE 1

#define BAUD 115200
#define LED 21

#define US_TO_S_FACTOR 1'000'000ULL
#define WAKE_INTERVAL_S 2 * 3600

void indicateStatus(const String& message, const size_t delayTime)
{
  Serial.println(message);
  for (size_t i = 0; i < 2; i++)
  {
    delay(delayTime);
    digitalWrite(LED, !digitalRead(LED));
  }
}

void setup() {
  Serial.begin(BAUD);
  pinMode(LED, OUTPUT);

  SPI.begin(EPD_SCK_PIN, EPD_MISO_PIN, EPD_MOSI_PIN, -1);

  // --- WiFi Setup ---
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("WiFi attempting to connect");
  while (WiFi.status() != WL_CONNECTED) {
    indicateStatus("connecting...", 250);
  }
  Serial.printf("\nWiFi connected. IP address: %s\n",
  WiFi.localIP().toString().c_str());

  // Initialize the display board
  Serial.println("Initialising display");
  DEV_Module_Init();
  EPD_10IN2G_Init();

  // Create a new image cache
  Serial.println("Allocating memory");
  UBYTE *canvas;
  UDOUBLE imageSize =
      ((EPD_10IN2G_WIDTH % 4 == 0) ? (EPD_10IN2G_WIDTH / 4)
                                   : (EPD_10IN2G_WIDTH / 4 + 1)) *
      EPD_10IN2G_HEIGHT;

  if ((canvas = (UBYTE *)ps_malloc(imageSize)) == nullptr) {
    Serial.println("Failed to allocate memory\n");
    return;
  }
  Serial.println("Zeroing canvas");
  memset(canvas, 0, imageSize);

  // HTTP Request
  HTTPClient http;
  const String url = String(ENDPOINT) + "/get?width=" + EPD_10IN2G_WIDTH + "&height=" + EPD_10IN2G_HEIGHT + "&rotate=" + ROTATE;
  http.begin(url);
  const int httpResponseCode = http.GET();

  if (httpResponseCode <= 0) {
    Serial.printf("Connection rejected. rc=%d\n", httpResponseCode);
    return;
  }

  Serial.printf("HTTP Status Code: %d\n", httpResponseCode);

  const int httpSize = http.getSize();
  WiFiClient* stream = http.getStreamPtr();

  if (httpSize <= 0)
  {
    Serial.printf("Invalid http size: %d\n", httpSize);
    return;
  }

  size_t bytesRead = 0;
  while (http.connected() && (bytesRead < httpSize)) {
    while (stream->available() > 0)
    {
      const UBYTE b = stream->read();
      const UBYTE paint = b > 0 ? EPD_10IN2G_WHITE : EPD_10IN2G_BLACK;
      canvas[bytesRead / 4] |= (paint << (2 * (3 - (bytesRead % 4))));
      bytesRead++;
    }
    delay(1);
  }

  if (bytesRead / 4 == imageSize && bytesRead % 4 == 0)
  {
    Serial.printf("Successfully read %d bytes\n", bytesRead);
  }
  else
  {
    Serial.printf("Read %d bytes. Expected %d\n", bytesRead, imageSize);
    return;
  }

  Serial.println("Closing HTTP client");
  http.end();


  Serial.println("Displaying");
  EPD_10IN2G_Display(canvas);
  DEV_Delay_ms(2000);

  Serial.println("Shutting down");
  EPD_10IN2G_Sleep();

  // Must be >=2s
  DEV_Delay_ms(2000);

  // close 5V, Module enters 0 power consumption
  DEV_Module_Exit();
  free(canvas);

  // Enter deep sleep until next wake up
  esp_sleep_enable_timer_wakeup(WAKE_INTERVAL_S * US_TO_S_FACTOR);
  esp_deep_sleep_start();
}

void loop() {
  delay(2000);
  Serial.println("Heartbeat");
}
