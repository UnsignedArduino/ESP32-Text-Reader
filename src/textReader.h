#include <Arduino.h>

extern File file;
extern unsigned long filePos;
extern unsigned long nextFilePos;

extern unsigned long fileSize;
extern unsigned long bytesOnScreen;

extern const byte maxCharPerLine;
extern const byte maxLines;

void drawScrollbar(unsigned long startFilePos, unsigned long endFilePos, unsigned long fileSize);

void drawTextReaderMenu(unsigned int page);

unsigned long printFromLocation(unsigned int rows);

void updateDisplay();

unsigned long getPosFromPage(unsigned int page, unsigned int rows);
