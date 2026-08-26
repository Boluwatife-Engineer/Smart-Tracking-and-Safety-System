#ifndef GSM_H
#define GSM_H

#include <Arduino.h>

// ======================================================
// GSM
// ======================================================

void initGSM();

bool isGSMReady();


// ======================================================
// FIREBASE
// ======================================================

bool sendLocation(
    float latitude,
    float longitude,
    float altitude,
    String gpsTime,
    String gpsDate,
    bool gpsAvailable
);


// ======================================================
// GPS NO FIX
// ======================================================

bool logGPSNoFix(
    String gpsTime,
    String gpsDate
);

#endif