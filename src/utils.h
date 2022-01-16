#include <Arduino.h>

#define working() digitalWrite(LED_BUILTIN, HIGH)
#define notWorking() digitalWrite(LED_BUILTIN, LOW)

void blinkError(unsigned int onTime, unsigned int offTime);
