#ifndef MPU6050_H
#define MPU6050_H

#include <Arduino.h>

bool initMPU();

void updateMPU();

bool isMoving();

void setMotionThresholds(
    float accelThreshold,
    float gyroThreshold
);

float getAccelX();
float getAccelY();
float getAccelZ();

float getGyroX();
float getGyroY();
float getGyroZ();

#endif