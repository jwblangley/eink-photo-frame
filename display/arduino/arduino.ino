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
#define WAKE_INTERVAL_S 4 * 3600

#define FAILED_CONNECTION_TIMEOUT_MS 15'000

UBYTE paintFromNetwork(const UBYTE netByte)
{
  if (netByte == 0)
  {
    return EPD_10IN2G_BLACK;
  }
  if (netByte == 1)
  {
    return EPD_10IN2G_WHITE;
  }
  if (netByte == 2)
  {
    return EPD_10IN2G_YELLOW;
  }
  if (netByte == 3)
  {
    return EPD_10IN2G_RED;
  }
  Serial.println("Unknown paint from network. Defaulting to black");
  return EPD_10IN2G_BLACK;
}

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

  // Time to catch upload/monitor
  delay(5'000);

  // --- WiFi Setup ---
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("WiFi attempting to connect");
  const auto before = millis();
  while (WiFi.status() != WL_CONNECTED) {
    indicateStatus("connecting...", 250);
    if (millis() - before >= FAILED_CONNECTION_TIMEOUT_MS) {
      Serial.printf("Failed to connect in %dms. Aborting\n", FAILED_CONNECTION_TIMEOUT_MS);
      Serial.flush();
      return;
    }
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
    Serial.println("Failed to allocate memory");
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
      const UBYTE paint = paintFromNetwork(b);
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
}

void loop() {
  // Enter deep sleep until next wake up
  // Runs on early exit (failure) or successful write
  esp_sleep_enable_timer_wakeup(WAKE_INTERVAL_S * US_TO_S_FACTOR);
  esp_deep_sleep_start();
}
