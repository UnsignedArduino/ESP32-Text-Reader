#include <Arduino.h>
#include <SdFat.h>
#include <JPEGDEC.h>

#define SCREENSAVERS_PATH "/screensavers"
#define IMAGE_EXT ".jpg"

extern FsFile jpegFile;
extern JPEGDEC jpeg;

void* jpegOpen(const char* filename, int32_t *size);
void jpegClose(void* handle);
int32_t jpegRead(JPEGFILE* handle, uint8_t* buffer, int32_t length);
int32_t jpegSeek(JPEGFILE* handle, long position);
int jpegDraw(JPEGDRAW* draw);

bool drawScreensaver();
