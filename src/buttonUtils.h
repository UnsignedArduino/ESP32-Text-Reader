#include <Arduino.h>
#include "pinouts.h"

#define readLeftButton() !digitalRead(leftButtonPin)
#define readCenterButton() !digitalRead(centerButtonPin)
#define readRightButton() !digitalRead(rightButtonPin)

int beginButtons();
