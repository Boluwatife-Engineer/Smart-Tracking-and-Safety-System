
// Source file to save the last seen value
#include "storage.h"

#include <Preferences.h>

Preferences preferences;

void initStorage()
{
    preferences.begin("tracker", false);
}

void saveLastSeen(const String &value)
{
    preferences.putString("lastSeen", value);
}

String loadLastSeen()
{
    return preferences.getString(
        "lastSeen",
        "No Last Seen"
    );
}