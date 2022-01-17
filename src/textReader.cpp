#include <Arduino.h>
#include <SD.h>
#include "sdUtils.h"
#include "epaperUtils.h"
#include <GUI_Paint.h>
#include "textReader.h"

File file;
unsigned long filePos = 0;
unsigned long nextFilePos = 0;

unsigned long fileSize = 0;
unsigned long bytesOnScreen = 0;

const byte maxCharPerLine = 42;
const byte maxLines = 31;

void drawScrollbar(unsigned long startFilePos, unsigned long endFilePos, unsigned long fileSize) {
  Serial.println("Drawing scrollbar");
  Paint_DrawRectangle(EPD_4IN2_HEIGHT - 3, 1,
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
  Paint_DrawRectangle(EPD_4IN2_HEIGHT - 2, startY,
                      EPD_4IN2_HEIGHT - 1, endY,
                      BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
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

  Serial.println("Showing contents");
  EPD_4IN2_Display(imageBuffer);

  return file.position();
}

unsigned long findLastPageLocation(unsigned int rows) {
  Serial.print("Back tracking from location ");
  Serial.println(file.position());
  for (byte row = 0; row < rows * 2 + 1; row ++) {
    for (byte i = 0; i < maxCharPerLine; i ++) {
      int nextByte = readBackwards(file);
      if (nextByte == -1) {
        Serial.print("Error reading or reached BOF!");
        return 0;
      }
      char ch = (char)nextByte;
      if (ch == '\n') { // \n
        if (peekBefore(file) == '\r') {
          readBackwards(file);  // Advance backward one more because we still have \n to consume
        }
        break;
      }
    }
  }
  Serial.print("New file position: ");
  Serial.println(file.position());
  return file.position();
}
