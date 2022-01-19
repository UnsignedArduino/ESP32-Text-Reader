#include <Arduino.h>
#include <SD.h>
#include "utils.h"
#include "sdUtils.h"
#include "epaperUtils.h"
#include <GUI_Paint.h>
#include "buttonUtils.h"
#include "textReader.h"
#include "config.h"

unsigned long page = 0;

void noSdScreen() {
  Paint_Clear(WHITE);
  Paint_DrawString_EN(0, 0, "No SD card is inserted!", &Font12, WHITE, BLACK);
  Paint_DrawString_EN(0, 26, "Please insert an SD card and press any ", &Font12, WHITE, BLACK);
  Paint_DrawString_EN(0, 39, "button to try again!", &Font12, WHITE, BLACK);
  EPD_4IN2_Display(imageBuffer);
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

  if (beginEPaper() != 0) {
    blinkError(250, 250);
  }

  while (beginSD() != 0) {
    Serial.println("No SD card detected!");
    noSdScreen();
    Serial.println("Waiting for button press to try again");
    notWorking();
    while (!readLeftButton() && !readCenterButton() && !readRightButton()) {
      ;
    }
    working();
    Serial.println("Trying again");
  }

  Serial.println("Opening /text.txt");
  file = SD.open("/text.txt", FILE_READ);
  if (!file) {
    Serial.println("Unable to open /text.txt");
    blinkError(500, 1000);
  }
  
  fileSize = file.size();
  Serial.print("File size: ");
  Serial.print(fileSize);
  Serial.println(" bytes");

  filePos = 0;
  file.seek(filePos);
  nextFilePos = printFromLocation(maxLines);

  notWorking();
}

void loop() {
  #if defined(SET_PAGE_VIA_SERIAL)
  if (Serial.available()) {
    working();
    // Flush receive buffer
    while(Serial.available()) {
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
    file.seek(getPosFromPage(page, maxLines));
    nextFilePos = printFromLocation(maxLines);
    notWorking();
  }
  #endif
  if (readRightButton()) {
    working();
    page ++;
    Serial.print("Going to page ");
    Serial.println(page);
    // We can use where we left off
    file.seek(nextFilePos);
    nextFilePos = printFromLocation(maxLines);
    notWorking();
  }
  if (readLeftButton() && page > 0) {
    working();
    page --;
    Serial.print("Going to page ");
    Serial.println(page);
    // Have to seek back to beginning and count pages to here
    file.seek(getPosFromPage(page, maxLines));
    nextFilePos = printFromLocation(maxLines);
    notWorking();
  }
}
