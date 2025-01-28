#include "HardwareSerial.h"
#ifndef BLE_NIMBLE_SERVER_H
#define BLE_NIMBLE_SERVER_H

#include "esp32-hal.h"
#include <NimBLEDevice.h>
#include <string>
#include "shared_com_vars.h"
#include "shared_yaml_parser.h"
#include "requests.h"
#include <atomic>


static NimBLECharacteristic *pCharacteristic;


static bool confirmationReceived = false;
std::atomic_flag has_client = ATOMIC_FLAG_INIT;
std::atomic_flag can_play_gesture = ATOMIC_FLAG_INIT;
static uint8_t** pointer_to_sensor_buff;



// UUIDs for the service and characteristics
#define SERVICE_UUID        "12345678-1234-5678-1234-56789abcdef3"
#define CHARACTERISTIC_UUID "12345678-1234-5678-1234-56789abcdef4"
#define MSG_SIZE (sizeof(struct msg_interp))
static NimBLEServer *pServer;

// Class to handle events on connection and discconection from client
class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(BLEServer* pServer, NimBLEConnInfo & 	connInfo) override {
        Serial.println("Client connected!");
        has_client.test_and_set();
    }

    void onDisconnect(BLEServer* pServer, NimBLEConnInfo & 	connInfo, int reason) override {
        Serial.println("Client disconnected! Advertsing again");
        pServer->startAdvertising();
    }
};

// Handle return button - test example
void return_BLE(){
  char* msg_str="Give YAML";
  int total_msg_num = ceil(((float)strlen(msg_str))/((float)(MAX_MSG_LEN-1)));
  size_t total_msg_len = strlen(msg_str);
  if (total_msg_num>1){Serial.println("The message is too long, dividing into multiple sends");}
  for (int msg_num=1;msg_num<=total_msg_num;msg_num++){
    uint8_t* msg_bytes = str_to_byte_msg(YAML_REQ,msg_str,msg_num, total_msg_num);
    uint16_t len = sizeof(struct msg_interp);
    print_msg((struct msg_interp*)msg_bytes);
    pCharacteristic->setValue(msg_bytes, len);
    pCharacteristic->notify();
      //TO DO- error handling
    free(msg_bytes);

  }
}




// Callback for receiving confirmations from the client
class MyCallbacks: public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) {
    const uint8_t* value = pCharacteristic->getValue().data();
    //print_byte_array(MSG_SIZE, value); // print byte array for debuging
    struct msg_interp* struct_val = (struct msg_interp*)value;
    //handle client response based on the request
    switch (struct_val->req_type) {
      case READ_REQ:
            // Add handling for READ_REQ here
          break;
      case EDIT_REQ:
          // Add handling for EDIT_REQ here
          break;
      case FUNC_REQ:
          // Add handling for FUNC_REQ here
          break;
      case YAML_REQ:
          // Add handling for YAML_REQ here
          break;
      case GEST_REQ:
          // Add handling for GEST_REQ here
          break;
      case READ_ANS:
          // Add handling for READ_ANS here
          break;
      case EDIT_ANS:
          // Add handling for EDIT_ANS here
          break;
      case FUNC_ANS:
          // Add handling for FUNC_ANS here
          break;
      case YML_SENSOR_ANS:
          //Check if the msg was recived succesfully by comparing the msg length and checksum to the desired values 
            pointer_to_sensor_buff = &sensors_yaml_buffer;
            ReciveYAMLField(pointer_to_sensor_buff,*struct_val);
            if (struct_val->tot_msg_count==struct_val->cur_msg_count){
              //////////////////////////////Dont forget to process!!!///////////

              //processYAMLField(YML_SENSOR_ANS ,*pointer_to_sensor_buff);
              // Free the buffer 
              //////////////////////////////Dont forget to free!!!///////////
              //free(*pointer_to_sensor_buff);
              //sensors_yaml_buffer = NULL;  
          }
          break;

      // case YML_MOTORS_ANS:
      //     ReciveYAMLField(motors_yaml_buffer,struct_val);
      //     processYAMLField(YML_MOTORS_ANS ,motors_yaml_buffer);

          
      //     // Free the buffer
      //     free(motors_yaml_buffer);
      //     motors_yaml_buffer = NULL;  
          
      //     break;

      // case YML_FUNC_ANS:
      //     ReciveYAMLField(funcs_yaml_buffer,struct_val);
      //     processYAMLField(YML_FUNC_ANS ,funcs_yaml_buffer);
      //     // Free the buffer 
      //     free(funcs_yaml_buffer);
      //     funcs_yaml_buffer = NULL;  
          
      //     break;

      // case YML_GENERAL_ANS:
      //     ReciveYAMLField(general_yaml_buffer,struct_val);
      //     processYAMLField(YML_GENERAL_ANS,general_yaml_buffer);
      //     // Free the buffer 
      //     free(general_yaml_buffer);
      //     general_yaml_buffer = NULL;  
      
      //     break;
      case YAML_ANS:
        
        break;
      case GEST_ANS:
          // Add handling for GEST_ANS here
          break;
    default:
        break;
    }
  }
};

// void return_BLE(){
//  uint8_t special_val[] = {0x41, 0xA0, 0x00, 0x00, 0x41, 0xA0, 0x00, 0x00, 0x41, 0xA0, 0x00, 0x00, 0x41, 0xA0, 0x00, 0x01, 0x41, 0xA0, 0x00, 0x01, 0x41, 0xA0, 0x00, 0x01, 0x41, 0xA0, 0x00, 0x01, 0x41, 0xA0, 0x00, 0x00, 0x41, 0xA0, 0x00, 0x00, 0x41, 0xA0, 0x00, 0x00, 0x41, 0xA0, 0x00, 0x01, 0x41, 0xA0, 0x00, 0x01, 0x41, 0xA0, 0x00, 0x01, 0x41, 0xA0, 0x00, 0x56};
//  uint16_t len = 100; 
//  pCharacteristic->setValue(special_val, len);
//  pCharacteristic->notify();
// }


void sending_gesture(char* gesture_name){
  uint8_t* byte_msg = str_to_byte_msg(GEST_REQ,gesture_name);
  print_byte_array(MSG_SIZE,byte_msg);
  uint16_t len = MSG_SIZE; 
  pCharacteristic->setValue(byte_msg, len);
  pCharacteristic->notify();
}


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

#endif //BLE_NIMBLE_SERVER_H
