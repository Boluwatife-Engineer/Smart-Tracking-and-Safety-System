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

extern NimBLECharacteristic *gpsLatitudeChar;
extern NimBLECharacteristic *gpsLongitudeChar;
extern NimBLECharacteristic *gpsAltitudeChar;
extern NimBLECharacteristic *gpsTimeChar;
extern NimBLECharacteristic *gpsStatusChar;

void createCharacteristics(NimBLEService *service);

#endif