#ifndef MPU6050_H
#define MPU6050_H

#include <Arduino.h>

bool initMPU();

void updateMPU();

float getAccelX();
float getAccelY();
float getAccelZ();

float getGyroX();
float getGyroY();
float getGyroZ();

#endif