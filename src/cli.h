#ifndef _GEOSCOPE_CLI_H
#define _GEOSCOPE_CLI_H

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "main.h"
#include <Commander.h>
#include <TelnetStream.h>

bool cliInit(Stream &main = Serial, Stream &alt = TelnetStream);
void cli_loop();

bool cli_exec(String command, Stream* outPort);

extern Commander cli;

bool backup(Commander &cmd = cli);
bool restore(Commander &cmd = cli);


#endif
