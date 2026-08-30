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
// GSM LOOP
// ======================================================

void gsmLoop();


// ======================================================
// LOCATION
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


// ======================================================
// SOS
// ======================================================

bool sendSOSState(
    bool active,
    float latitude,
    float longitude,
    float altitude,
    String gpsTime,
    String gpsDate,
    bool gpsAvailable
);


// ======================================================
// FIREBASE
// ======================================================

bool sendFirebaseRequest(
    const char *url,
    const String &payload,
    int method
);


// ======================================================
// NETWORK
// ======================================================

bool hasNetworkConnection();


// ======================================================
// OFFLINE SYNC
// ======================================================

void syncOfflineHistory();

#endif