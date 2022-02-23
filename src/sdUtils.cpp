#include <Arduino.h>
#include <SdFat.h>
#include "pinouts.h"
#include "sdUtils.h"

SdFat sd;

int beginSD() {
  Serial.println("Initializing SD card...");
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorPrint();
    Serial.println("Error initializing SD card!");
    return 1;
  }
  Serial.println("Successfully initialized SD card!");
  return 0;
}

unsigned int filesInRoot() {
  unsigned int fileCount = 0;
  FsFile root = sd.open("/");
  while (true) {
    FsFile entry = root.openNextFile();
    if (!entry) {
      break;
    }
    if (entry.isDirectory()) {
      continue;
    }
    Serial.print("Found file: ");
    // Serial.println(entry.name());
    entry.printName(&Serial);
    Serial.println();
    entry.close();
    fileCount ++;
  }
  root.close();
  Serial.print("File count: ");
  Serial.println(fileCount);
  return fileCount;
}

bool seekRelative(FsFile f, long difference) {
  return f.seek(f.position() + difference);
}

int readBackwards(FsFile f) {
  if (!seekRelative(f, -1)) {
    return -1;
  }
  int theByte = f.read();
  seekRelative(f, -1);
  return theByte;
}

int peekBefore(FsFile f) {
  if (!seekRelative(f, -1)) {
    return -1;
  }
  return f.read();
}

int peekAtRelative(FsFile f, long difference) {
  unsigned long long position = f.position();
  if (!seekRelative(f, difference)) {
    return -1;
  }
  int value = f.read();
  f.seek(position);
  return value;
}
