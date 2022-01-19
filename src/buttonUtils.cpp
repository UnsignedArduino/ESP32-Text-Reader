#include <Arduino.h>
#include "pinouts.h"
#include "buttonUtils.h"

int beginButtons() {
  pinMode(leftButtonPin, INPUT_PULLUP);
  pinMode(centerButtonPin, INPUT_PULLUP);
  pinMode(rightButtonPin, INPUT_PULLUP);
  return 0;
}

byte waitForButtonPress() {
  Serial.println("Waiting for button press...");
  while (true) {
    if (readLeftButton()) {
      return LEFT_BUTTON;
    } else if (readCenterButton()) {
      return CENTER_BUTTON;
    } else if (readRightButton()) {
      return RIGHT_BUTTON;
    }
  }
}

void waitForButtonRelease() {
  Serial.println("Waiting for button release...");
  while (readLeftButton() || readCenterButton() || readRightButton()) {
    ;
  }
}

