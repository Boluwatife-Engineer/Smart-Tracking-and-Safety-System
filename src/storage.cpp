#include "storage.h"

#include <Arduino.h>
#include <Preferences.h>
#include <LittleFS.h>


// ======================================================
// PREFERENCES
// ======================================================

Preferences preferences;

bool preferencesReady = false;


// ======================================================
// LITTLEFS
// ======================================================

bool littleFSReady = false;

#define OFFLINE_DIRECTORY "/offline"
#define OFFLINE_FILE_PREFIX "/offline/record_"
#define OFFLINE_FILE_SUFFIX ".json"


// ======================================================
// OFFLINE QUEUE
// ======================================================

int offlineRecordCount = 0;
int nextRecordID = 0;


// ======================================================
// INTERNAL HELPER
// ======================================================

String getRecordPath(int id)
{
    return String(OFFLINE_FILE_PREFIX) +
           String(id) +
           String(OFFLINE_FILE_SUFFIX);
}


// ======================================================
// GET RECORD ID
// ======================================================

int getRecordID(const String &filename)
{
    String name = filename;

    int slash = name.lastIndexOf('/');

    if (slash >= 0)
    {
        name = name.substring(slash + 1);
    }

    const String prefix = "record_";
    const String suffix = ".json";

    if (!name.startsWith(prefix))
    {
        return -1;
    }

    if (!name.endsWith(suffix))
    {
        return -1;
    }

    int start = prefix.length();

    int end =
        name.length() -
        suffix.length();

    if (start >= end)
    {
        return -1;
    }

    String idString =
        name.substring(
            start,
            end
        );

    for (unsigned int i = 0; i < idString.length(); i++)
    {
        if (!isDigit(idString[i]))
        {
            return -1;
        }
    }

    return idString.toInt();
}


// ======================================================
// SCAN OFFLINE RECORDS
// ======================================================

void scanOfflineRecords()
{
    offlineRecordCount = 0;

    int highestID = -1;

    if (!littleFSReady)
    {
        return;
    }

    File directory =
        LittleFS.open(
            OFFLINE_DIRECTORY
        );

    if (!directory)
    {
        Serial.println(
            "ERROR: Could not open offline directory."
        );

        return;
    }

    File file =
        directory.openNextFile();

    while (file)
    {
        if (!file.isDirectory())
        {
            String filename =
                file.name();

            int id =
                getRecordID(
                    filename
                );

            if (id >= 0)
            {
                offlineRecordCount++;

                if (id > highestID)
                {
                    highestID = id;
                }

                Serial.print(
                    "Found offline record: "
                );

                Serial.println(
                    filename
                );
            }
        }

        file.close();

        file =
            directory.openNextFile();
    }

    directory.close();

    nextRecordID =
        highestID + 1;

    Serial.print(
        "LittleFS offline records: "
    );

    Serial.println(
        offlineRecordCount
    );

    Serial.print(
        "Next record ID: "
    );

    Serial.println(
        nextRecordID
    );
}


// ======================================================
// INITIALIZE STORAGE
// ======================================================

void initStorage()
{
    Serial.println();

    Serial.println(
        "========== STORAGE INITIALIZATION =========="
    );


    // ==================================================
    // PREFERENCES
    // ==================================================

    preferencesReady =
        preferences.begin(
            "tracker",
            false
        );

    if (preferencesReady)
    {
        Serial.println(
            "Preferences initialized."
        );
    }
    else
    {
        Serial.println(
            "ERROR: Preferences initialization failed."
        );
    }


    // ==================================================
    // LITTLEFS
    // ==================================================

    littleFSReady =
        LittleFS.begin(
            true
        );

    if (!littleFSReady)
    {
        Serial.println(
            "ERROR: LittleFS initialization failed."
        );

        return;
    }

    Serial.println(
        "LittleFS initialized."
    );


    // ==================================================
    // OFFLINE DIRECTORY
    // ==================================================

    if (
        !LittleFS.exists(
            OFFLINE_DIRECTORY
        )
    )
    {
        if (
            LittleFS.mkdir(
                OFFLINE_DIRECTORY
            )
        )
        {
            Serial.println(
                "Offline directory created."
            );
        }
        else
        {
            Serial.println(
                "ERROR: Failed to create offline directory."
            );

            return;
        }
    }
    else
    {
        Serial.println(
            "Offline directory already exists."
        );
    }


    // ==================================================
    // SCAN EXISTING RECORDS
    // ==================================================

    scanOfflineRecords();


    // ==================================================
    // RESTORE NEXT RECORD ID
    // ==================================================

    if (preferencesReady)
    {
        int storedNextID =
            preferences.getInt(
                "nextID",
                0
            );

        if (
            storedNextID > nextRecordID
        )
        {
            nextRecordID =
                storedNextID;
        }

        preferences.putInt(
            "nextID",
            nextRecordID
        );
    }


    // ==================================================
    // STATUS
    // ==================================================

    Serial.println();

    Serial.print(
        "Offline records waiting: "
    );

    Serial.println(
        offlineRecordCount
    );

    Serial.print(
        "Next record ID: "
    );

    Serial.println(
        nextRecordID
    );

    Serial.println(
        "Storage initialization complete."
    );

    Serial.println(
        "============================================"
    );
}


