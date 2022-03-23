// .pio\libdeps\esp32doit-devkit-v1\SdFat\src\SdFatConfig.h
#define SD_FAT_TYPE 3
#define USE_FAT_FILE_FLAG_CONTIGUOUS 1
#define SPI_DRIVER_SELECT 1
#define USE_SPI_ARRAY_TRANSFER 1
#define USE_SEPARATE_FAT_CACHE 1
#define USE_EXFAT_BITMAP_CACHE 1
#define USE_MULTI_SECTOR_IO 1

#include <Arduino.h>
#include <SdFat.h>
#include "utils.h"
#include "sdUtils.h"
#include "bookmark.h"
#include "epaperUtils.h"
#include "screensaver.h"
#include <GUI_Paint.h>
#include "buttonUtils.h"
#include "battUtils.h"
#include "textReader.h"
#include "config.h"
#include "profiling.h"

unsigned long page = 0;
unsigned long maxPage = 0;

void textReader(const char* path) {
  working();

  Serial.print("Opening ");
  Serial.println(path);
  file = sd.open(path, O_READ);
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

  unsigned long crc;
  maxPage = getMaxPage(maxLines, crc);
  Serial.print("Max page: ");
  Serial.println(maxPage);

  Serial.print("CRC32: 0x");
  Serial.println(crc, HEX);

  Serial.println("First text print");
  filePos = 0;
  file.seek(filePos);
  nextFilePos = printFromLocation(maxLines);

  unsigned long lastPage = 0;
  
  if (getLastPage(path, fileSize, crc, lastPage) && lastPage > 0) {
    working();
    Serial.print("Found last page set to ");
    Serial.println(lastPage);
    const byte linesToStickUp = 7;
    const byte bufSize = maxCharPerLine + 1;
    char lastPageBuf[bufSize];
    snprintf(lastPageBuf, bufSize, "Your were last on page %lu.", lastPage + 1);
    const char* lines[linesToStickUp] = {
      lastPageBuf,
      "",
      "Do you want to go there?",
      "",
      "Left: No",
      "Middle: Cancel",
      "Right: Yes"
    };
    drawDialog(lines, linesToStickUp);
    updateDisplay();
    notWorking();
    byte pressed = waitForButtonPress();
    working();
    if (pressed == RIGHT_BUTTON) {
      Serial.println("Going to last page");
      filePos = getPosFromPage(lastPage, maxLines);
      page = lastPage;
    } else if (pressed == CENTER_BUTTON) {
      Serial.println("Cancel!");
      Serial.println("Closing file");
      file.close();
      notWorking();
      return;
    } else {
      Serial.println("Going to first page");
      filePos = 0;
      page = 0;
    }

    file.seek(filePos);
    nextFilePos = printFromLocation(maxLines);
  } else {
    page = 0;
  }

  working();

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
        setLastPage(path, fileSize, crc, page);
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

int fileSelector(char* pathBuf, byte maxPathLen) {
  working();
  unsigned long optionIdx = 1;
  unsigned long maxFileIdx = filesInRoot() - 1;
  unsigned long maxOptionIdx = maxFileIdx + 1;
  unsigned long optionSkip = 0;
  const unsigned long maxOptionPage = 24;
  FsFile root = sd.open("/");
  notWorking();
  while (true) {
    working();
    Paint_Clear(WHITE);
    Serial.print("Selected option: ");
    Serial.print(optionIdx);
    Serial.print("/");
    Serial.println(maxOptionIdx);
    if (optionIdx > maxOptionPage - 1) {
      optionSkip = (optionIdx / maxOptionPage) * maxOptionPage;
    } else {
      optionSkip = 0;
    }
    Serial.print("Skipping ");
    Serial.print(optionSkip);
    Serial.println(" options");
    const byte linesToStickUp = 6;
    char selectedPath[maxPathLen];
    root.rewindDirectory();
    Serial.println("Contents:");
    char line[maxCharPerLine + 1];
    const unsigned int maxRow = maxLines - linesToStickUp + optionSkip;
    for (unsigned int row = 0; row < maxRow; ) {
      unsigned int yPos = (row - optionSkip) * 13;
      memset(line, 0, maxCharPerLine + 1);
      if (row == 0) {
        if (row >= optionSkip) {
          if (row == optionIdx) {
            strncat(line, "> ", maxCharPerLine + 1);
          } else {
            strncat(line, "  ", maxCharPerLine + 1);
          }
          strncat(line, "Exit", maxPathLen);
          Paint_DrawLine(0, yPos + 16, EPD_4IN2_HEIGHT, yPos + 16, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
          yPos += 2;
        }
      } else {
        FsFile entry = root.openNextFile();
        if (!entry) {
          break;
        }
        if (entry.isDirectory()) {
          continue;
        }
        char path[maxPathLen];
        entry.getName(path, maxPathLen - 1);
        if (row == optionIdx) {
          strncat(line, "> ", maxCharPerLine + 1);
          strncpy(selectedPath, path, maxPathLen);
        } else {
          strncat(line, "  ", maxCharPerLine + 1);
        }
        if (2 + strlen(path) > maxCharPerLine) {
          strncat(line, path, maxCharPerLine - 3);
          strncat(line, "...", maxCharPerLine + 1);
        } else {
          strncat(line, path, maxCharPerLine + 1);
        }
        entry.close();
        yPos += 5;
      }
      if (row >= optionSkip) {  
        Serial.println(line);
        Paint_DrawString_EN(0, yPos, line, &Font12, WHITE, BLACK);
      }
      row ++;
    }
    drawScrollbar(EPD_4IN2_HEIGHT - 3, 1, 3, EPD_4IN2_WIDTH - (13 * linesToStickUp), 
                  optionSkip, min(optionSkip + maxOptionPage, maxOptionIdx), maxOptionIdx);
    const byte bufSize = maxCharPerLine + 1;
    char battBuf[bufSize];
    snprintf(battBuf, bufSize, "Battery: %u%%", readPercent());
    const char *lines[linesToStickUp] = {
      "Select an option",
      battBuf,
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
      root.close();
      if (optionIdx == 0) {
        Serial.println("Exiting!");
        return 1;
      } else {
        Serial.print("Selected path: ");
        Serial.println(selectedPath);
        strncpy(pathBuf, selectedPath, maxPathLen);
        return 0;
      }
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
        changeBy = 20;
      } else if (btnDiff > 6000) {
        changeBy = 10;
      } else if (btnDiff > 3000) {
        changeBy = 5;
      } else {
        changeBy = 1;
      }
      Serial.print("Change by ");
      Serial.println(changeBy);
      if (pressed == LEFT_BUTTON) {
        if (changeBy > optionIdx) {
          optionIdx = 0;
        } else {
          optionIdx -= changeBy;
        }
      } else {
        optionIdx = min(optionIdx + changeBy, maxOptionIdx);
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
  #if defined(PROFILING)
  Serial.println("Enabling profiling");
  prepareProfiling();
  Serial.println("Profiling enabled!");
  #endif
  #if defined(ENABLE_SCREENSAVERS)
  Serial.println("Screensavers are enabled!");
  #endif
  #if defined(SKIP_TO_SCREENSAVERS)
  Serial.println("Skipping directly to screensaver code after initalization");
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

  #if !defined(SKIP_TO_SCREENSAVERS)
  while (true) {
    // char* selected_path = "/text.txt";
    char selected_path[MAX_PATH_LEN];
    if (fileSelector(selected_path, MAX_PATH_LEN) == 1) {
      Serial.println("Exiting loop!");
      break;
    }
    Serial.print("Path selected: ");
    Serial.println(selected_path);
    textReader(selected_path);
  }
  #else
  Serial.println("Skipping text reader code, going to screensaver!");
  #endif

  working();
  Paint_Clear(WHITE);
  #if defined(ENABLE_SCREENSAVERS)
  if (drawScreensaver()) {
    Serial.println("Successfully drawn random screensaver!");
  } else {
    Serial.println("Failed to draw random screensaver!");
    Paint_Clear(WHITE);
  }
  if (anyButtonPressed()) {
    Serial.println("Skipping screensaver!");
    Paint_Clear(WHITE);
  }
  #endif
  updateDisplay();
  EPD_4IN2_Sleep();
  notWorking();
}

void loop() {
  
}
