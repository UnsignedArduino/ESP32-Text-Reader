#include <Arduino.h>
#include <SD.h>
#include "utils.h"
#include "sdUtils.h"
#include "buttonUtils.h"
#include "epaperUtils.h"
#include <GUI_Paint.h>
#include "textReader.h"

File file;
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

void drawTextReaderMenu(const char *filename, unsigned int page, unsigned int maxPage) {
  const byte linesToStickUp = 6;
  Serial.println("Drawing text reader menu");
  Paint_DrawRectangle(0, EPD_4IN2_WIDTH - (13 * linesToStickUp) - 2,
                      EPD_4IN2_HEIGHT, EPD_4IN2_WIDTH,
                      WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
  Paint_DrawRectangle(1, EPD_4IN2_WIDTH - (13 * linesToStickUp) - 2,
                      EPD_4IN2_HEIGHT, EPD_4IN2_WIDTH,
                      BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
  const unsigned int bufSize = maxCharPerLine + 1;
  char buf[bufSize];
  snprintf(buf, bufSize, "File: %s", filename);
  Paint_DrawString_EN(2, EPD_4IN2_WIDTH - (13 * linesToStickUp), 
                      buf, 
                      &Font12, WHITE, BLACK);
  snprintf(buf, bufSize, "Page: %u / %u", page, maxPage);
  Paint_DrawString_EN(2, EPD_4IN2_WIDTH - (13 * (linesToStickUp - 1)), 
                      buf, 
                      &Font12, WHITE, BLACK);
  Paint_DrawString_EN(2, EPD_4IN2_WIDTH - (13 * (linesToStickUp - 3)), 
                      "Left: Exit", 
                      &Font12, WHITE, BLACK);
  Paint_DrawString_EN(2, EPD_4IN2_WIDTH - (13 * (linesToStickUp - 4)), 
                      "Middle: Cancel", 
                      &Font12, WHITE, BLACK);
  Paint_DrawString_EN(2, EPD_4IN2_WIDTH - (13 * (linesToStickUp - 5)), 
                      "Right: Go to page", 
                      &Font12, WHITE, BLACK);
}

unsigned int askForPage(unsigned int curPg, unsigned int maxPg) {
  working();
  unsigned int value = curPg;
  Serial.println("Drawing page selector menu");
  notWorking();
  bool update = true;
  unsigned int changeBy = 1;
  while (true) {
    if (update) {
      Paint_Clear(WHITE);
      // Seek back to beginning of page and reprint it
      file.seek(filePos);
      nextFilePos = printFromLocation(maxLines);
      const byte linesToStickUp = 9;
      Paint_DrawRectangle(0, EPD_4IN2_WIDTH - (13 * linesToStickUp) - 2,
                      EPD_4IN2_HEIGHT, EPD_4IN2_WIDTH,
                      WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
      Paint_DrawRectangle(1, EPD_4IN2_WIDTH - (13 * linesToStickUp) - 2,
                          EPD_4IN2_HEIGHT, EPD_4IN2_WIDTH,
                          BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
      const unsigned int bufSize = maxCharPerLine + 1;
      char buf[bufSize];
      Paint_DrawString_EN(2, EPD_4IN2_WIDTH - (13 * linesToStickUp), 
                          "What page would you like to go to?", 
                          &Font12, WHITE, BLACK);
      snprintf(buf, bufSize, "Current page: %u / %u", curPg + 1, maxPg + 1);
      Paint_DrawString_EN(2, EPD_4IN2_WIDTH - (13 * (linesToStickUp - 2)), 
                          buf, &Font12, WHITE, BLACK);
      snprintf(buf, bufSize, "Go to page: %u", value + 1);
      Paint_DrawString_EN(2, EPD_4IN2_WIDTH - (13 * (linesToStickUp - 4)), 
                          buf, &Font12, WHITE, BLACK);
      Paint_DrawString_EN(2, EPD_4IN2_WIDTH - (13 * (linesToStickUp - 6)), 
                          "Left: Decrement", &Font12, WHITE, BLACK);
      Paint_DrawString_EN(2, EPD_4IN2_WIDTH - (13 * (linesToStickUp - 7)), 
                          "Middle: Enter", &Font12, WHITE, BLACK);
      Paint_DrawString_EN(2, EPD_4IN2_WIDTH - (13 * (linesToStickUp - 8)), 
                          "Right: Increment", &Font12, WHITE, BLACK);
      updateDisplay();
      update = false;
    }
    notWorking();
    byte pressed = waitForButtonPress();
    unsigned long startBtnPress = millis();
    while (anyButtonPressed()) {
      unsigned long blinkDelay;
      if (millis() - startBtnPress > 5000) {
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
    working();
    unsigned long endBtnPress = millis();
    unsigned long btnDiff = endBtnPress - startBtnPress;
    if (btnDiff > 5000) {
      Serial.println("Change by 25!");
      changeBy = 50;
    } else if (btnDiff > 3000) {
      Serial.println("Change by 10!");
      changeBy = 10;
    } else {
      Serial.println("Change by 1!");
      changeBy = 1;
    }
    if (pressed == LEFT_BUTTON) {
      if (value >= /*0 +*/ changeBy) {
        value -= changeBy;
        update = true;
      }
    } else if (pressed == RIGHT_BUTTON) {
      if (value <= maxPg - changeBy) {
        value += changeBy;
        update = true;
      }
    } else if (pressed == CENTER_BUTTON) {
      break;
    }
  }
  working();
  Serial.print("User selected page: ");
  Serial.println(value);
  return value;
}

void noSdScreen() {
  Paint_Clear(WHITE);
  Paint_DrawString_EN(0, 0, "No SD card is inserted!", &Font12, WHITE, BLACK);
  Paint_DrawString_EN(0, 26, "Please insert an SD card and press any ", &Font12, WHITE, BLACK);
  Paint_DrawString_EN(0, 39, "button to try again!", &Font12, WHITE, BLACK);
  EPD_4IN2_Display(imageBuffer);
}

void loadingTextFile(const char* filePath) {
  Paint_Clear(WHITE);
  const unsigned int bufSize = maxCharPerLine + 1;
  char buf[bufSize];
  snprintf(buf, bufSize, "Loading text file: %s", filePath);
  Paint_DrawString_EN(0, 0, buf, &Font12, WHITE, BLACK);
  EPD_4IN2_Display(imageBuffer);
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
