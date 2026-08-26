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

  // Explicitly bind SPI to the XIAO S3 hardware pins (SCK=7, MISO=-1, MOSI=9, SS=4)
  SPI.begin(7, -1, 9, 4);

  // Force panic handler to print full backtrace instead of silent reboot
  esp_reset_reason_t reason = esp_reset_reason();
  Serial.printf("Reset reason: %d\n", reason);


  // --- FIX: Extend Task Watchdog Timeout to 30 Seconds ---
  // This stops the ESP32-S3 from panicking during long e-paper refresh cycles
  // esp_task_wdt_config_t twdt_config = {
  //   .timeout_ms = 30000, // 30 seconds timeout
  //   .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
  //   .trigger_panic = true
  // };

  // esp_task_wdt_reconfigure(&twdt_config);
  // esp_task_wdt_add(NULL); // Add current loop task to watchdog tracking

  // Reset watchdog timer before heavy lifting
  // esp_task_wdt_reset();

  // Initialize the display board
  DEV_Module_Init();

  Serial.printf("e-Paper Init and Clear...\r\n");
  EPD_10IN2G_Init();
  EPD_10IN2G_Clear(EPD_10IN2G_WHITE); // White
  // esp_task_wdt_reset();
  DEV_Delay_ms(2000);

  // Create a new image cache
  UBYTE *BlackImage;
  UWORD Imagesize = ((EPD_10IN2G_WIDTH % 4 == 0)? (EPD_10IN2G_WIDTH / 4 ): (EPD_10IN2G_WIDTH / 4 + 1)) * EPD_10IN2G_HEIGHT;
  if((BlackImage = (UBYTE*) malloc(Imagesize)) == NULL) {
      printf("Failed to apply for black memory...\r\n");
      while(1);
  }
  printf("Paint_NewImage\r\n");
  Paint_NewImage(BlackImage, EPD_10IN2G_WIDTH, EPD_10IN2G_HEIGHT, 0, EPD_10IN2G_WHITE);
  // esp_task_wdt_reset();
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
  // esp_task_wdt_reset();
  Paint_DrawRectangle(80, 70, 130, 120, EPD_10IN2G_YELLOW, DOT_PIXEL_1X1, DRAW_FILL_FULL);
  // esp_task_wdt_reset();

  EPD_10IN2G_Sleep();
  DEV_Delay_ms(2000);
  DEV_Module_Exit();
}

void loop() {
  delay(1000);
  digitalWrite(LED, !digitalRead(LED));
  // esp_task_wdt_reset(); // Feed watchdog in loop as well
}
