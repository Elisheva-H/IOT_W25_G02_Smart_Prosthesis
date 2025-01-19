#include <NimBLEDevice.h>
#include <string>

static NimBLECharacteristic *pCharacteristic;
float temperature = 25.0;
static bool confirmationReceived = false;

// UUIDs for the service and characteristics
#define SERVICE_UUID        "12345678-1234-5678-1234-56789abcdef0"
#define CHARACTERISTIC_UUID "12345678-1234-5678-1234-56789abcdef1"
static NimBLEServer *pServer;
// Callback for receiving confirmations from the client
class MyCallbacks: public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) {
        std::string value = pCharacteristic->getValue();
        std::string confirmation = value.substr(0, 3);
        std::string dataValue = value.substr(3);
        Serial.println(String(confirmation.c_str()));
        Serial.println("LOOK HERE");
        if (confirmation == "ACK" || confirmation == "NAK") {
            confirmationReceived = true;
            Serial.println("->Confirmation received: " + String(dataValue.c_str()) + "->" + String(confirmation.c_str()));
        }
    }
};

void return_BLE(){
  uint8_t special_val[] = {0x41, 0xA0, 0x00, 0x00, 0x41, 0xA0, 0x00, 0x00, 0x41, 0xA0, 0x00, 0x00, 0x41, 0xA0, 0x00, 0x01, 0x41, 0xA0, 0x00, 0x01, 0x41, 0xA0, 0x00, 0x01, 0x41, 0xA0, 0x00, 0x01, 0x41, 0xA0, 0x00, 0x00, 0x41, 0xA0, 0x00, 0x00, 0x41, 0xA0, 0x00, 0x00, 0x41, 0xA0, 0x00, 0x01, 0x41, 0xA0, 0x00, 0x01, 0x41, 0xA0, 0x00, 0x01, 0x41, 0xA0, 0x00, 0x56};
  uint16_t len = 100; 
  pCharacteristic->setValue(special_val, len);
  pCharacteristic->notify();
}

void Start_BLE_server_NIMBLE(void* params) {
    NimBLEDevice::init("Screen_Server");
    pServer = NimBLEDevice::createServer();
    NimBLEService *pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID,
                        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY
                    );
    pCharacteristic->setCallbacks(new MyCallbacks());
    pService->start();
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    
    pAdvertising->setName("Screen_Server");
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->start();
   
    Serial.println("Is advertising");
    while (1) {
        // temperature += 0.1; // Simulate temperature change
        // pCharacteristic->setValue(temperature);
        // pCharacteristic->notify();
        // Serial.print("Send:" + String(temperature));
        // if (!confirmationReceived) {
        //   Serial.println("NO");
        // }
        // else {
        //   Serial.println("YES");
        // }
      if (!confirmationReceived) {
        //Serial.println("NO");
      }
      else {
        Serial.println("YES");
        break;
      }
        delay(2000);
    }
}