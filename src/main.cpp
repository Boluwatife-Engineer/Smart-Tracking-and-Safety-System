#include <Arduino.h>


// ======================================================
// MODULES
// ======================================================

#include "mpu6050.h"
#include "storage.h"
#include "ble.h"
#include "battery.h"
#include "gps.h"
#include "gsm.h"
#include "sim7600.h"
#include "tracker_state.h"




// ======================================================
// LOCATION SEND TIMER
// ======================================================

unsigned long lastLocationSend = 0;

const unsigned long LOCATION_SEND_INTERVAL =
    10000;


// ======================================================
// GPS NO-FIX LOG TIMER
// ======================================================

unsigned long lastNoFixLog = 0;

const unsigned long NO_FIX_LOG_INTERVAL =
    30000;


// ======================================================
// SETUP
// ======================================================

void setup()
{
    Serial.begin(115200);

    delay(2000);


    // ==================================================
    // STORAGE
    // ==================================================

    initStorage();


    // ==================================================
    // BATTERY
    // ==================================================

    initBattery();


    // ==================================================
    // MPU6050
    // ==================================================

    if (!initMPU())
    {
        Serial.println(
            "MPU Failed"
        );

        while (true)
        {
            delay(1000);
        }
    }


    // ==================================================
    // SIM7600
    // ==================================================

    initSIM7600();


    // ==================================================
    // GPS
    // ==================================================

    initGPS();


    // ==================================================
    // GSM / INTERNET
    // ==================================================

    initGSM();


    // ==================================================
    // BLE
    // ==================================================

    initBLE();


    // ==================================================
    // READY
    // ==================================================

    Serial.println();
    Serial.println(
        "================================"
    );

    Serial.println(
        "       SMART TRACKER READY"
    );

    Serial.println(
        "================================"
    );

    Serial.println();
}


// ======================================================
// LOOP
// ======================================================

void loop()
{
    // ==================================================
    // MPU6050
    // ==================================================

    updateMPU();


    // ==================================================
    // GPS
    // ==================================================

    updateGPS();


    // ==================================================
    // BLE
    // ==================================================

    updateBLE();


    // ==================================================
    // TRACKER OPERATING STATE
    // ==================================================

    updateTrackerMode();


    Serial.print(
        "TRACKER MODE: "
    );

    Serial.println(
        getTrackerModeName()
    );


    // ==================================================
    // CURRENT TIME
    // ==================================================

    unsigned long currentTime =
        millis();


    // ==================================================
    // GPS FIX AVAILABLE
    // ==================================================

    if (hasGPSFix())
    {
        // ------------------------------------------------
        // SEND LOCATION EVERY 10 SECONDS
        // ------------------------------------------------

        if (
            currentTime -
            lastLocationSend >=
            LOCATION_SEND_INTERVAL
        )
        {
            lastLocationSend =
                currentTime;


            Serial.println();
            Serial.println(
                "================================"
            );

            Serial.println(
                "GPS FIX AVAILABLE"
            );

            Serial.println(
                "Preparing Firebase update"
            );

            Serial.println(
                "================================"
            );


            // ------------------------------------------------
            // GPS DATA
            // ------------------------------------------------

            Serial.print(
                "Latitude:  "
            );

            Serial.println(
                getLatitude(),
                6
            );


            Serial.print(
                "Longitude: "
            );

            Serial.println(
                getLongitude(),
                6
            );


            Serial.print(
                "Altitude:  "
            );

            Serial.println(
                getAltitude(),
                1
            );


            Serial.print(
                "GPS Date:  "
            );

            Serial.println(
                getGPSDate()
            );


            Serial.print(
                "GPS Time:  "
            );

            Serial.println(
                getGPSTime()
            );


            // ------------------------------------------------
            // SEND LOCATION
            // ------------------------------------------------

            bool success =
                sendLocation(
                    getLatitude(),
                    getLongitude(),
                    getAltitude(),
                    getGPSTime(),
                    getGPSDate()
                );


            // ------------------------------------------------
            // RESULT
            // ------------------------------------------------

            if (success)
            {
                Serial.println();

                Serial.println(
                    "GPS LOCATION UPDATE COMPLETED."
                );
            }
            else
            {
                Serial.println();

                Serial.println(
                    "GPS LOCATION UPDATE FAILED."
                );
            }
        }
    }


    // ==================================================
    // GPS NO FIX
    // ==================================================

    else
    {
        // ------------------------------------------------
        // LOG NO-FIX STATUS EVERY 30 SECONDS
        // ------------------------------------------------

        if (
            currentTime -
            lastNoFixLog >=
            NO_FIX_LOG_INTERVAL
        )
        {
            lastNoFixLog =
                currentTime;


            Serial.println();

            Serial.println(
                "================================"
            );

            Serial.println(
                "GPS HAS NO FIX"
            );

            Serial.println(
                "Logging GPS status"
            );

            Serial.println(
                "================================"
            );


            Serial.print(
                "GPS Date: "
            );

            Serial.println(
                getGPSDate()
            );


            Serial.print(
                "GPS Time: "
            );

            Serial.println(
                getGPSTime()
            );


            // ------------------------------------------------
            // LOG NO FIX
            // ------------------------------------------------

            bool success =
                logGPSNoFix(
                    getGPSTime(),
                    getGPSDate()
                );


            // ------------------------------------------------
            // RESULT
            // ------------------------------------------------

            if (success)
            {
                Serial.println();

                Serial.println(
                    "GPS NO_FIX STATUS LOGGED."
                );
            }
            else
            {
                Serial.println();

                Serial.println(
                    "FAILED TO LOG GPS NO_FIX."
                );
            }
        }
    }


    // ==================================================
    // SMALL LOOP DELAY
    // ==================================================

    delay(100);
}