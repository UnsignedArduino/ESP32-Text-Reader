#include <Arduino.h>
#include <SD.h>

int beginSD();

unsigned int filesInRoot();

bool seekRelative(File f, long difference);

int readBackwards(File f);

int peekBefore(File f);

int peekAtRelative(File f, long difference);
