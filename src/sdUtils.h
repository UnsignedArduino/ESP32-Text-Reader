#include <Arduino.h>
#include <SD.h>

int beginSD();

bool seekRelative(File f, long difference);

int peekBefore(File f);

int peekAtRelative(File f, long difference);
