
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID        "6c09a8a9-be78-4596-9557-3c4bb4965058"
#define CHARACTERISTIC_UUID "6c09a8a9-be78-4596-9558-3c4bb4965058"

/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/


// See the following for generating UUIDs:
// https://www.uuidgenerator.net/


// Define the custom callback class
class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        Serial.println("Client connected.");
    }
    void onDisconnect(BLEServer* pServer) {
        Serial.println("Client disconnected.");
    }
};


class MyCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) override {
        Serial.println("Entered to onWrite func");
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
            } else if (value == "No") {
                pCharacteristic->setValue("Server is running!");
            } else {
                pCharacteristic->setValue("Unknown request (HaHa I know it was YES)");
            }
        }
    }
};


void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("Starting BLE work!");

  BLEDevice::init("Smart Prosthesis");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setValue("Hello Avigail");
  //pServer->setCallbacks(new MyServerCallbacks());

  pService->start();
  //BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
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
