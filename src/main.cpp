#include <Arduino.h>

#include "mpu6050.h"
#include "storage.h"
#include "ble.h"

void setup()
{
    Serial.begin(115200);

    delay(2000);

    Serial.println("Smart Tracker");

    initStorage();

    if (!initMPU())
    {
        Serial.println("MPU Failed");

        while (true);
    }

    initBLE();
}

void loop()
{
    updateMPU();

    updateBLE();

    delay(100);
}