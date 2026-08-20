#include "mpu6050.h"

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>


// ======================================================
// MPU6050 OBJECT
// ======================================================

Adafruit_MPU6050 mpu;


// ======================================================
// SENSOR VALUES
// ======================================================

static float accelX = 0.0f;
static float accelY = 0.0f;
static float accelZ = 0.0f;

static float gyroX = 0.0f;
static float gyroY = 0.0f;
static float gyroZ = 0.0f;


// ======================================================
// MOTION THRESHOLDS
// ======================================================

static float accelThreshold = 0.35f;
static float gyroThreshold  = 0.80f;


// ======================================================
// GRAVITY REFERENCE
// ======================================================

static float gravityReference = 9.81f;


// ======================================================
// INITIALIZE MPU6050
// ======================================================

bool initMPU()
{
    Wire.begin();


    // ==================================================
    // START MPU
    // ==================================================

    if (!mpu.begin())
    {
        Serial.println("MPU6050 not found!");

        return false;
    }


    // ==================================================
    // SENSOR CONFIGURATION
    // ==================================================

    mpu.setAccelerometerRange(
        MPU6050_RANGE_4_G
    );

    mpu.setGyroRange(
        MPU6050_RANGE_500_DEG
    );

    mpu.setFilterBandwidth(
        MPU6050_BAND_21_HZ
    );


    // ==================================================
    // CALIBRATION
    // ==================================================

    Serial.println(
        "Calibrating MPU... Keep device still."
    );

    delay(1000);


    float total = 0.0f;


    for (int i = 0; i < 200; i++)
    {
        sensors_event_t accel;
        sensors_event_t gyro;
        sensors_event_t temp;


        mpu.getEvent(
            &accel,
            &gyro,
            &temp
        );


        float magnitude =
            sqrt(
                accel.acceleration.x *
                accel.acceleration.x +

                accel.acceleration.y *
                accel.acceleration.y +

                accel.acceleration.z *
                accel.acceleration.z
            );


        total += magnitude;


        delay(5);
    }


    gravityReference =
        total / 200.0f;


    Serial.print(
        "Gravity Reference: "
    );

    Serial.println(
        gravityReference
    );


    Serial.println(
        "MPU6050 Initialized"
    );


    return true;
}


// ======================================================
// UPDATE MPU
// ======================================================

void updateMPU()
{
    sensors_event_t accel;
    sensors_event_t gyro;
    sensors_event_t temp;


    mpu.getEvent(
        &accel,
        &gyro,
        &temp
    );


    accelX =
        accel.acceleration.x;

    accelY =
        accel.acceleration.y;

    accelZ =
        accel.acceleration.z;


    gyroX =
        gyro.gyro.x;

    gyroY =
        gyro.gyro.y;

    gyroZ =
        gyro.gyro.z;
}


// ======================================================
// SET MOTION THRESHOLDS
// ======================================================

void setMotionThresholds(
    float accelThresh,
    float gyroThresh
)
{
    accelThreshold =
        accelThresh;

    gyroThreshold =
        gyroThresh;
}


// ======================================================
// DETECT MOTION
// ======================================================

bool isMoving()
{
    // ==================================================
    // ACCELERATION MAGNITUDE
    // ==================================================

    float accelMagnitude =
        sqrt(
            accelX * accelX +
            accelY * accelY +
            accelZ * accelZ
        );


    // ==================================================
    // REMOVE GRAVITY
    // ==================================================

    float linearAccel =
        fabs(
            accelMagnitude -
            gravityReference
        );


    // ==================================================
    // GYROSCOPE MAGNITUDE
    // ==================================================

    float gyroMagnitude =
        sqrt(
            gyroX * gyroX +
            gyroY * gyroY +
            gyroZ * gyroZ
        );


    // ==================================================
    // MOTION DECISION
    // ==================================================

    bool moving =
        (linearAccel > accelThreshold) ||
        (gyroMagnitude > gyroThreshold);


    // ==================================================
    // DEBUG
    // ==================================================

    Serial.printf(
        "AccelMag: %.2f | "
        "Linear: %.2f | "
        "GyroMag: %.2f | "
        "Motion: %s\n",

        accelMagnitude,

        linearAccel,

        gyroMagnitude,

        moving
            ? "MOVING"
            : "STATIONARY"
    );


    return moving;
}


// ======================================================
// ACCELEROMETER GETTERS
// ======================================================

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


// ======================================================
// GYROSCOPE GETTERS
// ======================================================

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