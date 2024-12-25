
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID        "6c09a8a9-be78-4596-9557-3c4bb4965058"
#define CHARACTERISTIC_UUID "6c09a8a9-be78-4596-9558-3c4bb4965058"

std::string value;


/*
BLECharacteristic *pCharacteristic;

class MyCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) override {
        std::string value = pCharacteristic->getValue();
        if (value.length() > 0) {
            Serial.println("Received request from client:");
            for (int i = 0; i < value.length(); i++) {
                Serial.print(value[i]);
            }
            Serial.println();

            // Respond based on the request
            if (value == "hello") {
                pCharacteristic->setValue("Hi from server!");
            } else if (value == "status") {
                pCharacteristic->setValue("Server is running!");
            } else {
                pCharacteristic->setValue("Unknown request");
            }
        }
    }
};

void setup() {
    Serial.begin(115200);
    delay(3000);
    // Initialize BLE
    BLEDevice::init("ESP32 BLE Server");
    BLEServer *pServer = BLEDevice::createServer();

    // Create service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create characteristic
    pCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID,
                        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
                     );

    pCharacteristic->setCallbacks(new MyCallbacks());

    // Start the service
    pService->start();

    // Start advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->start();

    Serial.println("BLE server is ready!");
}

void loop() {
    // Nothing to do here, server runs on BLE callbacks
}
*/
/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/


// See the following for generating UUIDs:
// https://www.uuidgenerator.net/


class MyCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) override {
        std::string value = pCharacteristic->getValue();
        if (value.length() > 0) {
            Serial.println("Received request from client:");
            for (int i = 0; i < value.length(); i++) {
                Serial.print(value[i]);
            }
            Serial.println();

            // Respond based on the request
            if (value == "Option1") {
                pCharacteristic->setValue("Server recived Option1 from you");
            } else if (value == "status") {
                pCharacteristic->setValue("Server is running!");
            } else {
                pCharacteristic->setValue("Unknown request");
            }
        }
    }
};


void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Starting BLE work!");

  BLEDevice::init("Long name works now");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setValue("Hello Avigail");
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pCharacteristic->setCallbacks(new MyCallbacks());
  //BLEDevice::startAdvertising();
  pAdvertising->start();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(2000);
} 
