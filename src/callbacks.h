#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <NimBLEDevice.h>

class ServerCallbacks : public NimBLEServerCallbacks
{
public:
    void onConnect(NimBLEServer *, NimBLEConnInfo &) override;

    void onDisconnect(
        NimBLEServer *,
        NimBLEConnInfo &,
        int) override;
};

class BuzzerCallbacks : public NimBLECharacteristicCallbacks
{
public:
    void onWrite(
        NimBLECharacteristic *,
        NimBLEConnInfo &) override;
};

class LastSeenCallbacks : public NimBLECharacteristicCallbacks
{
public:
    void onWrite(
        NimBLECharacteristic *,
        NimBLEConnInfo &) override;
};

#endif