#include <Arduino.h>
#include <SD.h>
#include "utils.h"
#include "sdUtils.h"
#include "epaperUtils.h"
#include <GUI_Paint.h>
#include "buttonUtils.h"

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  Serial.println();

  if (beginSD() != 0) {
    blinkError(100, 400);
  }
  if (beginEPaper() != 0) {
    blinkError(250, 250);
  }

  beginButtons();

  Paint_DrawString_EN(0, 0, "`1234567890-=qwertyuiop[]\\asdfghjkl;'zxcvbnm,./~!@#$%^&*()_+QWERTYUIOP{}|ASDFGHJKL:\"ZXCVBNM<>?", &Font12, WHITE, BLACK);
  EPD_4IN2_Display(imageBuffer);
}

void loop() {
  
}
