#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>

#include "src/waveshare/10in2g/ESP32/DEV_Config.h"
#include "src/waveshare/10in2g/ESP32/EPD_10in2g.h"
#include "src/waveshare/10in2g/ESP32/GUI_Paint.h"
#include "src/waveshare/10in2g/ESP32/ImageData.h"

#include "src/network_credentials.h"

#define BAUD 115200
#define LED 21

void setup() {
  Serial.begin(BAUD);
  pinMode(LED, OUTPUT);

  SPI.begin(EPD_SCK_PIN, EPD_MISO_PIN, EPD_MOSI_PIN, -1);

  // // --- WiFi Setup ---
  // WiFi.disconnect(true);
  // WiFi.mode(WIFI_STA);
  // WiFi.setSleep(false);
  //
  // WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  // Serial.print("WiFi attempting to connect");
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(250);
  //   digitalWrite(LED, !digitalRead(LED));
  //   Serial.print(".");
  // }
  // Serial.printf("\nWiFi connected. IP address: %s\n",
  // WiFi.localIP().toString().c_str());

  // Initialize the display board
  Serial.println("Initialising");
  DEV_Module_Init();
  EPD_10IN2G_Init();
  EPD_10IN2G_Clear(EPD_10IN2G_WHITE);
  DEV_Delay_ms(1000);

  // Create a new image cache
  Serial.println("Allocating memory");
  UBYTE *canvas;
  UDOUBLE Imagesize =
      ((EPD_10IN2G_WIDTH % 4 == 0) ? (EPD_10IN2G_WIDTH / 4)
                                   : (EPD_10IN2G_WIDTH / 4 + 1)) *
      EPD_10IN2G_HEIGHT;

  if ((canvas = (UBYTE *)ps_malloc(Imagesize)) == NULL) {
    Serial.println("Failed to allocate memory\n");
    Serial.flush();
    while (1) {
      digitalWrite(LED, HIGH);
      delay(100);
      digitalWrite(LED, LOW);
      delay(100);
    }
  }

  Serial.println("Painting");
  Paint_NewImage(canvas, EPD_10IN2G_WIDTH, EPD_10IN2G_HEIGHT, 0,
                 EPD_10IN2G_WHITE);
  Paint_SetScale(4);

  Paint_DrawRectangle(0, 0, 960, 160, EPD_10IN2G_BLACK, DOT_PIXEL_1X1,
                      DRAW_FILL_FULL);
  Paint_DrawRectangle(0, 160, 960, 320, EPD_10IN2G_WHITE, DOT_PIXEL_1X1,
                      DRAW_FILL_FULL);
  Paint_DrawRectangle(0, 320, 960, 480, EPD_10IN2G_YELLOW, DOT_PIXEL_1X1,
                      DRAW_FILL_FULL);
  Paint_DrawRectangle(0, 480, 960, 640, EPD_10IN2G_RED, DOT_PIXEL_1X1,
                      DRAW_FILL_FULL);

  Serial.println("Displaying");
  EPD_10IN2G_Display(canvas);
  DEV_Delay_ms(2000);

  Serial.println("Shutting down");
  EPD_10IN2G_Sleep();

  // Must be >2s
  DEV_Delay_ms(2000);

  // close 5V, Module enters 0 power consumption
  DEV_Module_Exit();
}

void loop() {
  delay(2000);
  Serial.println("Heartbeat");
}
