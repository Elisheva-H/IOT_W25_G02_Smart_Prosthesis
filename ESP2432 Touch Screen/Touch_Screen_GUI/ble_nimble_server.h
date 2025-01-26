#include "esp32-hal.h"
#include <NimBLEDevice.h>
#include <string>
#include "shared_com_vars.h"
#include "shared_yaml_parser.h"


static NimBLECharacteristic *pCharacteristic;
float temperature = 25.0;
static bool confirmationReceived = false;

// UUIDs for the service and characteristics
#define SERVICE_UUID        "12345678-1234-5678-1234-56789abcdef0"
#define CHARACTERISTIC_UUID "12345678-1234-5678-1234-56789abcdef1"
static NimBLEServer *pServer;

// Class to handle events on connection and discconection from client
class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(BLEServer* pServer, NimBLEConnInfo & 	connInfo) override {
        Serial.println("Client connected!");
    }

    void onDisconnect(BLEServer* pServer, NimBLEConnInfo & 	connInfo, int reason) override {
        Serial.println("Client disconnected! Advertsing again");
        pServer->startAdvertising();
    }
};

// Handle return button - test example
void return_BLE(){
  //uint8_t special_val[] = {0x41, 0xA0, 0x00, 0x00, 0x41, 0xA0, 0x00, 0x00, 0x41, 0xA0, 0x00, 0x00, 0x41, 0xA0, 0x00, 0x01, 0x41, 0xA0, 0x00, 0x01, 0x41, 0xA0, 0x00, 0x01, 0x41, 0xA0, 0x00, 0x01, 0x41, 0xA0, 0x00, 0x00, 0x41, 0xA0, 0x00, 0x00, 0x41, 0xA0, 0x00, 0x00, 0x41, 0xA0, 0x00, 0x01, 0x41, 0xA0, 0x00, 0x01, 0x41, 0xA0, 0x00, 0x01, 0x41, 0xA0, 0x00, 0x56};
  //uint16_t len = 100; 
  uint8_t* msg_bytes = str_to_byte_msg(0,"Hi! How are you today?! Here is a dot . ");
  uint16_t len = sizeof(struct msg_interp);
  print_msg((struct msg_interp*)msg_bytes);
  pCharacteristic->setValue(msg_bytes, len);
  pCharacteristic->notify();
  free(msg_bytes);
  init_yaml();
}


// Callback for receiving confirmations from the client
class MyCallbacks: public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) {
        std::string value = pCharacteristic->getValue();
        //std::string confirmation = value.substr(0, 3);
        //std::string dataValue = value.substr(3);
        Serial.println(String(value.c_str()));
        Serial.println("LOOK HERE");
        // if (confirmation == "ACK" || confirmation == "NAK") {
        //     confirmationReceived = true;
        //     Serial.println("->Confirmation received: " + String(dataValue.c_str()) + "->" + String(confirmation.c_str()));
        //}
    }
};



void Start_BLE_server_NIMBLE(void* params) {
    NimBLEDevice::init("");
    pServer = NimBLEDevice::createServer();
    NimBLEService *pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID,
                        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY
                    );
    pServer->setCallbacks(new ServerCallbacks());
    pCharacteristic->setCallbacks(new MyCallbacks());
    pService->start();
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    
    pAdvertising->setName("UIScreen");
    pAdvertising->addServiceUUID(NimBLEUUID(SERVICE_UUID));
    pAdvertising->start();
   
    Serial.println("Is advertising");
    while (1) {
       
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