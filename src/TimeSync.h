// TimeSync.h

#ifndef _TIMESYNC_H
#define _TIMESYNC_H

#include "main.h"
#include <time.h>      // configTime(), time()
#include <sys/time.h>  // gettimeofday(), timeval

extern bool timeSynced;
extern int64_t utc_mac_offset;  // add to MAC timer to get UTC microseconds
extern uint32_t last_sync_ms;
extern uint32_t sync_interval_ms;
extern char ts_server[CHAR_BUF_SIZE];

void timeSyncSetup();
void timeSyncLoop();
bool timeSyncNow();  // force offset re-capture; returns false if SNTP not ready

int64_t getEpochUs();   // current UTC in microseconds, -1 if not synced
uint32_t getSyncAge();  // ms since last successful sync, UINT32_MAX if never

void timeSyncLoad();
void timeSyncSave();
void timeSyncStatus(Print* p);

#endif
