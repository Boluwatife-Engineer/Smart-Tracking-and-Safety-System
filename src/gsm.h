#ifndef GSM_H
#define GSM_H

#include <Arduino.h>

// ======================================================
// GSM STATE
// ======================================================

extern bool gsmReady;


// ======================================================
// INITIALIZATION
// ======================================================

void initGSM();


// ======================================================
// GSM STATUS
// ======================================================

bool isGSMReady();


// ======================================================
// SEND LOCATION
// ======================================================

bool sendLocation(
    float latitude,
    float longitude,
    float altitude,
    String gpsTime,
    String gpsDate
);


// ======================================================
// LOG GPS NO FIX
// ======================================================

bool logGPSNoFix(
    String gpsTime,
    String gpsDate
);

#endif