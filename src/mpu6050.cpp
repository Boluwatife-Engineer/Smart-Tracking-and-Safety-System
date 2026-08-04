#include "mpu6050.h"

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;



static float accelX = 0;
static float accelY = 0;
static float accelZ = 0;

static float gyroX = 0;
static float gyroY = 0;
static float gyroZ = 0;



bool initMPU()
{
    Wire.begin();

    if (!mpu.begin())
    {
        Serial.println("MPU6050 not found!");

        return false;
    }

    mpu.setAccelerometerRange(MPU6050_RANGE_4_G);

    mpu.setGyroRange(MPU6050_RANGE_500_DEG);

    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

    Serial.println("MPU6050 Initialized");

    return true;
}


// Read Sensor


void updateMPU()
{
    sensors_event_t accel;
    sensors_event_t gyro;
    sensors_event_t temp;

    mpu.getEvent(&accel, &gyro, &temp);

    accelX = accel.acceleration.x;
    accelY = accel.acceleration.y;
    accelZ = accel.acceleration.z;

    gyroX = gyro.gyro.x;
    gyroY = gyro.gyro.y;
    gyroZ = gyro.gyro.z;
}



float getAccelX()
{
    return accelX;
}

float getAccelY()
{
    return accelY;
}

float getAccelZ()
{
    return accelZ;
}

float getGyroX()
{
    return gyroX;
}

float getGyroY()
{
    return gyroY;
}

float getGyroZ()
{
    return gyroZ;
}