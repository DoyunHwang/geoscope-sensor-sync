//
//

#include "TimeSync.h"
#include "Network.h"  // brings in esp_undocumented.h (ESP_WDEV_TIMESTAMP)

extern char MQTT_BROKER_IP[];

bool timeSynced = false;
int64_t utc_mac_offset = 0;
uint32_t last_sync_ms = 0;
uint32_t sntp_poll_s = 3600;         // 1 hour
char ts_server[CHAR_BUF_SIZE] = "";

static void captureOffset() {
    timeval tv;
    // Disable interrupts for the minimum time needed to atomically read
    // both clocks; prevents the ADC ISR from inserting a gap between them.
    noInterrupts();
    int64_t mac = ESP_WDEV_TIMESTAMP();
    gettimeofday(&tv, nullptr);
    interrupts();
    utc_mac_offset = (int64_t)tv.tv_sec * 1000000LL + tv.tv_usec - mac;
    timeSynced = true;
    last_sync_ms = millis();
}

void timeSyncSetup() {
    timeSyncLoad();
    const char* server = ts_server[0] ? ts_server : MQTT_BROKER_IP;
    configTime(0, 0, server);
}

void timeSyncSetPollInterval(uint32_t seconds) {
    sntp_poll_s = seconds;
}

void timeSyncLoop() {
    if (time(nullptr) < 1000000000LL)
        return;  // SNTP not yet synced
    if (!timeSynced || millis() - last_sync_ms > sntp_poll_s * 1000UL)
        captureOffset();
}

bool timeSyncNow() {
    if (time(nullptr) < 1000000000LL) return false;
    captureOffset();
    return true;
}

int64_t getEpochUs() {
    if (!timeSynced) return -1;
    return ESP_WDEV_TIMESTAMP() + utc_mac_offset;
}

uint32_t getSyncAge() {
    if (!timeSynced) return UINT32_MAX;
    return millis() - last_sync_ms;
}

void timeSyncLoad() {
    File f = LittleFS.open("/config/timesync/server", "r");
    if (f) {
        String s = f.readString();
        s.trim();
        strncpy(ts_server, s.c_str(), CHAR_BUF_SIZE);
        ts_server[CHAR_BUF_SIZE - 1] = '\0';
        f.close();
    }
    f = LittleFS.open("/config/timesync/poll", "r");
    if (f) {
        int v = f.parseInt();
        if (v > 0) sntp_poll_s = (uint32_t)v;
        f.close();
    }
}

void timeSyncSave() {
    File f = LittleFS.open("/config/timesync/server", "w");
    if (f) { f.println(ts_server); f.close(); }
    f = LittleFS.open("/config/timesync/poll", "w");
    if (f) { f.println(sntp_poll_s); f.close(); }
}

void timeSyncStatus(Print* p) {
    p->print(F("NTP Server    : "));
    p->println(ts_server[0] ? ts_server : MQTT_BROKER_IP);
    p->print(F("Recapture     : "));
    p->print(sntp_poll_s);
    p->println(F(" s"));
    p->print(F("Synced        : "));
    p->println(timeSynced ? F("yes") : F("no"));
    if (timeSynced) {
        p->print(F("Sync age      : "));
        p->print(getSyncAge() / 1000);
        p->println(F(" s"));
        char buf[24];
        snprintf(buf, sizeof(buf), "%lld", utc_mac_offset);
        p->print(F("UTC offset    : "));
        p->print(buf);
        p->println(F(" us"));
        snprintf(buf, sizeof(buf), "%lld", getEpochUs());
        p->print(F("Epoch now     : "));
        p->print(buf);
        p->println(F(" us"));
    }
}
