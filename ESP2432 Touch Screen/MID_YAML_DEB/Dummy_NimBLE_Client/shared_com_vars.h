#ifndef SHARED_COM_VALS_H
#define SHARED_COM_VALS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>


#define MAX_MSG_LEN 128

enum msg_type{ 
  READ_REQ, EDIT_REQ, FUNC_REQ, YAML_REQ, GEST_REQ,
  READ_ANS, EDIT_ANS, FUNC_ANS, YAML_ANS, GEST_ANS,
  YML_SENSOR_REQ, YML_MOTORS_REQ, YML_FUNC_REQ, YML_GENERAL_REQ,
  YML_SENSOR_ANS, YML_MOTORS_ANS, YML_FUNC_ANS, YML_GENERAL_ANS,
};

struct msg_interp{
  int req_type;
  int cur_msg_count;
  int tot_msg_count;
  int msg_length;
  char msg[MAX_MSG_LEN];
  int checksum;
};


uint8_t calculateChecksum(const char* data, size_t length) {
    uint8_t checksum = 0;

    for (size_t i = 0; i < length; ++i) {
        checksum ^= data[i]; // XOR each byte
    }

    return checksum;
}

uint8_t* str_to_byte_msg(int req_type, const char* msg_str, int msg_num=1, int total_msg_num=1){
  size_t struct_size = sizeof(struct msg_interp);
  uint8_t* byte_msg = (uint8_t*)malloc(struct_size);
  if (byte_msg == NULL) {
      perror("Failed to allocate memory for MSG");
      return NULL;
  }
  struct msg_interp *msg_buff = (struct msg_interp *) byte_msg;
  msg_buff->cur_msg_count = msg_num;  
  msg_buff->tot_msg_count = total_msg_num;  
  msg_buff->req_type = req_type;
  size_t start = (msg_num-1) * (MAX_MSG_LEN-1);
  size_t remainderToEnd = (strlen(msg_str) - start);
  size_t currentChunkSize = (MAX_MSG_LEN-1 < remainderToEnd) ? MAX_MSG_LEN-1 : remainderToEnd;
  strncpy(msg_buff->msg, msg_str + start, currentChunkSize);
  msg_buff->msg[currentChunkSize] = '\0';
  uint8_t checksum_result = calculateChecksum(msg_buff->msg, currentChunkSize);
  msg_buff->checksum= checksum_result;
  msg_buff->msg_length = currentChunkSize;
  // Copy the struct contents to the byte array
  return byte_msg;
}



void print_byte_array(size_t length, const uint8_t* pData){
  Serial.print("Byte array: ");
  for (size_t i = 0; i < length; i++) {
    Serial.print(pData[i], HEX);  // Prints each byte as hexadecimal
    Serial.print(" ");
  }
  Serial.println();
}


void print_msg(struct msg_interp* msg){
  Serial.printf("req_type: %d, cur_msg_count: %d, tot_msg_count: %d, msg_length: %d, msg: %s, desired checksum: %d\n",
  msg->req_type, msg->cur_msg_count, msg->tot_msg_count, msg->msg_length, msg->msg, msg->checksum);
}


/////////////////////////////////////////////
// Simple communication usage example      //
/////////////////////////////////////////////

// void return_BLE(){

//   uint8_t* msg_bytes = str_to_byte_msg(0,"Hi! How are you today?! Here is a dot . ");
//   uint16_t len = sizeof(struct msg_interp);
//   struct msg_interp* try_to_read = (struct msg_interp*)msg_bytes;
//   Serial.printf("req_type: %d, cur_msg_count: %d, tot_msg_count: %d, msg_length: %d, msg: %s",
//    try_to_read->req_type, try_to_read->cur_msg_count, try_to_read->tot_msg_count, try_to_read->msg_length, try_to_read->msg);


//   pCharacteristic->setValue(msg_bytes, len);
//   pCharacteristic->notify();
// }

#endif //SHARED_COM_VALS_H