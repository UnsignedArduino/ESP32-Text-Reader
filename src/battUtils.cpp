#include <Arduino.h>
#include "pinouts.h"

int beginBattReader() {
  pinMode(transistorPin, OUTPUT);
  pinMode(battPin, INPUT);
  digitalWrite(transistorPin, LOW);
  return 0;
}

unsigned long readBatt() {
  digitalWrite(transistorPin, HIGH);
  delay(5);
  unsigned long millivolts = analogReadMilliVolts(battPin);
  digitalWrite(transistorPin, LOW);
  Serial.print("Got battery reading: ");
  Serial.print(millivolts);
  Serial.println(" millivolts");
  unsigned long adjustedMV = (millivolts + milliVoltDiff) * milliVoltScale;
  Serial.print("Adjusted battery reading: ");
  Serial.print(adjustedMV);
  Serial.println(" millivolts");
  return adjustedMV;
}

byte readPercent() {
  unsigned long mvolts = readBatt();
  const byte voltMapCount = 22;
  // https://arduino.stackexchange.com/q/60419/50973
  unsigned long voltMap[voltMapCount][2] = {
    {0,    0},
    {3270, 0},
    {3610, 5},
    {3690, 10},
    {3710, 15},
    {3730, 20},
    {3750, 25},
    {3770, 30},
    {3790, 35},
    {3800, 40},
    {3820, 45},
    {3840, 50},
    {3850, 55},
    {3870, 60},
    {3910, 65},
    {3950, 70},
    {3980, 75},
    {4020, 80},
    {4080, 85},
    {4110, 90},
    {4150, 95},
    {4200, 100}
  };
  for (byte i = 0; i < voltMapCount; i ++) {
    if (voltMap[i][0] >= mvolts) {
      Serial.print("Battery percentage: ");
      Serial.print(voltMap[i][1]);
      Serial.println("%");
      return voltMap[i][1];
    }
  }
  Serial.print("Battery percentage: 100% (ran out of values)");
  return 100;
}
