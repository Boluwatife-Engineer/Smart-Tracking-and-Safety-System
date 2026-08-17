#include "gps.h"
#include <Arduino.h>
#include "sim7600.h"

// ======================================================
// GPS DATA
// ======================================================

float latitude = 0.0;
float longitude = 0.0;
float altitude = 0.0;

String gpsTime = "";
String gpsDate = "";

bool gpsFix = false;

// ======================================================
// PARSE GPS INFO
// ======================================================

void parseGPSInfo(String response)
{
    int start = response.indexOf("+CGPSINFO:");

    if (start == -1)
    {
        gpsFix = false;

        Serial.println("GPS parser: +CGPSINFO not found.");

        return;
    }

    String data = response.substring(
        start + strlen("+CGPSINFO:")
    );

    data.trim();

    Serial.print("GPS DATA: [");
    Serial.print(data);
    Serial.println("]");

    // ==================================================
    // NO FIX
    // ==================================================

    if (data.startsWith(","))
    {
        gpsFix = false;

        Serial.println(
            "GPS modem response: NO FIX"
        );

        return;
    }

    // ==================================================
    // SPLIT FIELDS
    // ==================================================

    String fields[8];

    int fieldIndex = 0;
    int previousIndex = 0;

    for (int i = 0; i <= data.length(); i++)
    {
        if (
            data[i] == ',' ||
            i == data.length()
        )
        {
            if (fieldIndex < 8)
            {
                fields[fieldIndex] =
                    data.substring(
                        previousIndex,
                        i
                    );
            }

            fieldIndex++;
            previousIndex = i + 1;
        }
    }

    Serial.print("GPS fields: ");
    Serial.println(fieldIndex);

    // ==================================================
    // CHECK LATITUDE
    // ==================================================

    if (
        fields[0].length() == 0 ||
        fields[1].length() == 0
    )
    {
        gpsFix = false;

        Serial.println(
            "GPS parser: latitude unavailable."
        );

        return;
    }

    // ==================================================
    // CHECK LONGITUDE
    // ==================================================

    if (
        fields[2].length() == 0 ||
        fields[3].length() == 0
    )
    {
        gpsFix = false;

        Serial.println(
            "GPS parser: longitude unavailable."
        );

        return;
    }

    // ==================================================
    // RAW COORDINATES
    // ==================================================

    float rawLat =
        fields[0].toFloat();

    float rawLon =
        fields[2].toFloat();

    // ==================================================
    // LATITUDE DDMM.MMMMM -> DECIMAL DEGREES
    // ==================================================

    int latDegrees =
        (int)(rawLat / 100);

    float latMinutes =
        rawLat -
        (latDegrees * 100);

    latitude =
        latDegrees +
        (latMinutes / 60.0);

    // ==================================================
    // LONGITUDE DDDMM.MMMMM -> DECIMAL DEGREES
    // ==================================================

    int lonDegrees =
        (int)(rawLon / 100);

    float lonMinutes =
        rawLon -
        (lonDegrees * 100);

    longitude =
        lonDegrees +
        (lonMinutes / 60.0);

    // ==================================================
    // SOUTH / WEST
    // ==================================================

    if (fields[1] == "S")
    {
        latitude = -latitude;
    }

    if (fields[3] == "W")
    {
        longitude = -longitude;
    }

    // ==================================================
    // DATE
    // ==================================================

    if (fields[4].length() > 0)
    {
        gpsDate = fields[4];
    }

    // ==================================================
    // TIME
    // ==================================================

    if (fields[5].length() > 0)
    {
        gpsTime = fields[5];
    }

    // ==================================================
    // ALTITUDE
    // ==================================================

    if (fields[6].length() > 0)
    {
        altitude =
            fields[6].toFloat();
    }

    gpsFix = true;

    Serial.println(
        "GPS parser: FIX FOUND."
    );
}

// ======================================================
// INITIALIZE GPS
// ======================================================

void initGPS()
{
    Serial.println();
    Serial.println(
        "========== GPS INITIALIZATION =========="
    );

    sendSIM7600Command(
        "AT+CGPS?",
        1000
    );

    Serial.println(
        "GPS initialization complete."
    );

    Serial.println(
        "Waiting for satellite fix..."
    );
}

// ======================================================
// UPDATE GPS
// ======================================================

void updateGPS()
{
    // ==================================================
    // CLEAR OLD UART DATA
    // ==================================================

    while (sim7600.available())
    {
        sim7600.read();
    }

    // ==================================================
    // REQUEST GPS DATA
    // ==================================================

    sim7600.println(
        "AT+CGPSINFO"
    );

    String response = "";

    unsigned long start =
        millis();

    while (
        millis() - start < 1500
    )
    {
        while (sim7600.available())
        {
            response +=
                (char)sim7600.read();
        }
    }

    // ==================================================
    // DEBUG RAW MODEM RESPONSE
    // ==================================================

    Serial.println();
    Serial.println(
        "========== GPS RAW RESPONSE =========="
    );

    Serial.print(response);

    Serial.println();
    Serial.println(
        "======================================="
    );

    // ==================================================
    // PARSE
    // ==================================================

    parseGPSInfo(response);

    // ==================================================
    // DISPLAY RESULT
    // ==================================================

    if (gpsFix)
    {
        Serial.println();
        Serial.println(
            "========== GPS DATA =========="
        );

        Serial.print(
            "Latitude:  "
        );

        Serial.println(
            latitude,
            6
        );

        Serial.print(
            "Longitude: "
        );

        Serial.println(
            longitude,
            6
        );

        Serial.print(
            "Altitude:  "
        );

        Serial.print(
            altitude,
            1
        );

        Serial.println(
            " m"
        );

        Serial.print(
            "Date:      "
        );

        Serial.println(
            gpsDate
        );

        Serial.print(
            "Time:      "
        );

        Serial.println(
            gpsTime
        );

        Serial.println(
            "GPS STATUS: FIXED"
        );

        Serial.println(
            "=============================="
        );
    }
    else
    {
        Serial.println(
            "GPS STATUS: SEARCHING..."
        );
    }
}

// ======================================================
// GETTERS
// ======================================================

bool hasGPSFix()
{
    return gpsFix;
}

float getLatitude()
{
    return latitude;
}

float getLongitude()
{
    return longitude;
}

float getAltitude()
{
    return altitude;
}

String getGPSTime()
{
    return gpsTime;
}

String getGPSDate()
{
    return gpsDate;
}