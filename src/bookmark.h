#include <Arduino.h>
#include "sdUtils.h"
#include "config.h"

#define BOOKMARK_DIR_PATH "/.bookmarks"

// As a JSON document it should appear like this: (worst case scenario)
// {
//   "path": "/some really long filename for worst case json fileeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee.txt",
//   "size": 18446744073709551615,
//   "crc32": 4294967295,
//   "page": 4294967295
// }

// struct Bookmark {
//   char path[MAX_PATH_LEN];
//   unsigned long long size;
//   unsigned long crc32;
//   unsigned long page;
// };

// -- To serialize:
// StaticJsonDocument<384> doc;
// doc["path"] = "/some really long filename for worst case json fileeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee.txt";
// doc["size"] = 18446744073709552000;
// doc["crc32"] = 4294967295;
// doc["page"] = 4294967295;
// serializeJson(doc, output);
// -- To deserialize
// // Stream& input;
// StaticJsonDocument<384> doc;
// DeserializationError error = deserializeJson(doc, input);
// if (error) {
//   Serial.print("deserializeJson() failed: ");
//   Serial.println(error.c_str());
//   return;
// }
// const char* path = doc["path"]; // "/some really long filename for worst case json ...
// double size = doc["size"]; // 18446744073709552000
// long long crc32 = doc["crc32"]; // 4294967295
// long long page = doc["page"]; // 4294967295

bool getLastPage(const char* path, unsigned long long size, unsigned long crc32, unsigned long &result);
bool setLastPage(const char* path, unsigned long long size, unsigned long crc32, unsigned long page);