// ======================================================
// SAVE LAST SEEN
// ======================================================

void saveLastSeen(
    const String &value
)
{
    if (!preferencesReady)
    {
        Serial.println(
            "ERROR: Preferences not ready."
        );

        return;
    }

    preferences.putString(
        "lastSeen",
        value
    );

    Serial.println(
        "Last seen saved to Preferences."
    );
}


// ======================================================
// LOAD LAST SEEN
// ======================================================

String loadLastSeen()
{
    if (!preferencesReady)
    {
        return "";
    }

    return preferences.getString(
        "lastSeen",
        ""
    );
}


// ======================================================
// SAVE OFFLINE RECORD
// ======================================================

bool saveOfflineRecord(
    const String &payload
)
{
    if (!littleFSReady)
    {
        Serial.println(
            "ERROR: LittleFS is not ready."
        );

        return false;
    }

    if (payload.length() == 0)
    {
        Serial.println(
            "ERROR: Cannot save empty payload."
        );

        return false;
    }

    int recordID =
        nextRecordID;

    String path =
        getRecordPath(
            recordID
        );

    Serial.println();

    Serial.println(
        "Saving offline record..."
    );

    Serial.print(
        "File: "
    );

    Serial.println(
        path
    );


    File file =
        LittleFS.open(
            path,
            FILE_WRITE
        );

    if (!file)
    {
        Serial.println(
            "ERROR: Failed to create offline file."
        );

        return false;
    }


    size_t written =
        file.print(
            payload
        );

    file.flush();

    file.close();


    if (
        written != payload.length()
    )
    {
        Serial.println(
            "ERROR: Incomplete offline record write."
        );

        LittleFS.remove(
            path
        );

        return false;
    }


    if (!LittleFS.exists(path))
    {
        Serial.println(
            "ERROR: Offline file does not exist after write."
        );

        return false;
    }


    offlineRecordCount++;

    nextRecordID =
        recordID + 1;


    if (preferencesReady)
    {
        preferences.putInt(
            "nextID",
            nextRecordID
        );
    }


    Serial.println(
        "Offline record saved."
    );

    Serial.print(
        "Offline records waiting: "
    );

    Serial.println(
        offlineRecordCount
    );

    return true;
}


// ======================================================
// FIND NEWEST RECORD ID
// ======================================================

int findNewestRecordID()
{
    if (!littleFSReady)
    {
        return -1;
    }

    File directory =
        LittleFS.open(
            OFFLINE_DIRECTORY
        );

    if (!directory)
    {
        Serial.println(
            "ERROR: Cannot open offline directory."
        );

        return -1;
    }

    int newestID = -1;

    File file =
        directory.openNextFile();

    while (file)
    {
        if (!file.isDirectory())
        {
            String filename =
                file.name();

            int id =
                getRecordID(
                    filename
                );

            if (id >= 0)
            {
                if (
                    newestID == -1 ||
                    id > newestID
                )
                {
                    newestID = id;
                }
            }
        }

        file.close();

        file =
            directory.openNextFile();
    }

    directory.close();

    return newestID;
}


// ======================================================
// GET OFFLINE RECORD
// ======================================================
//
// index 0 = newest record
//
// ======================================================

String getOfflineRecord(
    int index
)
{
    if (!littleFSReady)
    {
        return "";
    }

    if (index < 0)
    {
        return "";
    }

    if (index >= offlineRecordCount)
    {
        return "";
    }

    if (index != 0)
    {
        Serial.println(
            "ERROR: Only index 0 is supported."
        );

        return "";
    }


    // Get numerically newest record
    int newestID =
        findNewestRecordID();

    if (newestID < 0)
    {
        Serial.println(
            "ERROR: No offline record found."
        );

        return "";
    }


    String path =
        getRecordPath(
            newestID
        );

    Serial.print(
        "Reading offline record: "
    );

    Serial.println(
        path
    );


    File file =
        LittleFS.open(
            path,
            FILE_READ
        );

    if (!file)
    {
        Serial.println(
            "ERROR: Failed to open offline record."
        );

        return "";
    }


    String payload =
        file.readString();

    file.close();

    payload.trim();


    if (payload.length() == 0)
    {
        Serial.println(
            "ERROR: Offline record is empty."
        );

        return "";
    }


    Serial.print(
        "Offline payload loaded: "
    );

    Serial.println(
        payload
    );

    return payload;
}


// ======================================================
// DELETE OFFLINE RECORD
// ======================================================
//
// index 0 = newest record
//
// ======================================================

