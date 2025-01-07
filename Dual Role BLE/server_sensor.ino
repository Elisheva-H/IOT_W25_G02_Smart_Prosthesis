#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID        "c78e29bd-d4e2-4313-a39e-1bf2a5a6e8c2"
#define CHARACTERISTIC_UUID "c78e29bd-d4e2-4313-a39f-1bf2a5a6e8c2"

BLEServer *pServer = nullptr;
BLECharacteristic *pCharacteristic = nullptr;
BLEAdvertising *pAdvertising = nullptr;
bool deviceConnected = false;
bool shouldAdvertise = false;  // This flag ensures advertising restarts in loop()

// Define the custom callback class
class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        Serial.println("Client connected.");
        deviceConnected = true;
    }
    void onDisconnect(BLEServer* pServer) {
        Serial.println("❌ Client disconnected. Restarting advertising...");
        deviceConnected = false;
        shouldAdvertise = true;  // Set flag to restart advertising
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
            } else if (value == "MSG from Prosthesis' MCU") {
                pCharacteristic->setValue("Sensor SERVER recived MSG from client Prosthesis");
            } else {
                pCharacteristic->setValue("Unknown request");
            }
        }
    }
};


void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("Starting BLE work!");

  BLEDevice::init("Sensor Server");
  BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setValue("Hi, I'm Sensor server, you can send me requests");
    pCharacteristic->setCallbacks(new MyCallbacks());

  pService->start();
  //BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pCharacteristic->setCallbacks(new MyCallbacks());
  //BLEDevice::startAdvertising();
    pAdvertising->start();  // Start advertising initially
  // Serial.println("Characteristic defined! Now you can read it in may's computer :-) ");
}

void loop() {
    if ((!deviceConnected) && shouldAdvertise) {
        Serial.println("🔄 Restarting BLE advertising...");
        shouldAdvertise = false;  // Prevent duplicate calls
        BLEDevice::startAdvertising();
    }
    delay(1000);
  
} 