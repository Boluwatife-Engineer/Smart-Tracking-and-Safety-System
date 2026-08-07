#include "ble.h"

#include <Arduino.h>
#include <NimBLEDevice.h>

#include "battery.h"
#include "callbacks.h"
#include "characteristics.h"
#include "mpu6050.h"
#include "pins.h"
#include "uuids.h"

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

    bool moving = isMoving();

    float ax = getAccelX();
    float ay = getAccelY();
    float az = getAccelZ();

    float gx = getGyroX();
    float gy = getGyroY();
    float gz = getGyroZ();

    Serial.printf(
        "Accel: %.2f %.2f %.2f | Gyro: %.2f %.2f %.2f | %s | Battery: %d%%\n",
        ax,
        ay,
        az,
        gx,
        gy,
        gz,
        moving ? "MOVING" : "STATIONARY",
        getBatteryLevel());

    // Motion

    motionChar->setValue(
        moving ? "MOVING" : "STATIONARY");

    motionChar->notify();

    // Accelerometer

    accelXChar->setValue(
        String(ax, 2).c_str());

    accelYChar->setValue(
        String(ay, 2).c_str());

    accelZChar->setValue(
        String(az, 2).c_str());

    accelXChar->notify();
    accelYChar->notify();
    accelZChar->notify();

    // Gyroscope

    gyroXChar->setValue(
        String(gx, 2).c_str());

    gyroYChar->setValue(
        String(gy, 2).c_str());

    gyroZChar->setValue(
        String(gz, 2).c_str());

    gyroXChar->notify();
    gyroYChar->notify();
    gyroZChar->notify();

    // Battery

    uint8_t level = getBatteryLevel();

    batteryChar->setValue(&level, 1);

    batteryChar->notify();
}