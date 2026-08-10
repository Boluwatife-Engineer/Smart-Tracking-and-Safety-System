#include "ble.h"

#include <Arduino.h>
#include <NimBLEDevice.h>

#include "battery.h"
#include "callbacks.h"
#include "characteristics.h"
#include "mpu6050.h"
#include "pins.h"
#include "uuids.h"
#include "gps.h"

bool deviceConnected = false;

bool isBLEConnected()
{
    return deviceConnected;
}

void initBLE()
{
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    NimBLEDevice::init("Smart Tracker");

    NimBLEServer *server =
        NimBLEDevice::createServer();

    server->setCallbacks(
        new ServerCallbacks());

    NimBLEService *service =
        server->createService(
            SERVICE_UUID);

    createCharacteristics(service);

    service->start();

    NimBLEAdvertising *advertising =
        NimBLEDevice::getAdvertising();

    advertising->addServiceUUID(
        SERVICE_UUID);

    advertising->enableScanResponse(true);

    advertising->setName("Smart Tracker");

    advertising->start();

    Serial.println("Advertising Started");
}

void updateBLE()
{
    if (!deviceConnected)
    {
        return;
    }

    // ================= MOTION =================

    bool moving = isMoving();

    // ================= MPU6050 =================

    float ax = getAccelX();
    float ay = getAccelY();
    float az = getAccelZ();

    float gx = getGyroX();
    float gy = getGyroY();
    float gz = getGyroZ();

    // ================= SERIAL =================

    Serial.printf(
        "Accel: %.2f %.2f %.2f | "
        "Gyro: %.2f %.2f %.2f | "
        "%s | Battery: %d%%\n",

        ax,
        ay,
        az,

        gx,
        gy,
        gz,

        moving
            ? "MOVING"
            : "STATIONARY",

        getBatteryLevel()
    );

    // ================= MOTION =================

    motionChar->setValue(
        moving
            ? "MOVING"
            : "STATIONARY"
    );

    motionChar->notify();

    // ================= ACCELEROMETER =================

    String axValue = String(ax, 2);
    String ayValue = String(ay, 2);
    String azValue = String(az, 2);

    accelXChar->setValue(axValue.c_str());
    accelYChar->setValue(ayValue.c_str());
    accelZChar->setValue(azValue.c_str());

    accelXChar->notify();
    accelYChar->notify();
    accelZChar->notify();

    // ================= GYROSCOPE =================

    String gxValue = String(gx, 2);
    String gyValue = String(gy, 2);
    String gzValue = String(gz, 2);

    gyroXChar->setValue(gxValue.c_str());
    gyroYChar->setValue(gyValue.c_str());
    gyroZChar->setValue(gzValue.c_str());

    gyroXChar->notify();
    gyroYChar->notify();
    gyroZChar->notify();

    // ================= BATTERY =================

    uint8_t level = getBatteryLevel();

    batteryChar->setValue(&level, 1);
    batteryChar->notify();

    // ================= GPS =================

    if (hasGPSFix())
    {
        String latitude =
            String(getLatitude(), 6);

        String longitude =
            String(getLongitude(), 6);

        String altitude =
            String(getAltitude(), 1);

        String gpsTime =
            getGPSTime();

        gpsLatitudeChar->setValue(
            latitude.c_str()
        );

        gpsLongitudeChar->setValue(
            longitude.c_str()
        );

        gpsAltitudeChar->setValue(
            altitude.c_str()
        );

        gpsTimeChar->setValue(
            gpsTime.c_str()
        );

        gpsStatusChar->setValue(
            "FIXED"
        );

        gpsLatitudeChar->notify();
        gpsLongitudeChar->notify();
        gpsAltitudeChar->notify();
        gpsTimeChar->notify();
        gpsStatusChar->notify();
    }
    else
    {
        gpsStatusChar->setValue(
            "SEARCHING"
        );

        gpsStatusChar->notify();
    }
}