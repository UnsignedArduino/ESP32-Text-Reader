#include <Arduino.h>
#include <SdFat.h>
#include "utils.h"
#include "sdUtils.h"
#include "buttonUtils.h"
#include "battUtils.h"
#include "epaperUtils.h"
#include <GUI_Paint.h>
#include "textReader.h"

FsFile file;
unsigned long filePos = 0;
unsigned long nextFilePos = 0;

unsigned long fileSize = 0;
unsigned long bytesOnScreen = 0;

const byte maxCharPerLine = 40;
const byte maxLines = 30;

void drawScrollbar(unsigned long startFilePos, unsigned long endFilePos, unsigned long fileSize) {
  Serial.println("Drawing scrollbar");
  Paint_DrawRectangle(EPD_4IN2_HEIGHT - 4, 1,
                      EPD_4IN2_HEIGHT - 1, EPD_4IN2_WIDTH - 1, 
                      BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
  unsigned int startY, endY;
  if (fileSize == 0) {
    startY = 2;
    endY = EPD_4IN2_WIDTH - 1;
  } else {
    startY = map(startFilePos, 0, fileSize, 2, EPD_4IN2_WIDTH - 2);
    endY = map(endFilePos, 0, fileSize, 2, EPD_4IN2_WIDTH - 2) + 1;
  }
  Serial.print("Drawing from "); 
  Serial.print(startY);
  Serial.print(" to ");
  Serial.println(endY);
  Paint_DrawRectangle(EPD_4IN2_HEIGHT - 3, startY,
                      EPD_4IN2_HEIGHT - 1, endY,
                      BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
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

void drawTextReaderMenu(const char *filename, unsigned int page, unsigned int maxPage) {
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
  snprintf(pageBuf, bufSize, "Page: %u / %u", page, maxPage);
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

unsigned int askForPage(unsigned int curPg, unsigned int maxPg) {
  working();
  unsigned int value = curPg;
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
      snprintf(curPageBuf, bufSize, "Current page: %u / %u", curPg + 1, maxPg + 1);
      char goPageBuf[bufSize];
      snprintf(goPageBuf, bufSize, "Go to page: %u", value + 1);
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
  const byte linesToStickUp = 3;
  const char *lines[linesToStickUp] = {
    "No SD card is inserted!",
    "Please insert an SD card and press",
    "any button to try again!"
  };
  Paint_Clear(WHITE);
  drawDialog(lines, linesToStickUp);
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

unsigned long printFromLocation(unsigned int rows) {
  Paint_Clear(WHITE);

  unsigned long startFilePos = file.position();

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

  unsigned long endFilePos = file.position();

  Serial.print("Ended at position: "); 
  Serial.println(endFilePos);

  Serial.print("Bytes on screen: ");
  Serial.println(bytesOnScreen);

  drawScrollbar(startFilePos, endFilePos, fileSize);

  return file.position();
}

unsigned long getPosFromPage(unsigned int page, unsigned int rows) {
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

  unsigned long endFilePos = file.position();

  Serial.print("Ended at position: "); 
  Serial.println(endFilePos);

  return endFilePos;
}

unsigned int getMaxPage(unsigned int rows) {
  Serial.println("Finding max page");
  unsigned int page = 0;
  file.seek(0);
  while (true) {
    for (unsigned int row = 0; row < rows; row ++) {
      for (byte i = 0; i < maxCharPerLine; i ++) {
        int nextByte = file.read();
        if (nextByte == -1) {
          Serial.print("Last page is ");
          Serial.println(page);
          return page;
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
    page ++;
    if (page % 10 == 0) {
      Serial.print("On page ");
      Serial.print(page);
      Serial.println(" and still counting");
    }
  }
}
