#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>

// ======================================================
// BASIC STORAGE
// ======================================================

void initStorage();

void saveLastSeen(
    const String &value
);

String loadLastSeen();


// ======================================================
// OFFLINE HISTORY QUEUE
// ======================================================

// Save one Firebase history JSON payload locally.
bool saveOfflineRecord(
    const String &payload
);

// Number of records currently waiting.
int getOfflineRecordCount();

// Get oldest waiting record.
String getOfflineRecord(
    int index
);

// Delete a record after successful Firebase upload.
bool deleteOfflineRecord(
    int index
);

// Remove all offline records.
void clearOfflineRecords();

#endif