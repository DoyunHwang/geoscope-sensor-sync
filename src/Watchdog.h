// Watchdog.h

#ifndef _WATCHDOG_h
#define _WATCHDOG_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "main.h"

void forceReset();

#endif
