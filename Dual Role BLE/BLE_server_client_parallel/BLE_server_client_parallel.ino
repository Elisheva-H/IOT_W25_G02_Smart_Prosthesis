#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include "client_header.h"
#include "srever_header.h"

#define STACK_SIZE 8192
//Heavilly based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp, Evandro Copercini and chegewara

void setup() {
  Serial.begin(115200);
  delay(1500);
  //xTaskCreate(BLE_server_start,"Server", STACK_SIZE, nullptr, 1, nullptr);
  //xTaskCreate(Start_BLE_client,"Client", STACK_SIZE, nullptr, 2, nullptr);
  xTaskCreatePinnedToCore(BLE_server_start,"Server", STACK_SIZE, nullptr, 1, nullptr ,1);
  xTaskCreatePinnedToCore(Start_BLE_client,"Client", STACK_SIZE, nullptr, 2, nullptr,0);
}

void loop() {
  delay(2000);
}

