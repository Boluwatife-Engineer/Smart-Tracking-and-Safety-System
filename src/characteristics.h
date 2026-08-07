#ifndef CHARACTERISTICS_H
#define CHARACTERISTICS_H

#include <NimBLEDevice.h>

extern NimBLECharacteristic *motionChar;
extern NimBLECharacteristic *lastSeenChar;

extern NimBLECharacteristic *accelXChar;
extern NimBLECharacteristic *accelYChar;
extern NimBLECharacteristic *accelZChar;

extern NimBLECharacteristic *gyroXChar;
extern NimBLECharacteristic *gyroYChar;
extern NimBLECharacteristic *gyroZChar;

extern NimBLECharacteristic *batteryChar;
extern NimBLECharacteristic *buzzerChar;

void createCharacteristics(NimBLEService *service);

#endif