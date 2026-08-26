// ======================================================
// STORAGE
// ======================================================

#include "storage.h"

#include <Preferences.h>

Preferences preferences;


// ======================================================
// CONFIGURATION
// ======================================================

// Maximum number of offline records.
//
// Increase carefully because Preferences/NVS has
// limited storage space.
//
// 20 records is a safe starting point for your payloads.
#define MAX_OFFLINE_RECORDS 20


// ======================================================
// INITIALIZE STORAGE
// ======================================================

void initStorage()
{
    preferences.begin(
        "tracker",
        false
    );

    Serial.println();
    Serial.println(
        "========== STORAGE INITIALIZED =========="
    );

    Serial.print(
        "Offline records: "
    );

    Serial.println(
        getOfflineRecordCount()
    );
}


// ======================================================
// LAST SEEN
// ======================================================

void saveLastSeen(
    const String &value
)
{
    preferences.putString(
        "lastSeen",
        value
    );
}


String loadLastSeen()
{
    return preferences.getString(
        "lastSeen",
        "No Last Seen"
    );
}


// ======================================================
// GET OFFLINE COUNT
// ======================================================

int getOfflineRecordCount()
{
    return preferences.getInt(
        "queueCount",
        0
    );
}


// ======================================================
// SAVE OFFLINE RECORD
// ======================================================
//
// Saves the Firebase HISTORY payload.
//
// The record stays in storage until it is successfully
// uploaded to Firebase.
//
// ======================================================

bool saveOfflineRecord(
    const String &payload
)
{
    int count =
        getOfflineRecordCount();


    // ==================================================
    // QUEUE FULL
    // ==================================================

    if (
        count >= MAX_OFFLINE_RECORDS
    )
    {
        Serial.println();
        Serial.println(
            "OFFLINE QUEUE FULL."
        );

        Serial.println(
            "Dropping oldest record."
        );


        // Delete oldest record.
        for (
            int i = 0;
            i < count - 1;
            i++
        )
        {
            String next =
                preferences.getString(
                    ("q" + String(i + 1)).c_str(),
                    ""
                );


            preferences.putString(
                ("q" + String(i)).c_str(),
                next
            );
        }


        count =
            MAX_OFFLINE_RECORDS - 1;
    }


    // ==================================================
    // SAVE
    // ==================================================

    String key =
        "q" + String(count);


    size_t written =
        preferences.putString(
            key.c_str(),
            payload
        );


    if (
        written == 0
    )
    {
        Serial.println(
            "ERROR: Failed to save offline record."
        );

        return false;
    }


    // ==================================================
    // UPDATE COUNT
    // ==================================================

    preferences.putInt(
        "queueCount",
        count + 1
    );


    Serial.print(
        "Offline record saved. Queue size: "
    );

    Serial.println(
        count + 1
    );


    return true;
}


// ======================================================
// GET OFFLINE RECORD
// ======================================================

String getOfflineRecord(
    int index
)
{
    int count =
        getOfflineRecordCount();


    if (
        index < 0 ||
        index >= count
    )
    {
        return "";
    }


    String key =
        "q" + String(index);


    return preferences.getString(
        key.c_str(),
        ""
    );
}


// ======================================================
// DELETE OFFLINE RECORD
// ======================================================
//
// Deletes one record and shifts all newer records
// forward.
//
// ======================================================

bool deleteOfflineRecord(
    int index
)
{
    int count =
        getOfflineRecordCount();


    if (
        index < 0 ||
        index >= count
    )
    {
        return false;
    }


    // ==================================================
    // SHIFT RECORDS
    // ==================================================

    for (
        int i = index;
        i < count - 1;
        i++
    )
    {
        String next =
            preferences.getString(
                ("q" + String(i + 1)).c_str(),
                ""
            );


        preferences.putString(
            ("q" + String(i)).c_str(),
            next
        );
    }


    // ==================================================
    // REMOVE LAST ENTRY
    // ==================================================

    preferences.remove(
        ("q" + String(count - 1)).c_str()
    );


    // ==================================================
    // UPDATE COUNT
    // ==================================================

    preferences.putInt(
        "queueCount",
        count - 1
    );


    return true;
}


// ======================================================
// CLEAR OFFLINE RECORDS
// ======================================================

void clearOfflineRecords()
{
    int count =
        getOfflineRecordCount();


    for (
        int i = 0;
        i < count;
        i++
    )
    {
        preferences.remove(
            ("q" + String(i)).c_str()
        );
    }


    preferences.putInt(
        "queueCount",
        0
    );


    Serial.println(
        "Offline queue cleared."
    );
}