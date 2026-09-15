#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>

// ======================================================
// STORAGE INITIALIZATION
// ======================================================

void initStorage();


// ======================================================
// LAST SEEN
// ======================================================
//
// Keep the original interface used by BLE code.
//
// ======================================================

void saveLastSeen(const String &value);

String loadLastSeen();


// ======================================================
// OFFLINE HISTORY QUEUE
// ======================================================

bool saveOfflineRecord(const String &payload);

String getOfflineRecord(int index);

bool deleteOfflineRecord(int index);

int getOfflineRecordCount();

bool clearAllOfflineRecords();


// ======================================================
// OPTIONAL PERSISTENT SETTINGS
// ======================================================

void saveString(
    const char *key,
    const String &value
);

String loadString(
    const char *key,
    const String &defaultValue = ""
);


void saveFloat(
    const char *key,
    float value
);

float loadFloat(
    const char *key,
    float defaultValue = 0.0
);


void saveInt(
    const char *key,
    int value
);

int loadInt(
    const char *key,
    int defaultValue = 0
);


void saveBool(
    const char *key,
    bool value
);

bool loadBool(
    const char *key,
    bool defaultValue = false
);

#endif