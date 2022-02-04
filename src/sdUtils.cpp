#include <Arduino.h>
#include <SD.h>
#include "pinouts.h"
#include "sdUtils.h"

int beginSD() {
  Serial.println("Initializing SD card...");
  if (!SD.begin(sdCS)) {
    Serial.println("Error initializing SD card!");
    return 1;
  }
  Serial.println("Successfully initialized SD card!");
  return 0;
}

unsigned int filesInRoot() {
  unsigned int fileCount = 0;
  File root = SD.open("/");
  while (true) {
    File entry = root.openNextFile();
    if (!entry) {
      break;
    }
    if (entry.isDirectory()) {
      continue;
    }
    Serial.print("Found file: ");
    Serial.println(entry.name());
    entry.close();
    fileCount ++;
  }
  root.close();
  Serial.print("File count: ");
  Serial.println(fileCount);
  return fileCount;
}

bool seekRelative(File f, long difference) {
  return f.seek(f.position() + difference);
}

int readBackwards(File f) {
  if (!seekRelative(f, -1)) {
    return -1;
  }
  int theByte = f.read();
  seekRelative(f, -1);
  return theByte;
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
