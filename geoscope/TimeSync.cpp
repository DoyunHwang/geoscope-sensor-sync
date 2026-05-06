//
//

#include "TimeSync.h"
#include "Network.h"  // brings in esp_undocumented.h (ESP_WDEV_TIMESTAMP)

extern char MQTT_BROKER_IP[];

bool timeSynced = false;
int64_t utc_mac_offset = 0;
uint32_t last_sync_ms = 0;
uint32_t sync_interval_ms = 300000;  // 5 minutes
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

void timeSyncLoop() {
    if (time(nullptr) < 1000000000LL)
        return;  // SNTP not yet synced
    if (!timeSynced || millis() - last_sync_ms > sync_interval_ms)
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
    f = LittleFS.open("/config/timesync/interval", "r");
    if (f) {
        int v = f.parseInt();
        if (v > 0) sync_interval_ms = (uint32_t)v * 1000;
        f.close();
    }
}

void timeSyncSave() {
    File f = LittleFS.open("/config/timesync/server", "w");
    if (f) { f.println(ts_server); f.close(); }
    f = LittleFS.open("/config/timesync/interval", "w");
    if (f) { f.println(sync_interval_ms / 1000); f.close(); }
}

void timeSyncStatus(Print* p) {
    p->print(F("NTP Server : "));
    p->println(ts_server[0] ? ts_server : MQTT_BROKER_IP);
    p->print(F("Synced     : "));
    p->println(timeSynced ? F("yes") : F("no"));
    if (timeSynced) {
        p->print(F("Sync age   : "));
        p->print(getSyncAge() / 1000);
        p->println(F(" s"));
        char buf[24];
        snprintf(buf, sizeof(buf), "%lld", utc_mac_offset);
        p->print(F("UTC offset : "));
        p->print(buf);
        p->println(F(" us"));
        snprintf(buf, sizeof(buf), "%lld", getEpochUs());
        p->print(F("Epoch now  : "));
        p->print(buf);
        p->println(F(" us"));
    }
    p->print(F("Interval   : "));
    p->print(sync_interval_ms / 1000);
    p->println(F(" s"));
}
