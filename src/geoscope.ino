/*
Name: geoscope.ino
Original Created: 6/13/2018 1:34:31 PM
Original Author: Sripong Ariyadech (liurln)
Updated: 02 Oct 2024
 */

// the setup function runs once when you press reset or power the board
#include "Watchdog.h"
#include "ADCModule.h"
#include "MQTTService.h"
#include "Network.h"
#include "cli.h"
#include "TimeSync.h"
#include <ArduinoOTA.h>
#include "main.h"
#include <AsyncPing.h>
#include <StreamUtils.h>


static bool OTA_FS_UPDATE = false;

#define WIFI_PING_INTERVAL_MS 60000
static unsigned long nextPingTime = 0;

void ota_startup() {
	OTA_FS_UPDATE = ArduinoOTA.getCommand() == U_FS;
	if (OTA_FS_UPDATE) {
		out->println(F("<< WARNING: Updating Filesystem via OTA Update >>"));
		mqttNotify("OTA Filesystem Update");
		LittleFS.end();
	}
	else {
		backup(); //save configurations to FS
		mqttNotify("OTA Firmware Update");
		mqttShutdown();
		out->println(F("> Updating Firmware over OTA"));
	}
}

void ota_done() {
	if (OTA_FS_UPDATE) {
		LittleFS.begin();
		OTA_FS_UPDATE = false; //unset the flag
	}
}

void ota_error(ota_error_t error) {
	if (OTA_FS_UPDATE) {
		out->print(F("<< FS UPDATE ERROR "));
		out->print(error);
		out->println(F( " >>" ));
		mqttNotify("OTA FS Update Error");
		ota_done();
	}
	else {
		out->print(F( "<< FIRMWARE UPDATE ERROR " ));
		out->print(error);
		out->println(F( " >>" ));
		ESP.restart();
	}
}


AsyncPing ping;

bool ping_received(const AsyncPingResponse& response) {
	return false; //keep going
}

bool ping_done(const AsyncPingResponse& response) {
	if (response.total_recv == 0) { //if we got no responses
		IPAddress nullIP = IPAddress(0,0,0,0);
		WiFi.config(nullIP, nullIP, nullIP); //force WiFi back to DHCP
		MQTT_BROKER_TIMEOUT = 300000; //disable MQTT timeout because DHCP is slower
		if (WiFi.status() == WL_CONNECTED)
			WiFi.reconnect(); //and reconnect (or just connect freshly)
		return false;
	}
	return true;
}

Print* out = NULL;

void setup() {
	Serial.begin(115200);
	out = &Serial;

	out->println(F( "\n> Starting Geoscope Boot" ));

	LittleFS.begin();
	out->println(F( "> FS Mounted" ));

	networkSetup();
	yield();
	out->println(F( "> WiFi Configured" ));

	TelnetStream.begin();
	out = new WriteLoggingStream(Serial, TelnetStream);
	out->println(( "> Remote Console Configured" ));

	cliInit();
	out->println(F( "> CLI Ready" ));

	adcSetup();
	out->println(F( "> ADC Configured" ));

	mqttLoad(); //attempt to load configuration file
	mqttSetup();
	out->println(F( "> MQTT Configured" ));

	timeSyncSetup();
	out->println(F( "> Time Sync Configured" ));

	// OTA Setup
	ArduinoOTA.onStart(ota_startup);
	ArduinoOTA.onEnd(ota_done);
	ArduinoOTA.onError(ota_error);
	ArduinoOTA.setHostname(("GEOSCOPE_"+clientId).c_str());
	ArduinoOTA.begin();
	out->println(F( "> OTA Ready" ));

	ping.on(true, ping_received);
	ping.on(false, ping_done);
	nextPingTime = millis() + WIFI_PING_INTERVAL_MS;
	out->println(F("> Keepalive Ping Enabled"));

	ESP.wdtDisable();
	ESP.wdtEnable(5000);
	out->println(F( "> Boot Complete" ));
}

// the loop function runs over and over again until power down or reset
void loop() {
	ArduinoOTA.handle();
	adcPoll();
	mqttSend();
	timeSyncLoop();

	if (millis() >= nextPingTime ) { //every interval
		nextPingTime = millis() + WIFI_PING_INTERVAL_MS;
		ping.begin(WiFi.gatewayIP(),5); //try pinging the gateway
	}

	cli_loop();

	ESP.wdtFeed();
	//delay(1); //so the modem can possibly sleep sometimes
}
