// TimeSync.cpp

#include "TimeSync.h"
#include "Network.h"  // brings in esp_undocumented.h (ESP_WDEV_TIMESTAMP)

extern char MQTT_BROKER_IP[];

volatile bool timeSynced = false;
volatile int64_t utc_mac_offset = 0;
volatile uint32_t last_sync_ms = 0;

uint32_t sntp_poll_s = 3600;             // offset recapture interval: 1 hour
char ts_server[CHAR_BUF_SIZE] = "";

static const uint32_t MAX_RECAPTURE_INTERVAL_S = UINT32_MAX / 1000UL;

static uint32_t sanitizeRecaptureInterval(uint32_t seconds) {
    if (seconds == 0) {
        return 1;
    }

    if (seconds > MAX_RECAPTURE_INTERVAL_S) {
        return MAX_RECAPTURE_INTERVAL_S;
    }

    return seconds;
}

static void captureOffset() {
    timeval tv;
    int64_t mac;

    // Disable interrupts for the minimum time needed to atomically read
    // both clocks; prevents the ADC ISR from inserting a gap between them.
    noInterrupts();
    mac = ESP_WDEV_TIMESTAMP();
    gettimeofday(&tv, nullptr);
    interrupts();

    const int64_t new_offset =
        (int64_t)tv.tv_sec * 1000000LL + tv.tv_usec - mac;

    const uint32_t sync_ms = millis();

    // Protect the 64-bit write. On ESP8266, int64_t writes are not atomic.
    // This prevents an ISR calling getEpochUs() from reading a partially
    // updated utc_mac_offset.
    noInterrupts();
    utc_mac_offset = new_offset;
    timeSynced = true;
    last_sync_ms = sync_ms;
    interrupts();
}

void timeSyncSetup() {
    timeSyncLoad();

    const char* server = ts_server[0] ? ts_server : MQTT_BROKER_IP;
    configTime(0, 0, server);
}

void timeSyncSetRecaptureInterval(uint32_t seconds) {
    sntp_poll_s = sanitizeRecaptureInterval(seconds);
}

void timeSyncSetPollInterval(uint32_t seconds) {
    // Backward-compatible alias. This controls only the offset recapture
    // interval, not the ESP8266 SNTP stack's own polling interval.
    timeSyncSetRecaptureInterval(seconds);
}

void timeSyncLoop() {
    if (time(nullptr) < 1000000000LL) {
        return;  // SNTP not yet synced
    }

    const uint32_t elapsed_ms = millis() - last_sync_ms;
    const uint64_t recapture_interval_ms =
        (uint64_t)sntp_poll_s * 1000ULL;

    if (!timeSynced || (uint64_t)elapsed_ms > recapture_interval_ms) {
        captureOffset();
    }
}

bool timeSyncNow() {
    if (time(nullptr) < 1000000000LL) {
        return false;
    }

    captureOffset();
    return true;
}

int64_t getEpochUs() {
    if (!timeSynced) {
        return -1;
    }

    // Keep this lock-free because this function may be called from an ISR.
    // The writer side in captureOffset() is protected against 64-bit tearing.
    return ESP_WDEV_TIMESTAMP() + utc_mac_offset;
}

uint32_t getSyncAge() {
    if (!timeSynced) {
        return UINT32_MAX;
    }

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

        if (v > 0) {
            sntp_poll_s = sanitizeRecaptureInterval((uint32_t)v);
        }

        f.close();
    }
}

void timeSyncSave() {
    File f = LittleFS.open("/config/timesync/server", "w");
    if (f) {
        f.println(ts_server);
        f.close();
    }

    f = LittleFS.open("/config/timesync/poll", "w");
    if (f) {
        f.println(sntp_poll_s);
        f.close();
    }
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