bool deleteOfflineRecord(
    int index
)
{
    if (!littleFSReady)
    {
        return false;
    }

    if (index != 0)
    {
        Serial.println(
            "ERROR: Only index 0 is supported."
        );

        return false;
    }

    if (offlineRecordCount <= 0)
    {
        Serial.println(
            "ERROR: No offline records to delete."
        );

        return false;
    }


    // Delete the same newest record
    // returned by getOfflineRecord(0)

    int newestID =
        findNewestRecordID();

    if (newestID < 0)
    {
        Serial.println(
            "ERROR: Could not identify newest offline record."
        );

        return false;
    }


    String path =
        getRecordPath(
            newestID
        );


    Serial.print(
        "Deleting offline record: "
    );

    Serial.println(
        path
    );


    if (!LittleFS.exists(path))
    {
        Serial.println(
            "ERROR: Offline file does not exist."
        );

        return false;
    }


    if (!LittleFS.remove(path))
    {
        Serial.println(
            "ERROR: Failed to delete offline record."
        );

        return false;
    }


    if (offlineRecordCount > 0)
    {
        offlineRecordCount--;
    }


    Serial.println(
        "Offline record deleted successfully."
    );

    Serial.print(
        "Offline records remaining: "
    );

    Serial.println(
        offlineRecordCount
    );

    return true;
}


// ======================================================
//  ALL OFFLINE RECORDS
// ======================================================
//
// ONE-TIME CLEANUP OF OLD TEST DATA
//
// ======================================================

bool clearAllOfflineRecords()
{
    if (!littleFSReady)
    {
        Serial.println("ERROR: LittleFS is not ready.");
        return false;
    }

    File directory = LittleFS.open(OFFLINE_DIRECTORY);

    if (!directory)
    {
        Serial.println("ERROR: Could not open offline directory.");
        return false;
    }

    int deleted = 0;
    int failed = 0;

    File file = directory.openNextFile();

    while (file)
    {
        String filename = file.name();

        bool isRecord =
            !file.isDirectory() &&
            getRecordID(filename) >= 0;

        file.close();

        if (isRecord)
        {
            String fullPath;

            // LittleFS may return either:
            // record_91.json
            // or
            // /offline/record_91.json

            if (filename.startsWith("/"))
            {
                fullPath = filename;
            }
            else
            {
                fullPath =
                    String(OFFLINE_DIRECTORY) +
                    "/" +
                    filename;
            }

            Serial.print("Deleting: ");
            Serial.println(fullPath);

            if (LittleFS.exists(fullPath))
            {
                if (LittleFS.remove(fullPath))
                {
                    deleted++;
                }
                else
                {
                    failed++;

                    Serial.print(
                        "ERROR: Failed to delete: "
                    );

                    Serial.println(fullPath);
                }
            }
            else
            {
                failed++;

                Serial.print(
                    "ERROR: File does not exist: "
                );

                Serial.println(fullPath);
            }
        }

        file =
            directory.openNextFile();
    }

    directory.close();

    // Re-scan instead of blindly setting the count.
    // This makes the internal queue state match
    // what is actually still on LittleFS.
    scanOfflineRecords();

    if (offlineRecordCount == 0)
    {
        nextRecordID = 0;

        if (preferencesReady)
        {
            preferences.putInt(
                "nextID",
                0
            );
        }
    }

    Serial.println();
    Serial.println(
        "========== OFFLINE QUEUE CLEAR =========="
    );

    Serial.print(
        "Deleted: "
    );

    Serial.println(deleted);

    Serial.print(
        "Failed: "
    );

    Serial.println(failed);

    Serial.print(
        "Remaining: "
    );

    Serial.println(
        offlineRecordCount
    );

    Serial.println(
        "=========================================="
    );

    return failed == 0;
}


// ======================================================
// GET OFFLINE RECORD COUNT
// ======================================================

int getOfflineRecordCount()
{
    return offlineRecordCount;
}


// ======================================================
// GENERIC STRING
// ======================================================

void saveString(
    const char *key,
    const String &value
)
{
    if (!preferencesReady)
    {
        return;
    }

    preferences.putString(
        key,
        value
    );
}


String loadString(
    const char *key,
    const String &defaultValue
)
{
    if (!preferencesReady)
    {
        return defaultValue;
    }

    return preferences.getString(
        key,
        defaultValue
    );
}


// ======================================================
// GENERIC FLOAT
// ======================================================

void saveFloat(
    const char *key,
    float value
)
{
    if (!preferencesReady)
    {
        return;
    }

    preferences.putFloat(
        key,
        value
    );
}


float loadFloat(
    const char *key,
    float defaultValue
)
{
    if (!preferencesReady)
    {
        return defaultValue;
    }

    return preferences.getFloat(
        key,
        defaultValue
    );
}


// ======================================================
// GENERIC INT
// ======================================================

void saveInt(
    const char *key,
    int value
)
{
    if (!preferencesReady)
    {
        return;
    }

    preferences.putInt(
        key,
        value
    );
}


int loadInt(
    const char *key,
    int defaultValue
)
{
    if (!preferencesReady)
    {
        return defaultValue;
    }

    return preferences.getInt(
        key,
        defaultValue
    );
}


// ======================================================
// GENERIC BOOL
// ======================================================

void saveBool(
    const char *key,
    bool value
)
{
    if (!preferencesReady)
    {
        return;
    }

    preferences.putBool(
        key,
        value
    );
}


bool loadBool(
    const char *key,
    bool defaultValue
)
{
    if (!preferencesReady)
    {
        return defaultValue;
    }

    return preferences.getBool(
        key,
        defaultValue
    );
}