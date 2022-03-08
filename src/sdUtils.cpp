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

bool fileAtIndex(const char* path, const char* ext, unsigned long index, char* result, byte size) {
  Serial.print("Finding name of file with index ");
  Serial.print(index);
  Serial.print(" at path: ");
  Serial.print(path);
  Serial.print(" (with extension: ");
  Serial.print(ext);
  Serial.println(")");
  unsigned long currIdx = 0;
  FsFile root = sd.open(path);
  while (true) {
    FsFile entry = root.openNextFile();
    if (!entry) {
      break;
    }
    Serial.print("File: ");
    entry.printName(&Serial);
    Serial.print(" - ");
    if (entry.isDirectory()) {
      Serial.println("skipped because of directory!");
      entry.close();
      continue;
    }
    char entryName[MAX_PATH_LEN];
    entry.getName(entryName, MAX_PATH_LEN);
    char* lastPeriod = strrchr(entryName, '.');
    if (lastPeriod == nullptr) {
      Serial.println("skipped because no extension!");
      continue;
    }
    char entryExt[MAX_PATH_LEN];
    strncpy(entryExt, lastPeriod, MAX_PATH_LEN);
    if (strcmp(ext, entryExt) != 0) {
      Serial.print("skipped because of non-matching extension! (");
      Serial.print(entryExt);
      Serial.print(" != ");
      Serial.print(ext);
      Serial.println(")");
      continue;
    }
    Serial.print("counted! (extension: ");
    Serial.print(entryExt);
    Serial.println(")");
    if (currIdx == index) {
      Serial.print("Found target file! (");
      char entryName[MAX_PATH_LEN];
      entry.getName(entryName, MAX_PATH_LEN);
      Serial.print(entryName);
      Serial.println(")");
      strncpy(result, entryName, size);
      return true;
    }
    currIdx ++;
    entry.close();
  }
  root.close();
  return false;
}

unsigned long filesInDir(const char* path, const char* ext) {
  Serial.print("Counting files in: ");
  Serial.print(path);
  Serial.print(" (with extension: ");
  Serial.print(ext);
  Serial.println(")");
  unsigned long fileCount = 0;
  FsFile root = sd.open(path);
  while (true) {
    FsFile entry = root.openNextFile();
    if (!entry) {
      break;
    }
    Serial.print("File: ");
    entry.printName(&Serial);
    Serial.print(" - ");
    if (entry.isDirectory()) {
      Serial.println("skipped because of directory!");
      entry.close();
      continue;
    }
    char entryName[MAX_PATH_LEN];
    entry.getName(entryName, MAX_PATH_LEN);
    char* lastPeriod = strrchr(entryName, '.');
    if (lastPeriod == nullptr) {
      Serial.println("skipped because no extension!");
      continue;
    }
    char entryExt[MAX_PATH_LEN];
    strncpy(entryExt, lastPeriod, MAX_PATH_LEN);
    if (strcmp(ext, entryExt) != 0) {
      Serial.print("skipped because of non-matching extension! (");
      Serial.print(entryExt);
      Serial.print(" != ");
      Serial.print(ext);
      Serial.println(")");
      continue;
    }
    Serial.print("counted! (extension: ");
    Serial.print(entryExt);
    Serial.println(")");
    fileCount ++;
    entry.close();
  }
  root.close();
  Serial.print("File count: ");
  Serial.println(fileCount);
  return fileCount;
}

unsigned long filesInDir(const char* path) {
  Serial.print("Counting files in: ");
  Serial.println(path);
  unsigned long fileCount = 0;
  FsFile root = sd.open(path);
  while (true) {
    FsFile entry = root.openNextFile();
    if (!entry) {
      break;
    }
    if (entry.isDirectory()) {
      Serial.print("Skip directory: ");
      entry.printName(&Serial);
      entry.close();
      continue;
    }
    Serial.print("Found file: ");
    entry.printName(&Serial);
    Serial.println();
    fileCount ++;
    entry.close();
  }
  root.close();
  Serial.print("File count: ");
  Serial.println(fileCount);
  return fileCount;
}

unsigned long filesInRoot() {
  return filesInDir("/");
}

bool pickRandomFilename(const char* path, const char* ext, char* result, byte size) {
  Serial.print("Picking random file from ");
  Serial.println(path);
  unsigned long fileCount = filesInDir(path, ext);
  if (fileCount == 0) {
    Serial.println("Could not find any files!");
    return false;
  }
  Serial.print("Found ");
  Serial.print(fileCount);
  Serial.print(" files with ");
  Serial.print(ext);
  Serial.println(" extension");
  unsigned long index = random(fileCount);
  if (fileAtIndex(path, ext, index, result, size)) {
    Serial.println("Picked random file!");
    return true;
  } else {
    Serial.println("Failed to pick random file!");
    return false;
  }
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
