#include <Arduino.h>
#include <SdFat.h>

extern FsFile file;
extern unsigned long long filePos;
extern unsigned long long nextFilePos;

extern unsigned long long fileSize;
extern unsigned long bytesOnScreen;

extern const byte maxCharPerLine;
extern const byte maxLines;

void drawScrollbar(unsigned int startX, unsigned int startY, 
                   unsigned int width, unsigned int height,
                   unsigned long startFill, unsigned long endFill, unsigned long maximum);

void drawTextReaderScrollbar(unsigned long long startFilePos, unsigned long long endFilePos, unsigned long long fileSize);

void drawDialog(const char* lines[], byte lineCount);

void drawTextReaderMenu(const char* filename, unsigned long page, unsigned long maxPage);

unsigned long askForPage(unsigned long curPg, unsigned long maxPg);

void noSdScreen();

void noTextFilesScreen();

void loadingTextFile(const char* filePath);

unsigned long long printFromLocation(unsigned int rows);

unsigned long long getPosFromPage(unsigned long page, unsigned int rows);

unsigned long getMaxPage(unsigned int rows, unsigned long& crc);
