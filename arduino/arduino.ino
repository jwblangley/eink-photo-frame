#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <esp_task_wdt.h> // Required for ESP32 watchdog control

#include "src/waveshare/10in2g/EPD_10in2g.h"
#include "src/waveshare/10in2g/GUI_Paint.h"
#include "src/waveshare/10in2g/DEV_Config.h"

#include "src/network_credentials.h"


#define BAUD 115200
#define LED 21


void setup() {
  Serial.begin(BAUD);
  pinMode(LED, OUTPUT);

  // Force panic handler to print full backtrace instead of silent reboot
  esp_reset_reason_t reason = esp_reset_reason();
  Serial.printf("Reset reason: %d\n", reason);

  SPI.begin(EPD_SCK_PIN, EPD_MISO_PIN, EPD_MOSI_PIN, -1);

  // Initialize the display board
  DEV_Module_Init();

  Serial.println("e-Paper Init");
  EPD_10IN2G_Init();
  Serial.println("e-Paper Clear");
  EPD_10IN2G_Clear(EPD_10IN2G_WHITE); // White
  DEV_Delay_ms(1000);

  // Create a new image cache
  UBYTE *BlackImage;
  UDOUBLE Imagesize = ((EPD_10IN2G_WIDTH % 4 == 0)? (EPD_10IN2G_WIDTH / 4 ): (EPD_10IN2G_WIDTH / 4 + 1)) * EPD_10IN2G_HEIGHT;

  if((BlackImage = (UBYTE*) ps_malloc(Imagesize)) == NULL) {
      Serial.println("Failed to allocate memory\n");
      Serial.flush();
      while(1)
      {
        digitalWrite(LED, HIGH);
        delay(100);
        digitalWrite(LED, LOW);
        delay(100);
      }
  }

  Serial.println("Paint_NewImage");
  Serial.flush();
  delay(50);
  Paint_NewImage(BlackImage, EPD_10IN2G_WIDTH, EPD_10IN2G_HEIGHT, 0, EPD_10IN2G_WHITE);
  Paint_SetScale(4);

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
  // Serial.printf("\nWiFi connected. IP address: %s\n", WiFi.localIP().toString().c_str());

  Paint_DrawRectangle(20, 70, 70, 120, EPD_10IN2G_YELLOW, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
  Paint_DrawRectangle(80, 70, 130, 120, EPD_10IN2G_YELLOW, DOT_PIXEL_1X1, DRAW_FILL_FULL);

  EPD_10IN2G_Display(BlackImage);

  EPD_10IN2G_Sleep();
  DEV_Delay_ms(2000);
  DEV_Module_Exit();
}

void loop() {
  delay(2000);
  digitalWrite(LED, !digitalRead(LED));
  Serial.println("Heartbeat");
}
