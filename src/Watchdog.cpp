//
//
//

#include "Watchdog.h"
#include "cli.h"

void forceReset() {
	out->println(F("<< forceReset Triggered! >>"));
	ESP.wdtDisable();
	delay(10);
	ESP.restart();
}
