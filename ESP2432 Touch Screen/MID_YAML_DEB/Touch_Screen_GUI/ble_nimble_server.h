#include "HardwareSerial.h"
#ifndef BLE_NIMBLE_SERVER_H
#define BLE_NIMBLE_SERVER_H

#include "esp32-hal.h"
#include <NimBLEDevice.h>
#include <string>
#include "shared_com_vars.h"
#include "shared_yaml_parser.h"
#include "requests.h"


static NimBLECharacteristic *pCharacteristic;
static bool confirmationReceived = false;

static uint8_t** buffer_p;

// Initialize YAML flags
static bool is_yml_general_ready = false;
static bool is_yml_sensors_ready = false;
static bool is_yml_motors_ready = false;
static bool is_yml_functions_ready = false;

// UUIDs for the service and characteristics
#define SERVICE_UUID        "12345678-1234-5678-1234-56789abcdef3"
#define CHARACTERISTIC_UUID "12345678-1234-5678-1234-56789abcdef4"
static NimBLEServer *pServer;

#define MSG_SIZE (sizeof(struct msg_interp))

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
            buffer_p = &sensors_yaml_buffer;
            Serial.printf("buffer p is %d\n, buffer hold is %d",buffer_p,*buffer_p);
            ReciveYAMLField(buffer_p,*struct_val);
            Serial.printf("buffer p is %d\n, buffer hold is %d",buffer_p,*buffer_p);

            if (struct_val->tot_msg_count==struct_val->cur_msg_count){
              Serial.println("inside if");
              Serial.printf("\n#1:  %s\n",(char*)(*buffer_p));
              //////////////////////////////Dont forget to process!!!///////////

              //processYAMLField(YML_SENSOR_ANS ,*buffer_p);
              // Free the buffer 
              //////////////////////////////Dont forget to free!!!///////////
              //free(*buffer_p);
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