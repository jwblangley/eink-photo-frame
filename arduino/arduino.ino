#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <esp_task_wdt.h> // Required for ESP32 watchdog control

#include "src/waveshare/10in2g/EPD_10in2g.h"
#include "src/waveshare/10in2g/GUI_Paint.h"
#include "src/waveshare/10in2g/DEV_Config.h"
#include "src/waveshare/10in2g/ImageData.h"

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
  EPD_10IN2G_Clear(EPD_10IN2G_WHITE);
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

  printf("Drawing:BlackImage\r\n");
  // Paint_DrawPoint(10, 80, EPD_10IN2G_RED, DOT_PIXEL_1X1, DOT_STYLE_DFT);
  // Paint_DrawPoint(10, 90, EPD_10IN2G_YELLOW, DOT_PIXEL_2X2, DOT_STYLE_DFT);
  // Paint_DrawPoint(10, 100, EPD_10IN2G_BLACK, DOT_PIXEL_3X3, DOT_STYLE_DFT);
  // Paint_DrawLine(20, 70, 70, 120, EPD_10IN2G_RED, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
  // Paint_DrawLine(70, 70, 20, 120, EPD_10IN2G_RED, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
  // Paint_DrawRectangle(20, 70, 70, 120, EPD_10IN2G_YELLOW, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
  // Paint_DrawRectangle(80, 70, 130, 120, EPD_10IN2G_YELLOW, DOT_PIXEL_1X1, DRAW_FILL_FULL);
  // Paint_DrawCircle(45, 95, 20, EPD_10IN2G_BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
  // Paint_DrawCircle(105, 95, 20, EPD_10IN2G_BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
  // Paint_DrawLine(85, 95, 125, 95, EPD_10IN2G_RED, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
  // Paint_DrawLine(105, 75, 105, 115, EPD_10IN2G_YELLOW, EPD_10in2g_ReadBusyDOT_PIXEL_1X1, LINE_STYLE_DOTTED);
  // Paint_DrawString_EN(10, 0, "Red,yellow,white and black", &Font16, EPD_10IN2G_RED, EPD_10IN2G_YELLOW);
  // Paint_DrawString_EN(10, 20, "Four color e-Paper", &Font12, EPD_10IN2G_YELLOW, EPD_10IN2G_BLACK);
  // Paint_DrawString_CN(150, 20, "微雪电子", &Font24CN, EPD_10IN2G_RED, EPD_10IN2G_WHITE);
  // Paint_DrawNum(10, 35, 123456, &Font12, EPD_10IN2G_RED, EPD_10IN2G_BLACK);

  Paint_DrawRectangle(0,   0,   960, 160, EPD_10IN2G_BLACK,  DOT_PIXEL_1X1, DRAW_FILL_FULL);
  Paint_DrawRectangle(0,   160, 960, 320, EPD_10IN2G_WHITE,  DOT_PIXEL_1X1, DRAW_FILL_FULL);
  Paint_DrawRectangle(0,   320, 960, 480, EPD_10IN2G_YELLOW, DOT_PIXEL_1X1, DRAW_FILL_FULL);
  Paint_DrawRectangle(0,   480, 960, 640, EPD_10IN2G_RED,    DOT_PIXEL_1X1, DRAW_FILL_FULL);

  for (size_t i = 0; i < Imagesize; i++)
  {
    if (BlackImage[i] != 0)
    {
      Serial.printf("Non zero: %d: %d\n", i, BlackImage[i]);
    }
  }
  Serial.printf("D10: %d\n", D10);

  Serial.println("EPD_Display");
  EPD_10IN2G_Display(BlackImage);
  DEV_Delay_ms(10'000);
  EPD_10in2g_ReadBusy();

  // printf("Clear...\r\n");
  // EPD_10IN2G_Clear(EPD_10IN2G_WHITE);

  Serial.println("Goto Sleep");
  EPD_10IN2G_Sleep();
  // free(BlackImage);
  // BlackImage = NULL;
  DEV_Delay_ms(10'000);//important, at least 2s
  // close 5V
  Serial.println("close 5V, Module enters 0 power consumption");
  DEV_Module_Exit();

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
}

void loop() {
  delay(2000);
  digitalWrite(LED, !digitalRead(LED));
  Serial.println("Heartbeat");
}
