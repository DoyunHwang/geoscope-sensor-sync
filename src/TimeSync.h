// TimeSync.h

#ifndef _TIMESYNC_H
#define _TIMESYNC_H

#include "main.h"
#include <time.h>      // configTime(), time()
#include <sys/time.h>  // gettimeofday(), timeval

extern volatile bool timeSynced;
extern volatile int64_t utc_mac_offset;  // add to MAC timer to get UTC microseconds
extern volatile uint32_t last_sync_ms;

extern uint32_t sntp_poll_s;             // offset recapture interval, not SNTP stack poll interval
extern char ts_server[CHAR_BUF_SIZE];

void timeSyncSetup();
void timeSyncLoop();
bool timeSyncNow();  // force offset re-capture; returns false if SNTP not ready

// Sets how often this module re-captures the UTC-vs-MAC offset.
// This does NOT change the ESP8266 SNTP stack's own NTP polling interval.
void timeSyncSetRecaptureInterval(uint32_t seconds);

// Backward-compatible alias.
// Prefer timeSyncSetRecaptureInterval() in new code.
void timeSyncSetPollInterval(uint32_t seconds);

int64_t getEpochUs();   // current UTC in microseconds, -1 if not synced
uint32_t getSyncAge();  // ms since last successful sync, UINT32_MAX if never

void timeSyncLoad();
void timeSyncSave();
void timeSyncStatus(Print* p);

#endif