// ADCModule.h

#ifndef _ADCMODULE_h
#define _ADCMODULE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "main.h"
#include <SPI.h>
#include <MCP41XXX.h>

#define ADC_HOLD_TIME_CYCLES 8 //from MCP3201 datasheet: t_SUCS >= 100ns
#define SAMP_CLK_ERROR_THRESHOLD 10 //number of too-short or too-long samples to wait before adjusting the sample clock

extern MCP41xxx* gainPot;
extern double gainShiftRatio;
extern unsigned int sample_rate;

void adcSetup();
void adcPoll();
void samplingEnable();
void samplingDisable();
void changeAmplifierGain(float);
void changeSampleRate(unsigned int);
void gainSave();
void gainLoad();

#endif
