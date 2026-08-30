#include "tracker_state.h"

#include <Arduino.h>

#include "ble.h"
#include "mpu6050.h"
#include "sos.h"


// ======================================================
// CURRENT TRACKER MODE
// ======================================================

static TrackerMode currentMode =
    MODE_BLE_CONNECTED;


// ======================================================
// GET CURRENT MODE
// ======================================================

TrackerMode getTrackerMode()
{
    return currentMode;
}


// ======================================================
// UPDATE TRACKER MODE
// ======================================================

void updateTrackerMode()
{
    // ==================================================
    // SOS HAS HIGHEST PRIORITY
    // ==================================================

    if (isSOSActive())
    {
        currentMode =
            MODE_SOS;

        return;
    }


    // ==================================================
    // BLE CONNECTED
    // ==================================================

    if (isBLEConnected())
    {
        currentMode =
            MODE_BLE_CONNECTED;

        return;
    }


    // ==================================================
    // BLE LOST
    // ==================================================
    //
    // If BLE is lost, determine whether the tracker
    // is moving or stationary.
    //

    if (isMoving())
    {
        currentMode =
            MODE_MOVING;
    }
    else
    {
        currentMode =
            MODE_STATIONARY;
    }
}


// ======================================================
// GET MODE NAME
// ======================================================

const char* getTrackerModeName()
{
    switch (currentMode)
    {
        case MODE_BLE_CONNECTED:
            return "BLE_CONNECTED";


        case MODE_AUTONOMOUS:
            return "AUTONOMOUS";


        case MODE_STATIONARY:
            return "STATIONARY";


        case MODE_MOVING:
            return "MOVING";


        case MODE_SOS:
            return "SOS";


        default:
            return "UNKNOWN";
    }
}