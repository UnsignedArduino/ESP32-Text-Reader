#include <Arduino.h>
#include <SdFat.h>
#include "utils.h"
#include "sdUtils.h"
#include "epaperUtils.h"
#include <GUI_Paint.h>
#include "buttonUtils.h"
#include "battUtils.h"
#include "textReader.h"
#include "config.h"

unsigned int page = 0;
unsigned int maxPage = 0;

const byte maxPathLen = 255;

void textReader(const char* path) {
  working();

  Serial.print("Opening ");
  Serial.println(path);
  file = sd.open(path, FILE_READ);
  if (!file) {
    Serial.print("Unable to open ");
    Serial.println(path);
    blinkError(500, 1000);
  }
  
  fileSize = file.size();
  Serial.print("File size: ");
  Serial.print(fileSize);
  Serial.println(" bytes");

  Serial.println("Finding max page");
  loadingTextFile(path);

  maxPage = getMaxPage(maxLines);
  Serial.print("Max page: ");
  Serial.println(maxPage);

  Serial.println("First text print");
  filePos = 0;
  file.seek(filePos);
  nextFilePos = printFromLocation(maxLines);
  updateDisplay();

  notWorking();

  while (true) {
    #if defined(SET_PAGE_VIA_SERIAL)
    if (Serial.available()) {
      working();
      // Flush receive buffer
      while (Serial.available()) {
        Serial.read();
      }
      Serial.print("Current page is ");
      Serial.println(page);
      Serial.println("Type the page number you want to go to: ");
      delay(1000);
      Serial.println("\nParsing number");
      page = Serial.parseInt();
      Serial.print("Got: ");
      Serial.println(page);
      Serial.print("Going to page ");
      Serial.println(page);
      filePos = getPosFromPage(page, maxLines);
      file.seek(filePos);
      nextFilePos = printFromLocation(maxLines);
      updateDisplay();
      notWorking();
    }
    #endif
    if (readLeftButton() && page > 0) {
      working();
      page --;
      Serial.print("Going to page ");
      Serial.println(page);
      // Have to seek back to beginning and count pages to here
      filePos = getPosFromPage(page, maxLines);
      file.seek(filePos);
      nextFilePos = printFromLocation(maxLines);
      updateDisplay();
      notWorking();
    }
    if (readCenterButton()) {
      working();
      // Seek back to beginning of page and reprint it
      file.seek(filePos);
      nextFilePos = printFromLocation(maxLines);
      // Page is zero indexed
      drawTextReaderMenu(path, page + 1, maxPage + 1);
      updateDisplay();
      Serial.println("Waiting for button press");
      notWorking();
      byte pressed = waitForButtonPress();
      if (pressed == LEFT_BUTTON) {
        Serial.println("Left button pressed, exiting!");
        break;
      } else if (pressed == RIGHT_BUTTON) {
        working();
        Serial.println("Selecting page");
        page = askForPage(page, maxPage);
        Serial.print("Got page: ");
        Serial.println(page);
        filePos = getPosFromPage(page, maxLines);
        notWorking();
      }
      working();
      file.seek(filePos);
      nextFilePos = printFromLocation(maxLines);
      updateDisplay();
      notWorking();
      waitForButtonRelease();
    }
    if (readRightButton() && page < maxPage) {
      working();
      page ++;
      Serial.print("Going to page ");
      Serial.println(page);
      // We can use where we left off
      filePos = nextFilePos;
      file.seek(nextFilePos);
      nextFilePos = printFromLocation(maxLines);
      updateDisplay();
      notWorking();
    }
  }

  working();

  Serial.println("Closing file");
  file.close();

  notWorking();
}

void fileSelector(char* pathBuf, byte maxPathLen) {
  working();
  unsigned int fileIdx = 0;
  unsigned int maxFileIdx = filesInRoot() - 1;
  FsFile root = sd.open("/");
  notWorking();
  while (true) {
    working();
    Paint_Clear(WHITE);
    Serial.print("Selected file: ");
    Serial.print(fileIdx);
    Serial.print("/");
    Serial.println(maxFileIdx);
    const byte linesToStickUp = 5;
    char selectedPath[maxPathLen];
    root.rewindDirectory();
    Serial.println("Contents:");
    for (unsigned int row = 0; row < maxLines - maxLines - 2; ) {
      unsigned int yPos = row * 13;
      char line[maxCharPerLine + 1];
      FsFile entry = root.openNextFile();
      if (!entry) {
        break;
      }
      if (entry.isDirectory()) {
        continue;
      }
      char path[maxPathLen];
      entry.getName(path, maxPathLen);
      if (row == fileIdx) {
        snprintf(line, maxCharPerLine + 1, "> %s", path);
        strncpy(selectedPath, path, maxPathLen);
      } else {
        snprintf(line, maxCharPerLine + 1, "  %s", path);
      }
      entry.close();
      Serial.println(line);
      Paint_DrawString_EN(0, yPos, line, &Font12, WHITE, BLACK);
      row ++;
    }
    const char *lines[linesToStickUp] = {
      "Select a text file",
      "",
      "Left: Move up",
      "Center: Select",
      "Right: Move down"
    };
    drawDialog(lines, linesToStickUp);
    updateDisplay();
    notWorking();
    byte pressed = waitForButtonPress();
    if (pressed == CENTER_BUTTON) {
      Serial.print("Selected path: ");
      Serial.println(selectedPath);
      strncpy(pathBuf, selectedPath, maxPathLen);
      root.close();
      return;
    } else {
      unsigned long startBtnPress = millis();
      while (anyButtonPressed()) {
        unsigned long blinkDelay;
        if (millis() - startBtnPress > 6000) {
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
      if (btnDiff > 6000) {
        changeBy = 10;
      } else if (btnDiff > 3000) {
        changeBy = 5;
      } else {
        changeBy = 1;
      }
      Serial.print("Change by ");
      Serial.println(changeBy);
      if (pressed == LEFT_BUTTON) {
        if (changeBy > fileIdx) {
          fileIdx = 0;
        } else {
          fileIdx -= changeBy;
        }
      } else {
        fileIdx = min(fileIdx + changeBy, maxFileIdx);
      }
    }
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  working();

  Serial.begin(9600);
  Serial.println();
  #if defined(SET_PAGE_VIA_SERIAL)
  Serial.println("Setting page via serial is enabled!");
  #endif

  beginButtons();

  beginBattReader();
  readBatt();

  if (beginEPaper() != 0) {
    blinkError(250, 250);
  }

  while (beginSD() != 0) {
    Serial.println("No SD card detected!");
    noSdScreen();
    Serial.println("Waiting for button press to try again");
    notWorking();
    waitForButtonPress();
    working();
    Serial.println("Trying again");
  }

  if (filesInRoot() == 0) {
    Serial.println("No files in root directory!");
    noTextFilesScreen();
    notWorking();
    Serial.println("Infinite looping");
    while (true) {
      ;
    }
  }

  notWorking();

  while (true) {
    // char* selected_path = "/text.txt";
    char selected_path[maxPathLen];
    fileSelector(selected_path, maxPathLen);
    Serial.print("Path selected: ");
    Serial.println(selected_path);
    page = 0;
    textReader(selected_path);
  }

  working();
  Paint_Clear(WHITE);
  updateDisplay();
  EPD_4IN2_Sleep();
  notWorking();
}

void loop() {
  
}
