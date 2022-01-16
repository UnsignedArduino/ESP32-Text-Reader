#include <Arduino.h>
#include <SD.h>
#include "pinouts.h"

int beginSD() {
  Serial.println("Initializing SD card...");
  if (!SD.begin(sdCS)) {
    Serial.println("Error initializing SD card!");
    return 1;
  }
  Serial.println("Successfully initialized SD card!");
  return 0;
}
