#include <Arduino.h>
#include <NimBLEDevice.h>
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

#define SERVICE_UUID "6f1f9ea6-76b7-4460-918b-5fa33f709630"

#define TEMP_UUID "bc35d307-d854-44ca-96fd-f5e5e08fd3c4"
#define HUM_UUID  "b6042e4d-e374-4666-8159-aecd1e097b5c"
#define LED_UUID  "dabed4fd-f792-443f-b186-3da384f9d673"

NimBLECharacteristic *tempCharacteristic;
NimBLECharacteristic *humCharacteristic;
NimBLECharacteristic *ledCharacteristic;

class LedCallbacks : public NimBLECharacteristicCallbacks {

    void onWrite(NimBLECharacteristic *pCharacteristic,
                 NimBLEConnInfo &connInfo) override {

        std::string value = pCharacteristic->getValue();

        Serial.print("Received: ");
        Serial.println(value.c_str());

        if (value == "ON") {

            digitalWrite(8, HIGH);
            pCharacteristic->setValue("ON");
            Serial.println("LED ON");
        }

        else if (value == "OFF") {

            digitalWrite(8, LOW);
            pCharacteristic->setValue("OFF");
            Serial.println("LED OFF");
        }
    }
};

void setup() {

    Serial.begin(115200);

    pinMode(8, OUTPUT);
    digitalWrite(8, LOW);

    dht.begin();

    Serial.println("Starting NimBLE...");

    NimBLEDevice::init("My Tracker");

    NimBLEServer *server = NimBLEDevice::createServer();

    NimBLEService *service = server->createService(SERVICE_UUID);

    tempCharacteristic = service->createCharacteristic(
        TEMP_UUID,
        NIMBLE_PROPERTY::READ |
        NIMBLE_PROPERTY::NOTIFY
    );

    humCharacteristic = service->createCharacteristic(
        HUM_UUID,
        NIMBLE_PROPERTY::READ |
        NIMBLE_PROPERTY::NOTIFY
    );

    ledCharacteristic = service->createCharacteristic(
        LED_UUID,
        NIMBLE_PROPERTY::READ |
        NIMBLE_PROPERTY::WRITE
    );

    ledCharacteristic->setCallbacks(new LedCallbacks());
    ledCharacteristic->setValue("OFF");

    // Start the server
    server->start();

    NimBLEAdvertising *advertising = NimBLEDevice::getAdvertising();

    advertising->addServiceUUID(SERVICE_UUID);
    advertising->enableScanResponse(true);

    advertising->start();

    Serial.println("Advertising Started");
}

void loop() {

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h)) {

        tempCharacteristic->setValue(String(t, 1).c_str());
        tempCharacteristic->notify();

        humCharacteristic->setValue(String(h, 1).c_str());
        humCharacteristic->notify();

        Serial.printf("T: %.1f  H: %.1f\n", t, h);
    }

    delay(2000);
}