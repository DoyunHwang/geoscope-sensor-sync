// main.h

#ifndef _MAIN_h
#define _MAIN_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <string.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>

#define RAW_ROW_BUFFER_SIZE 3
#define RAW_COL_BUFFER_SIZE 500

#define CHAR_BUF_SIZE 32

#define DEVICE_IP 152



extern float amplifierGain;

extern bool fullfilledBuffer;
extern unsigned int currentBufferRow;
extern uint16_t rawBuffer[RAW_ROW_BUFFER_SIZE][RAW_COL_BUFFER_SIZE];
extern int64_t packet_macTime;

extern Print* out;

#endif
