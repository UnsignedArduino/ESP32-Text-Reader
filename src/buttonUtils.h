#include <Arduino.h>
#include "pinouts.h"

#define LEFT_BUTTON 0
#define CENTER_BUTTON 1
#define RIGHT_BUTTON 2

#define readLeftButton() !digitalRead(leftButtonPin)
#define readCenterButton() !digitalRead(centerButtonPin)
#define readRightButton() !digitalRead(rightButtonPin)

int beginButtons();

byte waitForButtonPress();

void waitForButtonRelease();
