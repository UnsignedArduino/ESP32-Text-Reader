#if !defined(INCLUDED_PROFILING_H)
#define INCLUDED_PROFILING_H

#include "config.h"

// Defined when anything is profiled
#if defined(PROFILE_GET_MAX_PAGE)

#define PROFILING

#define MAX_SECTION_COUNT 4
// 80MHZ / 80 prescaler = 1MHZ
// For PROFILER_PERIOD to be in microseconds use a prescaler of 80
#define PROFILER_PRESCALER 80
#define PROFILER_PERIOD 250 // microseconds

const float profilerSpeed = 80000000.0 / PROFILER_PRESCALER;

extern hw_timer_t* timer;
extern portMUX_TYPE timerMux;

extern volatile bool sectionStates[MAX_SECTION_COUNT];
extern volatile unsigned long sectionHits[MAX_SECTION_COUNT];
extern volatile unsigned long missedHits;
extern volatile unsigned long long totalHits;

extern unsigned long startProfileTime;
extern unsigned long endProfileTime;

void IRAM_ATTR profile();

void prepareProfiling();
void startProfiling();

#define startSection(index) portENTER_CRITICAL(&timerMux); sectionStates[index] = true; portEXIT_CRITICAL(&timerMux)
#define endSection(index) portENTER_CRITICAL(&timerMux); sectionStates[index] = false; portEXIT_CRITICAL(&timerMux);

void endProfiling();

#endif

#if defined(PROFILE_GET_MAX_PAGE)

extern const char* sectionNames[MAX_SECTION_COUNT];

#define SECTION_COUNT 4

#define FILE_READ_NEXTBYTE 0
#define CRC32_UPDATE 1
#define FILE_READ_SLASH_N 2
#define SERIAL_PRINT 3

#endif

#endif
