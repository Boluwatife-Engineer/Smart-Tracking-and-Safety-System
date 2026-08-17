#include <Arduino.h>

#include "mpu6050.h"
#include "storage.h"
#include "ble.h"
#include "battery.h"
#include "gps.h"
#include "gsm.h"
#include "sim7600.h"

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
// SETUP
// ======================================================

void setup()
{
    Serial.begin(115200);

    delay(2000);

    // --------------------------------------------------
    // Storage
    // --------------------------------------------------

    initStorage();

    // --------------------------------------------------
    // Battery
    // --------------------------------------------------

    initBattery();

    // --------------------------------------------------
    // MPU6050
    // --------------------------------------------------

    if (!initMPU())
    {
        Serial.println("MPU Failed");

        while (true)
        {
            delay(1000);
        }
    }

    // --------------------------------------------------
    // SIM7600
    // --------------------------------------------------

    initSIM7600();

    // --------------------------------------------------
    // GPS
    // --------------------------------------------------

    initGPS();

    // --------------------------------------------------
    // GSM
    // --------------------------------------------------

    initGSM();

    // --------------------------------------------------
    // BLE
    // --------------------------------------------------

    initBLE();

    Serial.println();
    Serial.println("=================================");
    Serial.println("SMART TRACKING SYSTEM");
    Serial.println("=================================");
}

// ======================================================
// LOOP
// ======================================================

void loop()
{
    // ==================================================
    // MPU
    // ==================================================

    updateMPU();

    // ==================================================
    // BLE
    // ==================================================

    updateBLE();

    // ==================================================
    // CHECK BLE STATE
    // ==================================================

    bool bleConnected = isBLEConnected();

    // ==================================================
    // BLE CONNECTED
    // ==================================================

    if (bleConnected)
    {
        Serial.println();
        Serial.println("BLE STATUS: CONNECTED");
        Serial.println("Owner is nearby.");
        Serial.println("Device GPS not required.");

        // Reset GPS no-fix timer.
        lastNoFixLog = millis();

        // ------------------------------------------------
        // Do NOT send device GPS while BLE is connected.
        // Smartphone location should be used by dashboard.
        // ------------------------------------------------

        delay(100);

        return;
    }

    // ==================================================
    // BLE DISCONNECTED
    // ==================================================

    Serial.println();
    Serial.println("BLE STATUS: DISCONNECTED");
    Serial.println("Owner is outside BLE range.");
    Serial.println("Checking device GPS...");

    // ==================================================
    // UPDATE DEVICE GPS
    // ==================================================

    updateGPS();

    // ==================================================
    // GPS FIX AVAILABLE
    // ==================================================

    if (hasGPSFix())
    {
        unsigned long currentTime = millis();

        if (
            currentTime - lastLocationSend >=
            LOCATION_SEND_INTERVAL
        )
        {
            lastLocationSend = currentTime;

            Serial.println();
            Serial.println("=================================");
            Serial.println("DEVICE GPS FIX AVAILABLE");
            Serial.println("=================================");

            Serial.print("Latitude: ");
            Serial.println(
                getLatitude(),
                6
            );

            Serial.print("Longitude: ");
            Serial.println(
                getLongitude(),
                6
            );

            Serial.print("Altitude: ");
            Serial.println(
                getAltitude(),
                1
            );

            Serial.print("GPS Date: ");
            Serial.println(
                getGPSDate()
            );

            Serial.print("GPS Time: ");
            Serial.println(
                getGPSTime()
            );

            // IMPORTANT:
            // sendLocation expects:
            // latitude
            // longitude
            // altitude
            // gpsTime
            // gpsDate

            sendLocation(
                getLatitude(),
                getLongitude(),
                getAltitude(),
                getGPSTime(),
                getGPSDate()
            );
        }
    }

    // ==================================================
    // GPS NO FIX
    // ==================================================

    else
    {
        unsigned long currentTime = millis();

        Serial.println();
        Serial.println("=================================");
        Serial.println("DEVICE GPS: NO SATELLITE FIX");
        Serial.println("=================================");

        // Don't create a Firebase record every loop.
        // Log once every 30 seconds.

        if (
            currentTime - lastNoFixLog >=
            NO_FIX_LOG_INTERVAL
        )
        {
            lastNoFixLog = currentTime;

            logGPSNoFix(
                getGPSTime(),
                getGPSDate()
            );
        }
    }

    // ==================================================
    // SMALL LOOP DELAY
    // ==================================================

    delay(100);
}