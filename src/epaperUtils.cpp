#include <Arduino.h>
#include <DEV_Config.h>
#include <EPD.h>
#include <GUI_Paint.h>
#include <epaperUtils.h>

const unsigned int bwImageSize = ((EPD_4IN2_WIDTH % 8 == 0) ? (EPD_4IN2_WIDTH / 8 ) : (EPD_4IN2_WIDTH / 8 + 1)) * EPD_4IN2_HEIGHT;
byte imageBuffer[bwImageSize] = {};

int beginEPaper() {
  Serial.println("Initializing EPaper display...");
  if (DEV_Module_Init() != 0) {
    Serial.println("Failed to configure SPI and GPIO for EPaper display!");
    return 1;
  }
  EPD_4IN2_Init();
  // EPD_4IN2_Clear();
  Serial.println("Successfully initalized EPaper display!");

  Serial.println("Preparing image buffer...");
  Paint_NewImage(imageBuffer, EPD_4IN2_WIDTH, EPD_4IN2_HEIGHT, ROTATE_90, WHITE);
  Paint_SelectImage(imageBuffer);
  Paint_Clear(WHITE);
  Serial.println("Successfully prepared image buffer!");
  return 0;
}

void updateDisplay() {
  Serial.println("Showing contents");
  EPD_4IN2_Display(imageBuffer);
}

void updateDisplayPartial(unsigned int startx, unsigned int starty, unsigned int endx, unsigned int endy) {
  Serial.println("Showing contents via partial refresh");
  Serial.print("From (");
  Serial.print(startx);
  Serial.print(", ");
  Serial.print(starty);
  Serial.print(") to (");
  Serial.print(endx);
  Serial.print(", ");
  Serial.print(endy);
  Serial.println(")");
  EPD_4IN2_PartialDisplay(startx, starty, endx, endy, imageBuffer);
}
