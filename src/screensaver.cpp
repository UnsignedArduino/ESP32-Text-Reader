#include <Arduino.h>
#include <SdFat.h>
#include "sdUtils.h"
#include <EPD.h>
#include <GUI_Paint.h>
#include "screensaver.h"

FsFile jpegFile;
JPEGDEC jpeg;

void* jpegOpen(const char* filename, int32_t* size) {
  jpegFile = sd.open(filename);
  *size = jpegFile.size();
  if (jpegFile) {
    Serial.print("Successfully opened file ");
  } else {
    Serial.print("Failed to open file ");
  }
  Serial.print(filename);
  Serial.print(" for JPEG reading! (Size: ");
  Serial.print(jpegFile.size());
  Serial.println(")");
  return &jpegFile;
}

void jpegClose(void* handle) {
  // FsFile* file = (FsFile*)(handle);
  // if (file != NULL) {
  //   file->close();
  // }
  if (!jpegFile) {
    Serial.println("Cannot close file for JPEG!");
    return;
  }
  Serial.println("Closed file for JPEG!");
  jpegFile.close();
}

int32_t jpegRead(JPEGFILE* handle, uint8_t* buffer, int32_t length) {
  // FsFile* file = (FsFile*)(handle);
  // if (file == NULL) {
  //   return 0;
  // }
  if (!jpegFile) {
    return 0;
  }
  return jpegFile.read(buffer, length);
}

int32_t jpegSeek(JPEGFILE* handle, int32_t position) {
  // FsFile* file = (FsFile*)(handle);
  // if (file == NULL) {
  //   return 0;
  // }
  if (!jpegFile) {
    return 0;
  }
  return jpegFile.seek(position);
}

int jpegDraw(JPEGDRAW* draw) {
  unsigned int x = draw->x;
  unsigned int y = draw->y;
  unsigned int w = draw->iWidth;
  unsigned int h = draw->iHeight;
  // Serial.print("X: ");
  // Serial.print(x);
  // Serial.print(" Y: ");
  // Serial.print(y);
  // Serial.print(" W: ");
  // Serial.print(w);
  // Serial.print(" H: ");
  // Serial.println(h);

  for (unsigned int ix = 0; ix < w; ix ++) {
    for (unsigned int iy = 0; iy < h; iy ++) {
      byte color;
      unsigned int pixelCol = draw->pPixels[ix + iy * w];
      if (pixelCol > 0xFFFF / 2) {
        color = WHITE;
      } else {
        color = BLACK;
      }
      unsigned int actualX = x + ix;
      unsigned int actualY = y + iy;
      if (actualX >= EPD_4IN2_HEIGHT || actualX < 0 ||
          actualY >= EPD_4IN2_WIDTH || actualY < 0) {
      //   Serial.print("Abnormal coordinates! (X: ");
      //   Serial.print(actualX);
      //   Serial.print(" Y: ");
      //   Serial.print(actualY);
      //   Serial.print(" Color: 0x");
      //   Serial.print(color, HEX);
      //   Serial.println(")");
        continue;
      }
      Paint_SetPixel(actualX, actualY, color);
    }
  }
  return 1;
}

bool drawScreensaver() {
  Serial.println("Attempting to draw screensaver");
  // unsigned long screensaverCount = filesInDir(SCREENSAVERS_PATH, IMAGE_EXT);
  // if (screensaverCount == 0) {
  //   Serial.println("Could not find any screensavers!");
  //   return false;
  // } else {
  //   Serial.print("Found ");
  //   Serial.print(screensaverCount);
  //   Serial.println(" screensavers");
  // }
  char tempPath[MAX_PATH_LEN] = {};
  if (!pickRandomFilename(SCREENSAVERS_PATH, IMAGE_EXT, tempPath, MAX_PATH_LEN)) {
    Serial.println("Failed to pick random filename!");
    return false;
  }
  char path[MAX_PATH_LEN] = {};
  strncat(path, SCREENSAVERS_PATH, MAX_PATH_LEN);
  strncat(path, "/", MAX_PATH_LEN);
  strncat(path, tempPath, MAX_PATH_LEN);
  Serial.print("Path selected: ");
  Serial.println(path);
  Serial.println("Opening JPEG image");
  if (jpeg.open(path, jpegOpen, jpegClose, jpegRead, jpegSeek, jpegDraw) == 1) {
    Serial.println("Successfully opened JPEG image!");
    Serial.print("Size: ");
    Serial.print(jpeg.getWidth());
    Serial.print("x");
    Serial.println(jpeg.getHeight());
    Paint_Clear(WHITE);
    // jpeg.setPixelType(ONE_BIT_DITHERED);
    // // you must provide your own RAM pointer which must hold at least width x 16 bytes
    // // https://github.com/bitbank2/JPEGDEC/wiki
    // const unsigned long bufSize = EPD_4IN2_HEIGHT * 16;
    // Serial.print("Using dither buffer of size ");
    // Serial.println(bufSize);
    // byte buffer[bufSize] = {};
    Serial.println("Decoding JPEG");
    if (jpeg.decode(0, 0, 0) == 1) {
      Serial.println("Successfully decoded JPEG image!");
    } else {
      jpeg.close();
      Serial.print("Failed to decode JPEG image! (Error: ");
      Serial.print(jpeg.getLastError());
      Serial.println(")");
      return false;
    }
    jpeg.close();
  } else {
    jpeg.close();
    Serial.print("Failed to open JPEG image! (Error: ");
    Serial.print(jpeg.getLastError());
    Serial.println(")");
    return false;
  }
  return true;
}
