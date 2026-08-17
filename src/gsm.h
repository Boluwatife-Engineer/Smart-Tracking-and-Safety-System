#ifndef GSM_H
#define GSM_H

#include <Arduino.h>

void initGSM();

bool isGSMReady();

bool sendLocation(
    float latitude,
    float longitude,
    float altitude,
    String gpsTime,
    String gpsDate
);

bool logGPSNoFix(
    String gpsTime,
    String gpsDate
);

#endif