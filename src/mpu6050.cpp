#include "mpu6050.h"

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>

Adafruit_MPU6050 mpu;

static float accelX = 0;
static float accelY = 0;
static float accelZ = 0;

static float gyroX = 0;
static float gyroY = 0;
static float gyroZ = 0;

// Motion thresholds
static float accelThreshold = 0.35f;   // m/s²
static float gyroThreshold  = 0.80f;   // rad/s

// Gravity reference learned during startup
static float gravityReference = 9.81f;

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

    Serial.println("Calibrating MPU... Keep device still.");

    delay(1000);

    float total = 0;

    for(int i = 0; i < 200; i++)
    {
        sensors_event_t accel;
        sensors_event_t gyro;
        sensors_event_t temp;

        mpu.getEvent(&accel, &gyro, &temp);

        float magnitude =
            sqrt(
                accel.acceleration.x * accel.acceleration.x +
                accel.acceleration.y * accel.acceleration.y +
                accel.acceleration.z * accel.acceleration.z
            );

        total += magnitude;

        delay(5);
    }

    gravityReference = total / 200.0f;

    Serial.print("Gravity Reference: ");
    Serial.println(gravityReference);

    Serial.println("MPU6050 Initialized");

    return true;
}

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

void setMotionThresholds(
    float accelThresh,
    float gyroThresh)
{
    accelThreshold = accelThresh;
    gyroThreshold = gyroThresh;
}

bool isMoving()
{
    float accelMagnitude =
        sqrt(
            accelX * accelX +
            accelY * accelY +
            accelZ * accelZ
        );

    float linearAccel =
        fabs(accelMagnitude - gravityReference);

    float gyroMagnitude =
        sqrt(
            gyroX * gyroX +
            gyroY * gyroY +
            gyroZ * gyroZ
        );

    bool moving =
        (linearAccel > accelThreshold) ||
        (gyroMagnitude > gyroThreshold);

    Serial.printf(
        "AccelMag: %.2f  Linear: %.2f  GyroMag: %.2f  -> %s\n",
        accelMagnitude,
        linearAccel,
        gyroMagnitude,
        moving ? "MOVING" : "STATIONARY"
    );

    return moving;
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