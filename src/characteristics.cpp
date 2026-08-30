#include "characteristics.h"

#include "battery.h"
#include "callbacks.h"
#include "storage.h"
#include "uuids.h"


NimBLECharacteristic *motionChar;

NimBLECharacteristic *lastSeenChar;


NimBLECharacteristic *accelXChar;

NimBLECharacteristic *accelYChar;

NimBLECharacteristic *accelZChar;


NimBLECharacteristic *gyroXChar;

NimBLECharacteristic *gyroYChar;

NimBLECharacteristic *gyroZChar;


NimBLECharacteristic *batteryChar;

NimBLECharacteristic *buzzerChar;


NimBLECharacteristic *gpsLatitudeChar;

NimBLECharacteristic *gpsLongitudeChar;

NimBLECharacteristic *gpsAltitudeChar;

NimBLECharacteristic *gpsTimeChar;

NimBLECharacteristic *gpsStatusChar;


NimBLECharacteristic *sosChar;


// ======================================================
// CREATE CHARACTERISTICS
// ======================================================

void createCharacteristics(
    NimBLEService *service
)
{
    // ==================================================
    // MOTION
    // ==================================================

    motionChar =
        service->createCharacteristic(
            MOTION_UUID,
            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY
        );


    // ==================================================
    // LAST SEEN
    // ==================================================

    lastSeenChar =
        service->createCharacteristic(
            LAST_SEEN_UUID,
            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::WRITE
        );


    lastSeenChar->setCallbacks(
        new LastSeenCallbacks()
    );


    String savedLastSeen =
        loadLastSeen();


    lastSeenChar->setValue(
        savedLastSeen.c_str()
    );


    // ==================================================
    // BATTERY
    // ==================================================

    batteryChar =
        service->createCharacteristic(
            BATTERY_UUID,
            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY
        );


    uint8_t level =
        getBatteryLevel();


    batteryChar->setValue(
        &level,
        1
    );


    // ==================================================
    // ACCELEROMETER
    // ==================================================

    accelXChar =
        service->createCharacteristic(
            ACCEL_X_UUID,
            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY
        );


    accelYChar =
        service->createCharacteristic(
            ACCEL_Y_UUID,
            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY
        );


    accelZChar =
        service->createCharacteristic(
            ACCEL_Z_UUID,
            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY
        );


    // ==================================================
    // GYROSCOPE
    // ==================================================

    gyroXChar =
        service->createCharacteristic(
            GYRO_X_UUID,
            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY
        );


    gyroYChar =
        service->createCharacteristic(
            GYRO_Y_UUID,
            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY
        );


    gyroZChar =
        service->createCharacteristic(
            GYRO_Z_UUID,
            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY
        );


    // ==================================================
    // BUZZER
    // ==================================================

    buzzerChar =
        service->createCharacteristic(
            BUZZER_UUID,
            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::WRITE
        );


    buzzerChar->setCallbacks(
        new BuzzerCallbacks()
    );


    buzzerChar->setValue(
        "OFF"
    );


    // ==================================================
    // GPS LATITUDE
    // ==================================================

    gpsLatitudeChar =
        service->createCharacteristic(
            GPS_LATITUDE_UUID,
            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY
        );


    // ==================================================
    // GPS LONGITUDE
    // ==================================================

    gpsLongitudeChar =
        service->createCharacteristic(
            GPS_LONGITUDE_UUID,
            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY
        );


    // ==================================================
    // GPS ALTITUDE
    // ==================================================

    gpsAltitudeChar =
        service->createCharacteristic(
            GPS_ALTITUDE_UUID,
            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY
        );


    // ==================================================
    // GPS TIME
    // ==================================================

    gpsTimeChar =
        service->createCharacteristic(
            GPS_TIME_UUID,
            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY
        );


    // ==================================================
    // GPS STATUS
    // ==================================================

    gpsStatusChar =
        service->createCharacteristic(
            GPS_STATUS_UUID,
            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY
        );


    // ==================================================
    // SOS
    // ==================================================

    sosChar =
        service->createCharacteristic(
            SOS_UUID,
            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY
        );


    // ==================================================
    // INITIAL GPS VALUES
    // ==================================================

    gpsLatitudeChar->setValue(
        "--"
    );

    gpsLongitudeChar->setValue(
        "--"
    );

    gpsAltitudeChar->setValue(
        "--"
    );

    gpsTimeChar->setValue(
        "--"
    );

    gpsStatusChar->setValue(
        "SEARCHING"
    );


    // ==================================================
    // INITIAL SOS VALUE
    // ==================================================

    sosChar->setValue(
        "INACTIVE"
    );
}