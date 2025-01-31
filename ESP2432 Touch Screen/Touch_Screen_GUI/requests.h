#include "HardwareSerial.h"
#include "Print.h"
#ifndef REQUESTS_H
#define REQUESTS_H
#include "shared_com_vars.h"
#include "shared_yaml_parser.h"

// Initialize YAML flags
static bool is_yml_general_ready = false;
// std::atomic_flag is_yml_sensors_ready = ATOMIC_FLAG_INIT;

static bool is_yml_sensors_ready = false;
static bool is_yml_motors_ready = false;
static bool is_yml_functions_ready = false;

// std::queue<uint8_t*> requestQueue;
// std::map<uint8_t*, bool> requestStatus;

void SendNotiyToClient(char* msg_str, int msg_type, NimBLECharacteristic *pCharacteristic){
  int total_msg_num = ceil(((float)strlen(msg_str))/((float)(MAX_MSG_LEN-1)));
  if (total_msg_num>1){Serial.println("The message is too long, dividing into multiple sends");}
  for (int msg_num=1;msg_num<=total_msg_num;msg_num++){
    uint8_t* msg_bytes = str_to_byte_msg(msg_type, msg_str,msg_num, total_msg_num);
    uint16_t len = sizeof(struct msg_interp);
    Serial.print("Sending msg:");
    print_msg((struct msg_interp*)msg_bytes);
    pCharacteristic->setValue(msg_bytes, len);
    pCharacteristic->notify();
      //TO DO- error handling
    free(msg_bytes);
  }
}


// void addRequest(uint8_t* request) {
//     requestQueue.push(request);
//     requestStatus[request] = false;
// }

// void processQueue() {
//     if (!requestQueue.empty()) {
//         uint8_t* currentRequest = requestQueue.front();
//         if (!requestStatus[currentRequest]) {
//           uint16_t len = sizeof(struct msg_interp);
//           Serial.print("Sending msg again: ");
//           print_msg((struct msg_interp*)msg_bytes);
//           pCharacteristic->setValue(msg_bytes, len);
//           pCharacteristic->notify();
//             //TO DO- error handling
//           free(msg_bytes);
//             sendRequest(currentRequest);
//         } else {
//             requestQueue.pop();
//             requestStatus.erase(currentRequest);
//         }
//     }
// }

bool isMsgCorrupted(struct msg_interp* struct_val){
  if ((struct_val->msg_length!=strlen(struct_val->msg))||
      calculateChecksum(struct_val->msg,struct_val->msg_length)!=(struct_val->checksum)){
            Serial.printf("got msg %d out of %d.\n",struct_val->cur_msg_count,struct_val->tot_msg_count);
            Serial.printf("Received MSG length and desired length are %s, (received length: %d, desired length: %d)\n",
                            struct_val->msg_length == strlen(struct_val->msg) ? "equal" : "not equal",
                            strlen(struct_val->msg), struct_val->msg_length);
            Serial.printf("Calculate MSG checksum is %s to desired cheksum!\n",
                (struct_val->checksum) == (calculateChecksum(struct_val->msg,struct_val->msg_length)) ? "equal" : "not equal");
    return true;
    }
  return false;
}

void ReciveYAMLField(uint8_t** buffer_to_use,struct msg_interp struct_val){
  Serial.printf("Recived msg %d out of %d.\n", struct_val.cur_msg_count,struct_val.tot_msg_count);
  //print_msg(struct_val);
  if ((struct_val.cur_msg_count)==1) {
    *buffer_to_use=(uint8_t*)calloc((struct_val.tot_msg_count)*MAX_MSG_LEN, sizeof(uint8_t));    
    if(!(*buffer_to_use)){
      Serial.println("calloc failed");
    }
  } 
  memcpy( (*buffer_to_use) + ((struct_val.cur_msg_count-1)*(MAX_MSG_LEN-1)) , struct_val.msg,  struct_val.msg_length );
}

#endif //REQUESTS_H

