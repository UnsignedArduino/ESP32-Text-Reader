#include <Arduino.h>
#include <SD.h>
#include "utils.h"
#include "sdUtils.h"
#include "epaperUtils.h"
#include <GUI_Paint.h>
#include "buttonUtils.h"
#include "textReader.h"

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  working();

  Serial.begin(9600);
  Serial.println();

  if (beginSD() != 0) {
    blinkError(100, 400);
  }
  if (beginEPaper() != 0) {
    blinkError(250, 250);
  }

  beginButtons();

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
  nextFilePos = printFromLocation();

  notWorking();
}

void loop() {
  if (readRightButton()) {
    working();
    file.seek(nextFilePos);
    nextFilePos = printFromLocation();
    notWorking();
  }
  if (readLeftButton()) {
    working();
    file.seek(findLastPageLocation((maxLines * 2) + 1));
    nextFilePos = printFromLocation();
    notWorking();
  }
}
