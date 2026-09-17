#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <NimBLEDevice.h>

// ======================================================
// BLE DISCONNECT EVENT
// ======================================================
//
// Set to true by onDisconnect().
// Processed later from the normal main loop through gsmLoop().
//
// Do NOT perform Firebase/network operations directly
// inside the NimBLE callback.
//

extern volatile bool bleDisconnectPending;


// ======================================================
// SERVER CALLBACKS
// ======================================================

class ServerCallbacks : public NimBLEServerCallbacks
{
public:

    void onConnect(
        NimBLEServer *server,
        NimBLEConnInfo &connInfo
    ) override;

    void onDisconnect(
        NimBLEServer *server,
        NimBLEConnInfo &connInfo,
        int reason
    ) override;
};


// ======================================================
// BUZZER CALLBACKS
// ======================================================

class BuzzerCallbacks : public NimBLECharacteristicCallbacks
{
public:

    void onWrite(
        NimBLECharacteristic *characteristic,
        NimBLEConnInfo &connInfo
    ) override;
};


// ======================================================
// LAST SEEN CALLBACKS
// ======================================================

class LastSeenCallbacks : public NimBLECharacteristicCallbacks
{
public:

    void onWrite(
        NimBLECharacteristic *characteristic,
        NimBLEConnInfo &connInfo
    ) override;
};

#endif