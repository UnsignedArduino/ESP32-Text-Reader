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
                   unsigned long long startFill, unsigned long long endFill, unsigned long long maximum);

void drawTextReaderScrollbar(unsigned long long startFilePos, unsigned long long endFilePos, unsigned long long fileSize);

void drawDialog(const char* lines[], byte lineCount);

void drawTextReaderMenu(const char* filename, unsigned int page, unsigned int maxPage);

unsigned int askForPage(unsigned int curPg, unsigned int maxPg);

void noSdScreen();

void noTextFilesScreen();

void loadingTextFile(const char* filePath);

unsigned long long printFromLocation(unsigned int rows);

unsigned long long getPosFromPage(unsigned int page, unsigned int rows);

unsigned int getMaxPage(unsigned int rows, unsigned long& crc);
