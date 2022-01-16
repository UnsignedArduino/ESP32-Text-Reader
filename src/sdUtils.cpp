#include <Arduino.h>
#include <SD.h>

int beginSD(byte chipSelect) {
  Serial.println("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("Error initializing SD card!");
    return 1;
  }
  Serial.println("Successfully initialized SD card!");
  return 0;
}
