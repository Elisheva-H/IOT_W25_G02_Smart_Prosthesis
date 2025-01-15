#include <NimBLEDevice.h>
#include <string>
#include <atomic>

SemaphoreHandle_t xMutex;
NimBLECharacteristic *pCharacteristic;
float temperature = 25.0;
std::atomic<bool> confirmationReceived(false);

// UUIDs for the service and characteristics
#define SERVICE_UUID        "12345678-1234-5678-1234-56789abcdef0"
#define CHARACTERISTIC_UUID "12345678-1234-5678-1234-56789abcdef1"

NimBLEServer *pServer;

// Callback for receiving confirmations from the client
class MyCallbacks: public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) {
        if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {
          Serial.println("LOOK HERE ON WRITE");
          std::string value = pCharacteristic->getValue();
          std::string confirmation = value.substr(0, 3);
          std::string dataValue = value.substr(3);
          Serial.println(String(confirmation.c_str()));
          
          if (confirmation == "ACK" || confirmation == "NAK") {
              // confirmationReceived = true;
              
              confirmationReceived.store(true, std::memory_order_release); 
            }
              Serial.println("->Confirmation received: " + String(dataValue.c_str()) + "->" + String(confirmation.c_str()));
          }
        xSemaphoreGive(xMutex);

    }
};

void return_BLE(){
  float special_val = 20.0;
  pCharacteristic->setValue(special_val);
  pCharacteristic->notify();

}

void Start_BLE_server_NIMBLE(void* params) {
    Serial.begin(115200);
    delay(500);


    NimBLEDevice::init("ESP32_Touch_screen_Server");
    pServer = NimBLEDevice::createServer();

    NimBLEService *pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID,
                        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::INDICATE  
                    );

    pCharacteristic->setCallbacks(new MyCallbacks());
    pService->start();

    NimBLEAdvertising *pAdvertising = pServer->getAdvertising();

    pAdvertising->start();
    while (1) {
        // temperature += 0.1; // Simulate temperature change
        // pCharacteristic->setValue(temperature);
        // pCharacteristic->notify();
        // Serial.print("Send:" + String(temperature));
        // if (!confirmationReceived) {
        //   // Serial.println("NO");
        // }
        // else {
        //   Serial.println("YES");
        // }
        delay(2000);
    }
}

