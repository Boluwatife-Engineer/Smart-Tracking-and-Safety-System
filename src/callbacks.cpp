#include "callbacks.h"

#include <Arduino.h>

#include "storage.h"
#include "pins.h"


// ======================================================
// EXTERNAL BLE STATE
// ======================================================

extern bool deviceConnected;


// ======================================================
// BLE DISCONNECT EVENT
// ======================================================
//
// This flag is intentionally processed outside the
// NimBLE callback.
//
// Do NOT perform Firebase HTTP requests here.
//

volatile bool bleDisconnectPending = false;


// ======================================================
// SERVER CALLBACKS
// ======================================================

void ServerCallbacks::onConnect(
    NimBLEServer *,
    NimBLEConnInfo &)
{
    deviceConnected = true;

    Serial.println("BLE Connected");
}


// ======================================================
// BLE DISCONNECT
// ======================================================

void ServerCallbacks::onDisconnect(
    NimBLEServer *,
    NimBLEConnInfo &,
    int)
{
    deviceConnected = false;

    // Tell the normal application loop that BLE just
    // disconnected and the last known GPS location
    // needs to be captured.
    bleDisconnectPending = true;

    Serial.println("BLE Disconnected");
    Serial.println("Last Seen capture pending.");

    NimBLEDevice::startAdvertising();
}


// ======================================================
// BUZZER
// ======================================================

void BuzzerCallbacks::onWrite(
    NimBLECharacteristic *characteristic,
    NimBLEConnInfo &)
{
    std::string value = characteristic->getValue();

    if (value == "ON")
    {
        digitalWrite(BUZZER_PIN, HIGH);

        characteristic->setValue("ON");

        Serial.println("Buzzer ON");
    }
    else if (value == "OFF")
    {
        digitalWrite(BUZZER_PIN, LOW);

        characteristic->setValue("OFF");

        Serial.println("Buzzer OFF");
    }
}


// ======================================================
// LAST SEEN BLE WRITE
// ======================================================
//
// This is kept for compatibility with your existing
// BLE characteristic.
//
// Automatic Last Seen capture now happens when BLE
// disconnects.
//

void LastSeenCallbacks::onWrite(
    NimBLECharacteristic *characteristic,
    NimBLEConnInfo &)
{
    std::string value = characteristic->getValue();

    characteristic->setValue(value);

    saveLastSeen(String(value.c_str()));

    Serial.println("Last Seen Updated:");
    Serial.println(value.c_str());
}