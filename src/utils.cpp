#include <Arduino.h>

void blinkError(unsigned int onTime, unsigned int offTime) {
  Serial.print("Fatal error! (Code: "); 
  Serial.print(onTime);
  Serial.print(", ");
  Serial.print(offTime);
  Serial.println(")");
  while (true) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(onTime);
    digitalWrite(LED_BUILTIN, LOW);
    delay(offTime);
  }
}
