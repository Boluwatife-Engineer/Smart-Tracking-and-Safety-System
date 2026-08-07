#include <Arduino.h>

#include "mpu6050.h"
#include "storage.h"
#include "ble.h"
#include "battery.h"

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

    initBLE();

    Serial.println("Smart Tracker");
}

void loop()
{
    updateMPU();

    updateBLE();

    delay(100);
}