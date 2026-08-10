#ifndef GPS_H
#define GPS_H

#include <Arduino.h>

void initGPS();
void updateGPS();

bool hasGPSFix();

float getLatitude();
float getLongitude();
float getAltitude();

String getGPSTime();
String getGPSDate();

#endif