#include <Arduino.h>
#include "pinouts.h"

int beginButtons() {
  pinMode(leftButtonPin, INPUT_PULLUP);
  pinMode(centerButtonPin, INPUT_PULLUP);
  pinMode(rightButtonPin, INPUT_PULLUP);
  return 0;
}
