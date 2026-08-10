#include <Arduino.h>

#include "mpu6050.h"
#include "storage.h"
#include "ble.h"
#include "battery.h"
#include "gps.h"

void setup()
{
    Serial.begin(115200);

    delay(2000);

    initStorage();
    initBattery();

    if (!initMPU())
    {
        Serial.println("MPU Failed");

        while (true);
    }

    // Initialize SIM7600 GPS
    initGPS();

    // Initialize BLE
    initBLE();

    Serial.println("Smart Tracker");
}

void loop()
{
    updateMPU();

    // Update GPS
    updateGPS();

    // Update BLE
    updateBLE();

    delay(100);
}