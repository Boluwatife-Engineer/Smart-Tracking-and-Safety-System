#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

#include <NimBLEDevice.h>
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT22

#define LED_PIN 8

DHT dht(DHTPIN, DHTTYPE);

AsyncWebServer server(80);

//WiFi 

const char* ssid = "TIFEH100";
const char* password = "Doyouknowlade^";

//BLE UUID

#define SERVICE_UUID "6f1f9ea6-76b7-4460-918b-5fa33f709630"

#define TEMP_UUID "bc35d307-d854-44ca-96fd-f5e5e08fd3c4"
#define HUM_UUID  "b6042e4d-e374-4666-8159-aecd1e097b5c"
#define LED_UUID  "dabed4fd-f792-443f-b186-3da384f9d673"


NimBLECharacteristic *tempCharacteristic;
NimBLECharacteristic *humCharacteristic;
NimBLECharacteristic *ledCharacteristic;

bool deviceConnected = false;

//BLE SERVER CALLBACK
class ServerCallbacks : public NimBLEServerCallbacks {

    void onConnect(NimBLEServer*,
                   NimBLEConnInfo&) override {

        deviceConnected = true;

        Serial.println("BLE Connected");
    }

    void onDisconnect(NimBLEServer*,
                      NimBLEConnInfo&,
                      int) override {

        deviceConnected = false;

        Serial.println("BLE Disconnected");

        NimBLEDevice::startAdvertising();
    }
};

//LED CALLBACK

class LedCallbacks : public NimBLECharacteristicCallbacks {

    void onWrite(NimBLECharacteristic *characteristic,
                 NimBLEConnInfo&) override {

        std::string value = characteristic->getValue();

        Serial.print("Received: ");
        Serial.println(value.c_str());

        if (value == "ON") {

            digitalWrite(LED_PIN, LOW);

            characteristic->setValue("ON");

            Serial.println("LED ON");
        }

        else if (value == "OFF") {

            digitalWrite(LED_PIN, HIGH);

            characteristic->setValue("OFF");

            Serial.println("LED OFF");
        }
    }
};



void setup() {

    Serial.begin(115200);

    delay(3000);

    Serial.println("SETUP STARTED");


    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    dht.begin();

    //LITTLEFS

    if (!LittleFS.begin()) {

        Serial.println("LittleFS Mount Failed");

        while (true);

    }

    Serial.println("LittleFS Mounted");

    
    // WiFi Station
    

    WiFi.mode(WIFI_STA);

    WiFi.begin(ssid, password);

    Serial.print("Connecting to WiFi");

    while (WiFi.status() != WL_CONNECTED) {

        delay(500);
        Serial.print(".");

    }

    Serial.println();
    Serial.println("--------------------------------");
    Serial.println("WiFi Connected");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.println("--------------------------------");

   
    // Web Server
   

    server.serveStatic("/", LittleFS, "/")
          .setDefaultFile("index.html");

    server.begin();

    Serial.println("Web Server Started");

    
    // BLE
   

    NimBLEDevice::init("My Tracker");

    NimBLEServer *bleServer = NimBLEDevice::createServer();

    bleServer->setCallbacks(new ServerCallbacks());

    NimBLEService *service =
        bleServer->createService(SERVICE_UUID);

    tempCharacteristic =
        service->createCharacteristic(

            TEMP_UUID,

            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY
        );

    humCharacteristic =
        service->createCharacteristic(

            HUM_UUID,

            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY
        );

    ledCharacteristic =
        service->createCharacteristic(

            LED_UUID,

            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::WRITE
        );

    ledCharacteristic->setCallbacks(new LedCallbacks());

    ledCharacteristic->setValue("OFF");

    bleServer->start();

    NimBLEAdvertising *advertising =
        NimBLEDevice::getAdvertising();

    advertising->addServiceUUID(SERVICE_UUID);

    advertising->enableScanResponse(true);

    advertising->setName("My Tracker");

    advertising->start();

    Serial.println("BLE Advertising Started");
}



void loop() {

    float temperature = dht.readTemperature();

    float humidity = dht.readHumidity();

    if (!isnan(temperature) && !isnan(humidity)) {

        Serial.printf(
            "T: %.1f°C   H: %.1f%%\n",
            temperature,
            humidity
        );

        if (deviceConnected) {

            tempCharacteristic->setValue(
                String(temperature, 1).c_str());

            tempCharacteristic->notify();

            humCharacteristic->setValue(
                String(humidity, 1).c_str());

            humCharacteristic->notify();

        }

    }

    delay(2000);

}