#include <Arduino.h>
#include <SdFat.h>

#define MAX_PATH_LEN 255

#define SD_FAT_TYPE 3
#define SPI_CLOCK SD_SCK_MHZ(10)
#define SD_CONFIG SdSpiConfig(sdCS, SHARED_SPI, SPI_CLOCK)

// #define SPI_DRIVER_SELECT 1  // Use standard SPI library

extern SdFs sd;

int beginSD();

unsigned long filesInRoot();

bool seekRelative(FsFile f, long difference);

int readBackwards(File32 f);

int peekBefore(File32 f);

int peekAtRelative(File32 f, long difference);
