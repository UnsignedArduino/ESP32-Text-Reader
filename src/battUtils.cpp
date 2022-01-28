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
