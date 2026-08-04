#include <Arduino.h>
#include <NimBLEDevice.h>

#include "mpu6050.h"

#define LED_PIN 4

//UUIDS

#define SERVICE_UUID "6f1f9ea6-76b7-4460-918b-5fa33f709630"

#define MOTION_UUID "11111111-1111-1111-1111-111111111111"

#define ACCEL_X_UUID "22222222-2222-2222-2222-222222222221"
#define ACCEL_Y_UUID "22222222-2222-2222-2222-222222222222"
#define ACCEL_Z_UUID "22222222-2222-2222-2222-222222222223"

#define GYRO_X_UUID "33333333-3333-3333-3333-333333333331"
#define GYRO_Y_UUID "33333333-3333-3333-3333-333333333332"
#define GYRO_Z_UUID "33333333-3333-3333-3333-333333333333"

#define LED_UUID "dabed4fd-f792-443f-b186-3da384f9d673"



bool deviceConnected = false;

NimBLECharacteristic *motionChar;

NimBLECharacteristic *accelXChar;
NimBLECharacteristic *accelYChar;
NimBLECharacteristic *accelZChar;

NimBLECharacteristic *gyroXChar;
NimBLECharacteristic *gyroYChar;
NimBLECharacteristic *gyroZChar;

NimBLECharacteristic *ledChar;



class ServerCallbacks : public NimBLEServerCallbacks {

    void onConnect(NimBLEServer*, NimBLEConnInfo&) override {

        deviceConnected = true;

        Serial.println("BLE Connected");

    }

    void onDisconnect(
        NimBLEServer*,
        NimBLEConnInfo&,
        int
    ) override {

        deviceConnected = false;

        Serial.println("BLE Disconnected");

        NimBLEDevice::startAdvertising();

    }

};



class LedCallbacks : public NimBLECharacteristicCallbacks {

    void onWrite(
        NimBLECharacteristic *characteristic,
        NimBLEConnInfo&
    ) override {

        std::string value = characteristic->getValue();

        if(value == "ON")
        {
            digitalWrite(LED_PIN, HIGH);

            characteristic->setValue("ON");

            Serial.println("LED ON");
        }

        else if(value == "OFF")
        {
            digitalWrite(LED_PIN, LOW);

            characteristic->setValue("OFF");

            Serial.println("LED OFF");
        }

    }

};

void setup()
{

    Serial.begin(115200);

    delay(2000);

    pinMode(LED_PIN,OUTPUT);

    digitalWrite(LED_PIN,LOW);

    Serial.println("Smart Tracker");

   

    if(!initMPU())
    {
        Serial.println("MPU Failed");

        while(true);
    }

   

    NimBLEDevice::init("Smart Tracker");

    NimBLEServer *server =
        NimBLEDevice::createServer();

    server->setCallbacks(new ServerCallbacks());

    NimBLEService *service =
        server->createService(SERVICE_UUID);

    //MOTION (MPU6050)

    motionChar =
        service->createCharacteristic(

            MOTION_UUID,

            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY

        );

   //ACCELEROMETER

    accelXChar =
        service->createCharacteristic(

            ACCEL_X_UUID,

            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY

        );

    accelYChar =
        service->createCharacteristic(

            ACCEL_Y_UUID,

            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY

        );

    accelZChar =
        service->createCharacteristic(

            ACCEL_Z_UUID,

            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY

        );

    //GYROSCOPE

    gyroXChar =
        service->createCharacteristic(

            GYRO_X_UUID,

            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY

        );

    gyroYChar =
        service->createCharacteristic(

            GYRO_Y_UUID,

            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY

        );

    gyroZChar =
        service->createCharacteristic(

            GYRO_Z_UUID,

            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY

        );


    //LED

    ledChar =
        service->createCharacteristic(

            LED_UUID,

            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::WRITE

        );

    ledChar->setCallbacks(new LedCallbacks());

    ledChar->setValue("OFF");



    service->start();

    NimBLEAdvertising *advertising =
        NimBLEDevice::getAdvertising();

    advertising->addServiceUUID(SERVICE_UUID);

    advertising->enableScanResponse(true);

    advertising->setName("Smart Tracker");

    advertising->start();

    Serial.println("Advertising Started");

}



void loop()
{

    updateMPU();

    float ax = getAccelX();
    float ay = getAccelY();
    float az = getAccelZ();

    float gx = getGyroX();
    float gy = getGyroY();
    float gz = getGyroZ();

    //MOTION DETECTION

    bool moving =

        abs(gx) > 5 ||

        abs(gy) > 5 ||

        abs(gz) > 5;

    

    Serial.printf(

        "Motion: %s\n",

        moving ? "MOVING" : "STATIONARY"

    );

    

    if(deviceConnected)
    {

        motionChar->setValue(

            moving ?

            "MOVING" :

            "STATIONARY"

        );

        motionChar->notify();

        accelXChar->setValue(String(ax,2).c_str());
        accelYChar->setValue(String(ay,2).c_str());
        accelZChar->setValue(String(az,2).c_str());

        gyroXChar->setValue(String(gx,2).c_str());
        gyroYChar->setValue(String(gy,2).c_str());
        gyroZChar->setValue(String(gz,2).c_str());

        accelXChar->notify();
        accelYChar->notify();
        accelZChar->notify();

        gyroXChar->notify();
        gyroYChar->notify();
        gyroZChar->notify();

    }

    delay(200);

}