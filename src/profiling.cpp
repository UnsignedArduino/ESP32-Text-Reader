#include <Arduino.h>
#include "profiling.h"

#if defined(PROFILING)

hw_timer_t* timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

volatile bool sectionStates[MAX_SECTION_COUNT];
volatile unsigned long sectionHits[MAX_SECTION_COUNT];
volatile unsigned long missedHits = 0;
volatile unsigned long long totalHits = 0;

unsigned long startProfileTime = 0;
unsigned long endProfileTime = 0;

#if defined(PROFILE_GET_MAX_PAGE)

const char* sectionNames[MAX_SECTION_COUNT] = {
  "FILE_READ_NEXTBYTE",
  "CRC32_UPDATE",
  "FILE_READ_SLASH_N",
  "SERIAL_PRINT"
};

#endif

void IRAM_ATTR profile() {
  bool didHit = false;
  portENTER_CRITICAL_ISR(&timerMux);
  for (int i = 0; i < MAX_SECTION_COUNT; i ++) {
    if (sectionStates[i]) {
      sectionHits[i] ++;
      totalHits ++;
      didHit = true;
    }
  }
  if (!didHit) {
    missedHits ++;
    totalHits ++;
  }
  portEXIT_CRITICAL_ISR(&timerMux);
}

void prepareProfiling() {
  timer = timerBegin(0, PROFILER_PRESCALER, true);
  timerAttachInterrupt(timer, &profile, true);
  timerAlarmWrite(timer, PROFILER_PERIOD, true);
}

void startProfiling() {
  for (int i = 0; i < MAX_SECTION_COUNT; i ++) {
    sectionHits[i] = 0;
    sectionStates[i] = false;
  }
  missedHits = 0;
  totalHits = 0;
  startProfileTime = millis();
  timerAlarmEnable(timer);
  Serial.print("Start profiling");
  Serial.print(" (period: ");
  Serial.print(PROFILER_PERIOD);
  Serial.println(" microseconds)");
}

void endProfiling() {
  timerAlarmDisable(timer);
  endProfileTime = millis();
  unsigned long elapsedProfileTime = endProfileTime - startProfileTime;
  Serial.println("End profiling");
  Serial.print("Profiler period: ");
  Serial.print(PROFILER_PERIOD);
  Serial.println(" microseconds");
  Serial.print("Profile time: ");
  Serial.print(elapsedProfileTime);
  Serial.println(" milliseconds");
  Serial.print("Total section hits: ");
  Serial.println(totalHits);
  Serial.println("Section hit distribution: ");
  Serial.print(" -1 UNPROFILED ");
  Serial.print(missedHits);
  Serial.print(" (");
  float sectionPercentage = (float)missedHits / (float)totalHits;
  Serial.print(sectionPercentage * (float)elapsedProfileTime);
  Serial.print(" milliseconds - ");
  Serial.print(sectionPercentage * 100.0);
  Serial.println("%)");
  for (byte i = 0; i < SECTION_COUNT; i ++) {
    Serial.print("  ");
    Serial.print(i);
    Serial.print(" ");
    Serial.print(sectionNames[i]);
    Serial.print(" ");
    Serial.print(sectionHits[i]);
    Serial.print(" (");
    float sectionPercentage = (float)sectionHits[i] / (float)totalHits;
    Serial.print(sectionPercentage * (float)elapsedProfileTime);
    Serial.print(" milliseconds - ");
    Serial.print(sectionPercentage * 100.0);
    Serial.println("%)");
  }
}

#endif
