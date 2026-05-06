// Network.h

#ifndef _NETWORK_h
#define _NETWORK_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "main.h"
#include <IPAddress.h>
#include <esp_undocumented.h>

extern char SSID[32], PASSWORD[32];
extern IPAddress  GATEWAY_IP, NETMASK, GEOSCOPE_IP, DNS;


void networkSetup();
void wifiSetup();
void loadWifiConfig();
void saveWifiConfig();
void initWifiConfig();
void showWifiConfig(Print*);

#endif
