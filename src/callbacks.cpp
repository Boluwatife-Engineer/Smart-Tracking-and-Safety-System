#include "callbacks.h"

#include <Arduino.h>

#include "storage.h"

#include "pins.h"



extern bool deviceConnected;

void ServerCallbacks::onConnect(
    NimBLEServer *,
    NimBLEConnInfo &)
{
    deviceConnected = true;

    Serial.println("BLE Connected");
}

void ServerCallbacks::onDisconnect(
    NimBLEServer *,
    NimBLEConnInfo &,
    int)
{
    deviceConnected = false;

    Serial.println("BLE Disconnected");

    NimBLEDevice::startAdvertising();
}

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