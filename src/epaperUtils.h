#include <Arduino.h>
#include <EPD.h>

extern byte imageBuffer[];

int beginEPaper();

void updateDisplay();

void updateDisplayPartial(unsigned int startx, unsigned int starty, unsigned int endx, unsigned int endy);
