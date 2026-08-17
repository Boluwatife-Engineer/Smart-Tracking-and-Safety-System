#ifndef GPS_H
#define GPS_H

#include <Arduino.h>

extern HardwareSerial sim7600;

void initGPS();
void updateGPS();

bool hasGPSFix();

float getLatitude();
float getLongitude();
float getAltitude();

String getGPSTime();
String getGPSDate();

#endif