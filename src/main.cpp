#include <Arduino.h>
#include <SD.h>
#include "utils.h"
#include "sdUtils.h"
#include "epaperUtils.h"
#include <GUI_Paint.h>
#include "buttonUtils.h"

File file;
unsigned long filePos = 0;

const byte maxCharPerLine = 42;
const byte maxLines = 30;

void printFromLocation() {
  Paint_Clear(WHITE);

  Serial.print("Reading from location ");
  Serial.println(file.position());

  Serial.println("Loading from file");
  Serial.println("Got: ");
  for (unsigned int yPos = 0; yPos < EPD_4IN2_WIDTH; yPos += 13) {
    char line[maxCharPerLine + 1] = "";
    byte i = 0;
    for ( ; i < maxCharPerLine; i ++) {
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
      } else {
        line[i] = ch;
      }
    }
    line[i] = '\0';
    Serial.println(line);

    Paint_DrawString_EN(0, yPos, line, &Font12, WHITE, BLACK);
  }

  Serial.println("Showing contents");
  EPD_4IN2_Display(imageBuffer);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  Serial.println();

  if (beginSD() != 0) {
    blinkError(100, 400);
  }
  if (beginEPaper() != 0) {
    blinkError(250, 250);
  }

  beginButtons();

  Serial.println("Opening /bible.txt");
  file = SD.open("/bible.txt", FILE_READ);
  if (!file) {
    Serial.println("Unable to open /bible.txt");
    blinkError(500, 1000);
  }
  
  filePos = 0;
  file.seek(filePos);
  printFromLocation();
  file.close();
}

void loop() {
  
}
