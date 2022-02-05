#include <Arduino.h>
#include <SdFat.h>

extern FsFile file;
extern unsigned long filePos;
extern unsigned long nextFilePos;

extern unsigned long fileSize;
extern unsigned long bytesOnScreen;

extern const byte maxCharPerLine;
extern const byte maxLines;

void drawScrollbar(unsigned long startFilePos, unsigned long endFilePos, unsigned long fileSize);

void drawDialog(const char* lines[], byte lineCount);

void drawTextReaderMenu(const char* filename, unsigned int page, unsigned int maxPage);

unsigned int askForPage(unsigned int curPg, unsigned int maxPg);

void noSdScreen();

void noTextFilesScreen();

void loadingTextFile(const char* filePath);

unsigned long printFromLocation(unsigned int rows);

unsigned long getPosFromPage(unsigned int page, unsigned int rows);

unsigned int getMaxPage(unsigned int rows);
