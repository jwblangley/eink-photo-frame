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

  UBYTE *temp; if((temp = (UBYTE*) ps_malloc(5)) == NULL) {
      Serial.printf("Failed to apply for lang memory...\r\n");
  }
  else
  {
    Serial.printf("Successful lang malloc: %d\n", temp);
  }
  Serial.flush();
  delay(50);

  // Explicitly bind SPI to the XIAO S3 hardware pins (SCK=7, MISO=-1, MOSI=9, SS=4)
  // SPI.begin(7, -1, 9, 4);
  SPI.begin(8, -1, 10, 2);

  // Initialize the display board
  DEV_Module_Init();

  Serial.printf("e-Paper Init and Clear...\r\n");
  EPD_10IN2G_Init();
  EPD_10IN2G_Clear(EPD_10IN2G_WHITE); // White
  DEV_Delay_ms(2000);

  // Create a new image cache
  UBYTE *BlackImage;
  UDOUBLE Imagesize = ((EPD_10IN2G_WIDTH % 4 == 0)? (EPD_10IN2G_WIDTH / 4 ): (EPD_10IN2G_WIDTH / 4 + 1)) * EPD_10IN2G_HEIGHT;
  Serial.printf("ImageSize=%d\n", Imagesize);
  Serial.printf("Max alloc=%d\n", ESP.getMaxAllocPsram());
  Serial.flush();
  delay(500);

  if((BlackImage = (UBYTE*) ps_malloc(Imagesize)) == NULL) {
      Serial.printf("Failed to apply for james memory...\r\n");
  }
  else
  {
    Serial.printf("Successful james malloc: %d\n", BlackImage);
  }
  Serial.flush();
  delay(500);


  if((BlackImage = (UBYTE*) ps_malloc(Imagesize)) == NULL) {
      Serial.printf("Failed to apply for black memory...\r\n");
      Serial.flush();
      while(1)
      {
        digitalWrite(LED, HIGH);
        delay(100);
        digitalWrite(LED, LOW);
        delay(100);
      }
  }
  Serial.printf("Paint_NewImage\r\n");
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
  delay(1000);
  digitalWrite(LED, !digitalRead(LED));
}
