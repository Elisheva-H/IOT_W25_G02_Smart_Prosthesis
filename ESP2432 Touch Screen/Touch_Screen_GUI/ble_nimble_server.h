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
static bool send_yaml_request = false;
static bool welcome_screen_flag = true;
std::atomic_flag has_client = ATOMIC_FLAG_INIT;
std::atomic_flag can_play_gesture = ATOMIC_FLAG_INIT;
std::atomic_flag is_demo_yaml = ATOMIC_FLAG_INIT;


lv_obj_t* home_tab = NULL;
lv_obj_t* stat_tab = NULL;
lv_obj_t* setup_tab = NULL;
lv_obj_t *welcome_screen  = NULL; //



// static uint8_t** pointer_to_sensor_buff;
// static uint8_t** pointer_to_motors_buff;
// static uint8_t** pointer_to_func_buff;
// static uint8_t** pointer_to_general_buff;


// UUIDs for the service and characteristics
#define SERVICE_UUID        "12345678-1234-5678-1234-56789abcdef0"
#define CHARACTERISTIC_UUID "12345678-1234-5678-1234-56789abcdef1"
#define MSG_SIZE (sizeof(struct msg_interp))
static NimBLEServer *pServer;

/***
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
    Serial.println("Sent yaml request");

  }
}
***/
// Class to handle events on connection and discconection from client
class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(BLEServer* pServer, NimBLEConnInfo & 	connInfo) override {
        Serial.println("Client connected!");
        has_client.test_and_set();
        send_yaml_request = true;
    }

    void onDisconnect(BLEServer* pServer, NimBLEConnInfo & 	connInfo, int reason) override {
        Serial.println("Client disconnected! Advertsing again");
        has_client.clear();
        if(!welcome_screen_flag){
          if(!is_demo_yaml.test_and_set()){
            is_demo_yaml.clear();
            lv_event_send(welcome_screen,LV_EVENT_LONG_PRESSED ,NULL);
          }
        }

        pServer->startAdvertising();
    }
};

// Handle return button - test example
void return_BLE(){
  uint8_t* msg_bytes = str_to_byte_msg(0,"Hi! How are you today?! Here is a dot . ");
  uint16_t len = sizeof(struct msg_interp);
  // print_msg((struct msg_interp*)msg_bytes);
  pCharacteristic->setValue(msg_bytes, len);
  pCharacteristic->notify();
  free(msg_bytes);
}


// Callback for receiving confirmations from the client
class MyCallbacks: public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) {
    const uint8_t* recived_data = pCharacteristic->getValue().data();
    //print_byte_array(MSG_SIZE, recived_data); // print byte array for debuging
    struct msg_interp* recived_data_struct = (struct msg_interp*)recived_data;
    // if (isMsgCorrupted()){
    //   request_msg_again = malloc(strlen(name)+1+4); /* make space for the new string (should check the return value ...) */
    //   strcpy(name_with_extension, name); /* copy name into the new var */
    //   strcat(name_with_extension, extension); /* add the extension */
    //   SendNotiyToClient((char*)recived_data_struct->req_type, RESEND_REQ, pCharacteristic);
    //   free(name_with_extension);
    //   return;
    // }
    //handle client response based on the request
    switch (recived_data_struct->req_type) {
      case READ_REQ:
            // Add handling for READ_REQ here
          break;
      case EDIT_REQ:
          // Add handling for EDIT_REQ here
          break;
      case FUNC_REQ:
          // Add handling for FUNC_REQ here
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
        ReciveYAMLField(pointer_to_sensor_buff,*recived_data_struct);
        if (recived_data_struct->cur_msg_count==recived_data_struct->tot_msg_count){
          is_yml_sensors_ready = true;
          SendNotiyToClient("Please send Motors data", YML_MOTORS_REQ, pCharacteristic);
          Serial.println("Sent Motors data request");
        }

        break;

      case YML_MOTORS_ANS:
        pointer_to_motors_buff = &motors_yaml_buffer;
        ReciveYAMLField(pointer_to_motors_buff,*recived_data_struct);
        if (recived_data_struct->cur_msg_count==recived_data_struct->tot_msg_count){
          is_yml_motors_ready=true;
          SendNotiyToClient( "Please send functions data", YML_FUNC_REQ, pCharacteristic );
          Serial.println("Sent functions data request");

        }
        break;

      case YML_FUNC_ANS:
          pointer_to_func_buff = &funcs_yaml_buffer;
          ReciveYAMLField(pointer_to_func_buff,*recived_data_struct);
          if (recived_data_struct->cur_msg_count==recived_data_struct->tot_msg_count){
            is_yml_functions_ready=true;
            SendNotiyToClient( "Please send general data", YML_GENERAL_REQ, pCharacteristic );
            Serial.println("Sent general data request");

          }
          break;     

      case YML_GENERAL_ANS:
          pointer_to_general_buff = &general_yaml_buffer;
          ReciveYAMLField(pointer_to_general_buff,*recived_data_struct);
          if (recived_data_struct->cur_msg_count==recived_data_struct->tot_msg_count){
            is_yml_general_ready=true;
          }
          break;     
      case YAML_ANS:
        
        break;
      case GEST_ANS:
        can_play_gesture.test_and_set();
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
  
  Serial.println("Server is advertising");
  while (1) {
    // if ( send_yaml_request ) {
    //   SendNotiyToClient("Please send YAML data", YAML_REQ, pCharacteristic);
    //   Serial.println("Sent yaml request");
    //   send_yaml_request = false;
    // }
    // if (!confirmationReceived) {
    //   //Serial.println("NO");
    // }
    // else {
    //   break;
    // }
    delay(2000);
  }
}

#endif //BLE_NIMBLE_SERVER_H

