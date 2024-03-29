#include <Arduino.h>
#include <SdFat.h>
#include <CRC32.h>
#include "utils.h"
#include "sdUtils.h"
#include "buttonUtils.h"
#include "battUtils.h"
#include "epaperUtils.h"
#include <GUI_Paint.h>
#include "textReader.h"
#include "config.h"
#include "profiling.h"

FsFile file;
unsigned long long filePos = 0;
unsigned long long nextFilePos = 0;

unsigned long long fileSize = 0;
unsigned long bytesOnScreen = 0;

const byte maxCharPerLine = 40;
const byte maxLines = 30;

void drawScrollbar(unsigned int startX, unsigned int startY, 
                   unsigned int width, unsigned int height,
                   unsigned long startFill, unsigned long endFill, unsigned long maximum) {
  Serial.print("Drawing scrollbar (");
  Serial.print(startFill);
  Serial.print(" - ");
  Serial.print(endFill);
  Serial.print(") / ");
  Serial.print(maximum);
  Serial.println(")");
  Paint_DrawRectangle(startX, startY, startX + width, startY + height, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
  unsigned int actualStartY, actualEndY;
  if (maximum == 0) {
    actualStartY = startY + 1;
    actualEndY = startY + height;
  } else {
    actualStartY = map(startFill, 0, maximum, startY, startY + height);
    actualEndY = map(endFill, 0, maximum, startY, startY + height);
  }
  Serial.print("Drawing from ");
  Serial.print(actualStartY);
  Serial.print(" to ");
  Serial.print(actualEndY);
  Serial.print(" (range ");
  Serial.print(startY);
  Serial.print(" to ");
  Serial.print(startY + height);
  Serial.println(")");
  Paint_DrawRectangle(startX + 1, actualStartY,
                      startX + width - 1, actualEndY,
                      BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
}

void drawTextReaderScrollbar(unsigned long long startFilePos, unsigned long long endFilePos, unsigned long long fileSize) {
  drawScrollbar(EPD_4IN2_HEIGHT - 3, 1, 3, EPD_4IN2_WIDTH - 2, startFilePos, endFilePos, fileSize);
}

void drawDialog(const char* lines[], byte lineCount) {
  Paint_DrawRectangle(0, EPD_4IN2_WIDTH - (13 * lineCount) - 2,
                      EPD_4IN2_HEIGHT, EPD_4IN2_WIDTH,
                      WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
  Paint_DrawRectangle(1, EPD_4IN2_WIDTH - (13 * lineCount) - 2,
                      EPD_4IN2_HEIGHT, EPD_4IN2_WIDTH,
                      BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
  for (int i = 0; i < lineCount; i ++) {
    Serial.print("Line ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(lines[i]);
    Paint_DrawString_EN(2, EPD_4IN2_WIDTH - (13 * (lineCount - i)), lines[i], &Font12, WHITE, BLACK);
  }
}

void drawTextReaderMenu(const char *filename, unsigned long page, unsigned long maxPage) {
  const byte linesToStickUp = 7;
  Serial.println("Drawing text reader menu");
  const byte bufSize = maxCharPerLine + 1;
  char fileBuf[bufSize];
  snprintf(fileBuf, bufSize, "File: %s", filename);
  if (6 + strlen(filename) > maxCharPerLine) {
    for (int i = 4; i > 1; i --) {
      fileBuf[maxCharPerLine - i] = '.';
    }
    fileBuf[maxCharPerLine - 1] = '\0';
  }
  char pageBuf[bufSize];
  snprintf(pageBuf, bufSize, "Page: %lu / %lu", page, maxPage);
  char battBuf[bufSize];
  // snprintf(battBuf, bufSize, "Battery: %#.2Fv", (float)readBatt() / 1000.0);
  snprintf(battBuf, bufSize, "Battery: %u%%", readPercent());
  const char *lines[linesToStickUp] = {
    fileBuf,
    pageBuf,
    battBuf,
    "",
    "Left: Exit",
    "Middle: Cancel",
    "Right: Go to page"
  };
  drawDialog(lines, linesToStickUp);
}

unsigned long askForPage(unsigned long curPg, unsigned long maxPg) {
  working();
  unsigned long value = curPg;
  Serial.println("Drawing page selector menu");
  notWorking();
  bool update = true;
  while (true) {
    if (update) {
      working();
      Paint_Clear(WHITE);
      // Seek back to beginning of page and reprint it
      file.seek(filePos);
      nextFilePos = printFromLocation(maxLines);
      const byte linesToStickUp = 9;
      const unsigned int bufSize = maxCharPerLine + 1;
      char curPageBuf[bufSize];
      snprintf(curPageBuf, bufSize, "Current page: %lu / %lu", curPg + 1, maxPg + 1);
      char goPageBuf[bufSize];
      snprintf(goPageBuf, bufSize, "Go to page: %lu", value + 1);
      const char *lines[linesToStickUp] = {
        "What page would you like to go to?",
        "",
        curPageBuf,
        "",
        goPageBuf,
        "",
        "Left: Decrement",
        "Middle: Enter",
        "Right: Increment"
      };
      drawDialog(lines, linesToStickUp);
      updateDisplay();
      update = false;
      notWorking();
    }
    byte pressed = waitForButtonPress();
    if (pressed == CENTER_BUTTON) {
      break;
    } else {
      unsigned long startBtnPress = millis();
      while (anyButtonPressed()) {
        unsigned long blinkDelay;
        if (millis() - startBtnPress > 9000) {
          blinkDelay = 25;
        } else if (millis() - startBtnPress > 6000) {
          blinkDelay = 50;
        } else if (millis() - startBtnPress > 3000) {
          blinkDelay = 100;
        } else {
          blinkDelay = 200;
        }
        working();
        delay(blinkDelay);
        notWorking();
        delay(blinkDelay);
      }
      unsigned long endBtnPress = millis();
      unsigned long btnDiff = endBtnPress - startBtnPress;
      unsigned int changeBy;
      if (btnDiff > 9000) {
        changeBy = 100;
      } else if (btnDiff > 6000) {
        changeBy = 50;
      } else if (btnDiff > 3000) {
        changeBy = 10;
      } else {
        changeBy = 1;
      }
      Serial.print("Change by ");
      Serial.println(changeBy);
      if (pressed == LEFT_BUTTON) {
        if (changeBy > value) {
          value = 0;
        } else {
          value -= changeBy;
        }
        update = true;
      } else if (pressed == RIGHT_BUTTON) {
        value = min(value + changeBy, maxPg);
        update = true;
      }
    }
  }
  working();
  Serial.print("User selected page: ");
  Serial.println(value);
  return value;
}

void noSdScreen() {
  Paint_Clear(WHITE);
  if (sd.sdErrorCode() == SD_CARD_ERROR_CMD0) {
    const byte linesToStickUp = 4;
    const char *lines[linesToStickUp] = {
      "No SD card is inserted!",
      "",
      "Please insert an SD card and press",
      "any button to try again!"
    };
    drawDialog(lines, linesToStickUp);
  } else {
    const byte linesToStickUp = 6;
    const byte bufSize = maxCharPerLine + 1;
    char errorBuf[bufSize];
    snprintf(errorBuf, bufSize, 
             "Error: 0x%02X, 0x%02X", 
             sd.sdErrorCode(), sd.sdErrorData());
    const char *lines[linesToStickUp] = {
      "SD card error!",
      "",
      errorBuf,
      "",
      "Go to https://bit.ly/SdFatErrorCodes for",
      "error code list"
    };
    drawDialog(lines, linesToStickUp);
  }
  updateDisplay();
}

void noTextFilesScreen() {
  const byte linesToStickUp = 5;
  const char *lines[linesToStickUp] = {
    "No files on SD card!"
    "",
    "Please add some text files to the SD ",
    "card, then turn the device off and ",
    "back on!"
  };
  Paint_Clear(WHITE);
  drawDialog(lines, linesToStickUp);
  updateDisplay();
}

void loadingTextFile(const char* filePath) {
  char tempLine[maxCharPerLine + 1];
  strncpy(tempLine, filePath, maxCharPerLine + 1);
  const byte linesToStickUp = 2;
  if (strlen(tempLine) > maxCharPerLine) {
    tempLine[maxCharPerLine - 3] = '\0';
    strncat(tempLine, "...", maxCharPerLine + 1);
  }
  const char *lines[linesToStickUp] = {
    "Loading text file:",
    tempLine
  };
  Paint_Clear(WHITE);
  drawDialog(lines, linesToStickUp);
  updateDisplay();
}

unsigned long long printFromLocation(unsigned int rows) {
  Paint_Clear(WHITE);

  unsigned long long startFilePos = file.position();

  Serial.print("Reading from location ");
  Serial.println(startFilePos);

  Serial.println("Loading from file");
  bytesOnScreen = 0;
  // Serial.println("Got: ");
  for (unsigned int row = 0; row < rows; row ++) {
    unsigned int yPos = row * 13;
    char line[maxCharPerLine + 1] = "";
    byte i = 0;
    for ( ; i < maxCharPerLine; i ++) {
      int nextByte = file.read();
      bytesOnScreen ++;
      if (nextByte == -1) {
        break;
      }
      char ch = (char)nextByte;
      if (ch == '\n') { // \n
        break;
      } else if (ch == '\r') {  // \r\n
        file.read();  // Advance forward one more because we still have \n to consume
        bytesOnScreen ++;
        break;
      } else {
        line[i] = ch;
      }
    }
    line[i] = '\0';
    // Serial.println(line);

    Paint_DrawString_EN(0, yPos, line, &Font12, WHITE, BLACK);
  }

  unsigned long long endFilePos = file.position();

  Serial.print("Ended at position: "); 
  Serial.println(endFilePos);

  Serial.print("Bytes on screen: ");
  Serial.println(bytesOnScreen);

  drawTextReaderScrollbar(startFilePos, endFilePos, fileSize);

  return file.position();
}

unsigned long long getPosFromPage(unsigned long page, unsigned int rows) {
  file.seek(0);
  for (int p = 0; p < page; p ++) {
    for (unsigned int row = 0; row < rows; row ++) {
      for (byte i = 0; i < maxCharPerLine; i ++) {
        int nextByte = file.read();
        if (nextByte == -1) {
          break;
        }
        char ch = (char)nextByte;
        if (ch == '\n') { // \n
          break;
        } else if (ch == '\r') {  // \r\n
          file.read();  // Advance forward one more because we still have \n to consume
          break;
        }
      }
    }
  }

  unsigned long long endFilePos = file.position();

  Serial.print("Ended at position: "); 
  Serial.println(endFilePos);

  return endFilePos;
}

unsigned long getMaxPage(unsigned int rows, unsigned long& crc) {
  Serial.println("Finding max page");
  unsigned long startTime = millis();
  CRC32 crc32;
  unsigned long page = 0;
  file.seek(0);
  #if defined(PROFILE_GET_MAX_PAGE)
  startProfiling();
  #endif
  while (true) {
    for (unsigned int row = 0; row < rows; row ++) {
      for (byte i = 0; i < maxCharPerLine; i ++) {
        #if defined(PROFILE_GET_MAX_PAGE)
        startSection(FILE_READ_NEXTBYTE);
        #endif
        int nextByte = file.read();
        #if defined(PROFILE_GET_MAX_PAGE)
        endSection(FILE_READ_NEXTBYTE);
        #endif
        if (nextByte == -1) {
          #if defined(PROFILE_GET_MAX_PAGE)
          endProfiling();
          #endif
          Serial.print("Last page is ");
          Serial.println(page);
          crc = crc32.finalize();
          Serial.print("CRC32 is 0x");
          Serial.println(crc, HEX);
          unsigned long endTime = millis();
          float elapsedTime = (endTime - startTime) / 1000.0;
          Serial.print("Took ");
          Serial.print(elapsedTime);
          Serial.print("s to parse ");
          float fileSize = file.size() / 1024.0;
          Serial.print(fileSize);
          Serial.print(" kB (average ");
          float parseSpeed = fileSize / elapsedTime;
          Serial.print(parseSpeed);
          Serial.println(" kB/sec)");
          return page;
        }
        char ch = (char)nextByte;
        #if defined(PROFILE_GET_MAX_PAGE)
        startSection(CRC32_UPDATE);
        #endif
        crc32.update((byte)nextByte);
        #if defined(PROFILE_GET_MAX_PAGE)
        endSection(CRC32_UPDATE);
        #endif
        if (ch == '\n') { // \n
          break;
        } else if (ch == '\r') {  // \r\n
          #if defined(PROFILE_GET_MAX_PAGE)
          startSection(FILE_READ_SLASH_N);
          #endif
          file.read();  // Advance forward one more because we still have \n to consume
          #if defined(PROFILE_GET_MAX_PAGE)
          endSection(FILE_READ_SLASH_N);
          #endif
          break;
        }
      }
    }
    page ++;
    if (page % 10 == 0) {
      #if defined(PROFILE_GET_MAX_PAGE)
      startSection(SERIAL_PRINT);
      #endif
      Serial.print("On page ");
      Serial.print(page);
      Serial.println(" and still counting");
      #if defined(PROFILE_GET_MAX_PAGE)
      endSection(SERIAL_PRINT);
      #endif
    }
  }
}
