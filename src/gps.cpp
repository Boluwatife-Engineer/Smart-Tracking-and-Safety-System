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
    // ==================================================
    // FIND +CGPSINFO
    // ==================================================

    int start =
        response.indexOf("+CGPSINFO:");

    if (start == -1)
    {
        gpsFix = false;

        Serial.println(
            "GPS parser: +CGPSINFO not found."
        );

        return;
    }


    // ==================================================
    // GET GPS DATA
    // ==================================================

    String data =
        response.substring(
            start + strlen("+CGPSINFO:")
        );


    // ==================================================
    // CLEAN MODEM RESPONSE
    // ==================================================

    data.replace("\r", "");
    data.replace("\n", "");

    int okIndex =
        data.indexOf("OK");

    if (okIndex != -1)
    {
        data =
            data.substring(
                0,
                okIndex
            );
    }

    data.trim();


    Serial.print(
        "GPS CLEAN DATA: ["
    );

    Serial.print(data);

    Serial.println(
        "]"
    );


    // ==================================================
    // NO DATA / NO FIX
    // ==================================================

    if (
        data.length() == 0 ||
        data.startsWith(",")
    )
    {
        gpsFix = false;

        Serial.println(
            "GPS modem response: NO FIX"
        );

        return;
    }


    // ==================================================
    // SPLIT GPS FIELDS
    //
    // +CGPSINFO:
    //
    // latitude,
    // N/S,
    // longitude,
    // E/W,
    // date,
    // time,
    // altitude,
    // ...
    //
    // ==================================================

    String fields[8];

    int fieldIndex = 0;
    int previousIndex = 0;


    for (
        int i = 0;
        i <= data.length();
        i++
    )
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

                fields[fieldIndex].trim();
            }

            fieldIndex++;

            previousIndex =
                i + 1;
        }
    }


    Serial.print(
        "GPS fields: "
    );

    Serial.println(
        fieldIndex
    );


    // ==================================================
    // DEBUG FIELDS
    // ==================================================

    for (
        int i = 0;
        i < 8;
        i++
    )
    {
        Serial.print(
            "Field["
        );

        Serial.print(
            i
        );

        Serial.print(
            "] = ["
        );

        Serial.print(
            fields[i]
        );

        Serial.println(
            "]"
        );
    }


    // ==================================================
    // VALIDATE LATITUDE
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
    // VALIDATE LONGITUDE
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
    // RAW GPS COORDINATES
    //
    // Latitude:
    // DDMM.MMMMM
    //
    // Longitude:
    // DDDMM.MMMMM
    //
    // ==================================================

    float rawLat =
        fields[0].toFloat();

    float rawLon =
        fields[2].toFloat();


    // ==================================================
    // CONVERT LATITUDE
    // DDMM.MMMMM
    // TO DECIMAL DEGREES
    // ==================================================

    int latDegrees =
        (int)(
            rawLat / 100.0
        );

    float latMinutes =
        rawLat -
        (
            latDegrees *
            100.0
        );

    latitude =
        latDegrees +
        (
            latMinutes /
            60.0
        );


    // ==================================================
    // CONVERT LONGITUDE
    // DDDMM.MMMMM
    // TO DECIMAL DEGREES
    // ==================================================

    int lonDegrees =
        (int)(
            rawLon / 100.0
        );

    float lonMinutes =
        rawLon -
        (
            lonDegrees *
            100.0
        );

    longitude =
        lonDegrees +
        (
            lonMinutes /
            60.0
        );


    // ==================================================
    // SOUTH / WEST
    // ==================================================

    if (
        fields[1] == "S"
    )
    {
        latitude =
            -latitude;
    }

    if (
        fields[3] == "W"
    )
    {
        longitude =
            -longitude;
    }


    // ==================================================
    // DATE
    // ==================================================

    if (
        fields[4].length() > 0
    )
    {
        gpsDate =
            fields[4];
    }


    // ==================================================
    // TIME
    // ==================================================

    if (
        fields[5].length() > 0
    )
    {
        gpsTime =
            fields[5];
    }


    // ==================================================
    // ALTITUDE
    // ==================================================

    if (
        fields[6].length() > 0
    )
    {
        altitude =
            fields[6].toFloat();
    }


    // ==================================================
    // GPS FIX CONFIRMED
    // ==================================================

    gpsFix = true;


    Serial.println();
    Serial.println(
        "******** GPS FIX FOUND ********"
    );

    Serial.print(
        "Latitude: "
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
        "Altitude: "
    );

    Serial.print(
        altitude,
        1
    );

    Serial.println(
        " m"
    );

    Serial.print(
        "Date: "
    );

    Serial.println(
        gpsDate
    );

    Serial.print(
        "Time: "
    );

    Serial.println(
        gpsTime
    );

    Serial.println(
        "GPS STATUS: FIXED"
    );

    Serial.println(
        "********************************"
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


    // ==================================================
    // CHECK GPS STATE
    // ==================================================

    Serial.println(
        "Checking GPS state..."
    );

    sendSIM7600Command(
        "AT+CGPS?",
        1000
    );


    // ==================================================
    // TURN GPS ON
    // ==================================================

    Serial.println(
        "Turning GPS ON..."
    );

    sendSIM7600Command(
        "AT+CGPS=1",
        2000
    );


    // ==================================================
    // VERIFY GPS STATE
    // ==================================================

    Serial.println(
        "Verifying GPS state..."
    );

    sendSIM7600Command(
        "AT+CGPS?",
        1000
    );


    // ==================================================
    // INITIAL STATE
    // ==================================================

    gpsFix = false;

    Serial.println();

    Serial.println(
        "GPS initialization complete."
    );

    Serial.println(
        "Waiting for satellite fix..."
    );

    Serial.println();
}


// ======================================================
// UPDATE GPS
// ======================================================

void updateGPS()
{
    // ==================================================
    // CLEAR OLD UART DATA
    // ==================================================

    while (
        sim7600.available()
    )
    {
        sim7600.read();
    }


    // ==================================================
    // REQUEST GPS INFORMATION
    // ==================================================

    Serial.println();
    Serial.println(
        "Requesting GPS information..."
    );

    sim7600.println(
        "AT+CGPSINFO"
    );


    // ==================================================
    // READ RESPONSE
    // ==================================================

    String response = "";

    unsigned long start =
        millis();


    while (
        millis() - start < 2000
    )
    {
        while (
            sim7600.available()
        )
        {
            response +=
                (char)
                sim7600.read();
        }
    }


    // ==================================================
    // RAW MODEM RESPONSE
    // ==================================================

    Serial.println();
    Serial.println(
        "========== GPS RAW RESPONSE =========="
    );

    Serial.print(
        response
    );

    Serial.println();

    Serial.println(
        "======================================="
    );


    // ==================================================
    // PARSE RESPONSE
    // ==================================================

    parseGPSInfo(
        response
    );


    // ==================================================
    // DISPLAY RESULT
    // ==================================================

    if (
        gpsFix
    )
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
        Serial.println();
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