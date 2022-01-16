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

bool seekRelative(File f, long difference) {
  return f.seek(f.position() + difference);
}

int peekBefore(File f) {
  if (!seekRelative(f, -1)) {
    return -1;
  }
  return f.read();
}

int peekAtRelative(File f, long difference) {
  unsigned long position = f.position();
  if (!seekRelative(f, difference)) {
    return -1;
  }
  int value = f.read();
  f.seek(position);
  return value;
}
