#include <Arduino.h>
#include <SdFat.h>

#define MAX_PATH_LEN 255

#define SPI_CLOCK SD_SCK_MHZ(10)
#define SD_CONFIG SdSpiConfig(sdCS, SHARED_SPI, SPI_CLOCK)

extern SdFs sd;

int beginSD();

bool fileAtIndex(const char* path, const char* ext, unsigned long index, char* result, byte size);

unsigned long filesInDir(const char* path, const char* ext);

unsigned long filesInDir(const char* path);

unsigned long filesInRoot();

bool pickRandomFilename(const char* path, const char* ext, char* result, byte size);

bool seekRelative(FsFile f, long difference);

int readBackwards(File32 f);

int peekBefore(File32 f);

int peekAtRelative(File32 f, long difference);
