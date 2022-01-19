#include <Arduino.h>

extern File file;
extern unsigned long filePos;
extern unsigned long nextFilePos;

extern unsigned long fileSize;
extern unsigned long bytesOnScreen;

extern const byte maxCharPerLine;
extern const byte maxLines;

void drawScrollbar(unsigned long startFilePos, unsigned long endFilePos, unsigned long fileSize);

unsigned long printFromLocation(unsigned int rows);

unsigned long getPosFromPage(unsigned long page, unsigned int rows);
