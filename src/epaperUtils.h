#include <Arduino.h>
#include "EPD.h"

const unsigned int bwImageSize = ((EPD_4IN2_WIDTH % 8 == 0) ? (EPD_4IN2_WIDTH / 8 ) : (EPD_4IN2_WIDTH / 8 + 1)) * EPD_4IN2_HEIGHT;
byte imageBuffer[bwImageSize] = {};

int beginEPaper();
