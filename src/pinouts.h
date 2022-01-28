#if !defined(INCLUDED_PINOUTS_H)
#define INCLUDED_PINOUTS_H

#include <Arduino.h>

// MicroSD card connections

const byte sdCS = 5;

// Button connections

const byte leftButtonPin = 32;
const byte centerButtonPin = 33;
const byte rightButtonPin = 25;

// Battery reading connections

const byte transistorPin = 27;
const byte battPin = 26;

const unsigned long milliVoltScale = 2;
const unsigned long milliVoltDiff = 600;

#endif