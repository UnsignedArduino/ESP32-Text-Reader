#include <Arduino.h>
#include <SD.h>
#include "utils.h"
#include "sdUtils.h"
#include "epaperUtils.h"
#include "pinouts.h"

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  Serial.println();

  if (beginSD(sdCS) != 0) {
    blinkError(100, 400);
  }
  if (beginEPaper() != 0) {
    blinkError(250, 250);
  }
}

void loop() {
  
}
