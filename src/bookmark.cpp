#include <Arduino.h>
#include <SdFat.h>
#include <ArduinoJson.h>
#include "sdUtils.h"
#include "bookmark.h"

bool getLastPage(const char* path, unsigned long long size, unsigned long crc32, unsigned long &result) {
  char bookmarkPath[MAX_PATH_LEN];
  memset(bookmarkPath, 0, MAX_PATH_LEN);
  strncat(bookmarkPath, BOOKMARK_DIR_PATH, MAX_PATH_LEN);
  strncat(bookmarkPath, "/", MAX_PATH_LEN);
  strncat(bookmarkPath, path, MAX_PATH_LEN);
  Serial.print("Looking for bookmark file at ");
  Serial.println(bookmarkPath);
  if (sd.exists(bookmarkPath)) {
    Serial.print("Opening file ");
    Serial.print(bookmarkPath);
    Serial.println(" for bookmark");
    FsFile bookmarkFile = sd.open(bookmarkPath);
    if (!bookmarkFile) {
      Serial.println("Failed to open bookmark file for writing!");
      return false;
    }
    StaticJsonDocument<384> doc;
    DeserializationError error = deserializeJson(doc, bookmarkFile);
    bookmarkFile.close();
    if (error) {
      Serial.print("Failed to deserialize JSON: ");
      Serial.println(error.c_str());
      return false;
    }
    const char* jsonPath = doc["path"];
    unsigned long long jsonSize = doc["size"];
    unsigned long jsonCrc32 = doc["crc32"];
    unsigned long jsonPage = doc["page"];
    if (strcasecmp(path, jsonPath) != 0) {
      Serial.print("Paths are different! (");
      Serial.print(path);
      Serial.print(" != ");
      Serial.print(jsonPath);
      Serial.println(")");
      return false;
    } else if (size != jsonSize) {
      Serial.print("Sizes are different! (");
      Serial.print(size);
      Serial.print(" != ");
      Serial.print(jsonSize, HEX);
      Serial.println(")");
      return false;
    } else if (crc32 != jsonCrc32) {
      Serial.print("CRC32s are different! (0x");
      Serial.print(crc32, HEX);
      Serial.print(" != 0x");
      Serial.print(jsonCrc32, HEX);
      Serial.println(")");
      return false;
    } else {
      Serial.print("Old page: ");
      Serial.println(jsonPage);
      result = jsonPage;
      Serial.println("Successfully read bookmark!");
      return true;
    }
  } else {
    Serial.println("No last page has been set!");
    return false;
  }
}

bool setLastPage(const char* path, unsigned long long size, unsigned long crc32, unsigned long page) {
  char bookmarkPath[MAX_PATH_LEN];
  memset(bookmarkPath, 0, MAX_PATH_LEN);
  strncat(bookmarkPath, BOOKMARK_DIR_PATH, MAX_PATH_LEN);
  strncat(bookmarkPath, "/", MAX_PATH_LEN);
  strncat(bookmarkPath, path, MAX_PATH_LEN);
  if (!sd.exists(BOOKMARK_DIR_PATH)) {
    Serial.print("Bookmark directory ");
    Serial.print(BOOKMARK_DIR_PATH);
    Serial.println(" does not exist, creating it!");
    sd.mkdir(BOOKMARK_DIR_PATH);
  }
  Serial.print("Opening file ");
  Serial.print(bookmarkPath);
  Serial.println(" for bookmark");
  FsFile bookmarkFile = sd.open(bookmarkPath, O_WRITE | O_CREAT | O_TRUNC);
  if (!bookmarkFile) {
    Serial.println("Failed to open bookmark file for writing!");
    return false;
  }
  StaticJsonDocument<384> doc;
  Serial.print("Path: ");
  Serial.println(path);
  doc["path"] = path;
  Serial.print("Size: ");
  Serial.println(size);
  doc["size"] = size;
  Serial.print("CRC32: 0x");
  Serial.println(crc32, HEX);
  doc["crc32"] = crc32;
  Serial.print("Last page: ");
  Serial.println(page);
  doc["page"] = page;
  serializeJson(doc, bookmarkFile);
  bookmarkFile.close();
  Serial.println("Successfully written bookmark!");
  return true;
}
