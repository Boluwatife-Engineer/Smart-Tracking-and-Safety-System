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
#include "sos.h"


// ======================================================
// LOCATION SEND TIMER
// ======================================================

unsigned long lastLocationSend = 0;

const unsigned long LOCATION_SEND_INTERVAL = 10000;


// ======================================================
// GPS NO-FIX LOG TIMER
// ======================================================

unsigned long lastNoFixLog = 0;

const unsigned long NO_FIX_LOG_INTERVAL = 30000;


// ======================================================
// SENSOR DEBUG TIMER
// ======================================================

unsigned long lastSensorLog = 0;

const unsigned long SENSOR_LOG_INTERVAL = 2000;


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
    // ONE-TIME CLEANUP OF OLD TEST QUEUE
    // ==================================================
    //
    // RUN THIS ONCE.
    //
    // AFTER THE QUEUE IS CLEARED:
    // REMOVE OR COMMENT OUT THIS LINE.
    //
    // ==================================================

    // clearAllOfflineRecords();


    // ==================================================
    // BATTERY
    // ==================================================

    initBattery();


    // ==================================================
    // MPU6050
    // ==================================================

    if (!initMPU())
    {
        Serial.println("MPU Failed");

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
    // SOS
    // ==================================================

    initSOS();


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

    Serial.println(
        "SOS BUTTON READY"
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
    // SOS BUTTON
    // ==================================================

    updateSOS();


    // ==================================================
    // TRACKER MODE
    // ==================================================

    updateTrackerMode();


    // ==================================================
    // CURRENT TIME
    // ==================================================

    unsigned long currentTime =
        millis();


    // ==================================================
    // LIVE SENSOR DISPLAY
    // ==================================================

    if (
        currentTime - lastSensorLog >=
        SENSOR_LOG_INTERVAL
    )
    {
        lastSensorLog =
            currentTime;


        // ----------------------------------------------
        // READ SENSOR VALUES
        // ----------------------------------------------

        float accelX =
            getAccelX();

        float accelY =
            getAccelY();

        float accelZ =
            getAccelZ();


        float gyroX =
            getGyroX();

        float gyroY =
            getGyroY();

        float gyroZ =
            getGyroZ();


        bool moving =
            isMoving();


        uint8_t battery =
            getBatteryLevel();


        const char *mode =
            getTrackerModeName();


        // ----------------------------------------------
        // DISPLAY
        // ----------------------------------------------

        Serial.println();

        Serial.println(
            "================================"
        );

        Serial.println(
            "LIVE TRACKER SENSOR DATA"
        );

        Serial.println(
            "================================"
        );


        // ----------------------------------------------
        // BLE
        // ----------------------------------------------

        Serial.print(
            "BLE: "
        );


        if (isBLEConnected())
        {
            Serial.println(
                "CONNECTED"
            );
        }
        else
        {
            Serial.println(
                "DISCONNECTED"
            );
        }


        // ----------------------------------------------
        // TRACKER MODE
        // ----------------------------------------------

        Serial.print(
            "Tracker Mode: "
        );

        Serial.println(
            mode
        );


        // ----------------------------------------------
        // SOS
        // ----------------------------------------------

        Serial.print(
            "SOS: "
        );


        if (isSOSActive())
        {
            Serial.println(
                "ACTIVE"
            );
        }
        else
        {
            Serial.println(
                "INACTIVE"
            );
        }


        // ----------------------------------------------
        // MOTION
        // ----------------------------------------------

        Serial.print(
            "Motion: "
        );


        if (moving)
        {
            Serial.println(
                "MOVING"
            );
        }
        else
        {
            Serial.println(
                "STATIONARY"
            );
        }


        // ----------------------------------------------
        // GPS
        // ----------------------------------------------

        Serial.print(
            "GPS: "
        );


        if (hasGPSFix())
        {
            Serial.println(
                "FIX AVAILABLE"
            );
        }
        else
        {
            Serial.println(
                "NO FIX"
            );
        }


        // ----------------------------------------------
        // ACCELEROMETER
        // ----------------------------------------------

        Serial.println();

        Serial.println(
            "ACCELEROMETER"
        );


        Serial.print(
            "X: "
        );

        Serial.println(
            accelX,
            3
        );


        Serial.print(
            "Y: "
        );

        Serial.println(
            accelY,
            3
        );


        Serial.print(
            "Z: "
        );

        Serial.println(
            accelZ,
            3
        );


        // ----------------------------------------------
        // GYROSCOPE
        // ----------------------------------------------

        Serial.println();

        Serial.println(
            "GYROSCOPE"
        );


        Serial.print(
            "X: "
        );

        Serial.println(
            gyroX,
            3
        );


        Serial.print(
            "Y: "
        );

        Serial.println(
            gyroY,
            3
        );


        Serial.print(
            "Z: "
        );

        Serial.println(
            gyroZ,
            3
        );


        // ----------------------------------------------
        // BATTERY
        // ----------------------------------------------

        Serial.println();

        Serial.print(
            "Battery: "
        );

        Serial.print(
            battery
        );

        Serial.println(
            "%"
        );


        Serial.println(
            "================================"
        );
    }


    // ==================================================
    // GPS FIX AVAILABLE
    // ==================================================

    if (hasGPSFix())
    {
        // ------------------------------------------------
        // SEND COMPLETE TRACKER STATE EVERY 10 SECONDS
        // ------------------------------------------------

        if (
            currentTime - lastLocationSend >=
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
                    getGPSDate(),
                    true
                );


            // ------------------------------------------------
            // RESULT
            // ------------------------------------------------

            Serial.println();


            if (success)
            {
                Serial.println(
                    "GPS LOCATION UPDATE COMPLETED."
                );
            }
            else
            {
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
        // GPS HAS NO FIX
        // ------------------------------------------------

        if (
            currentTime - lastNoFixLog >=
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
                "Logging tracker sensor state"
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
            // LOG SENSOR STATE
            // ------------------------------------------------

            bool success =
                logGPSNoFix(
                    getGPSTime(),
                    getGPSDate()
                );


            // ------------------------------------------------
            // RESULT
            // ------------------------------------------------

            Serial.println();


            if (success)
            {
                Serial.println(
                    "GPS NO_FIX STATUS LOGGED."
                );
            }
            else
            {
                Serial.println(
                    "FAILED TO LOG GPS NO_FIX."
                );
            }
        }
    }


    // ==================================================
    // GSM / OFFLINE HISTORY SYNC
    // ==================================================

    gsmLoop();


    // ==================================================
    // SMALL LOOP DELAY
    // ==================================================

    delay(100);
}