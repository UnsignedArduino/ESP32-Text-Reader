#include <Arduino.h>
#include "DEV_Config.h"
#include "EPD.h"

int beginEPaper() {
  Serial.println("Initializing EPaper display...");
  if (DEV_Module_Init() != 0) {
    Serial.println("Failed to configure SPI and GPIO for EPaper display!");
    return 1;
  }
  EPD_4IN2_Init();
  EPD_4IN2_Clear();
  Serial.println("Successfully initalized EPaper display!");
  return 0;
}
