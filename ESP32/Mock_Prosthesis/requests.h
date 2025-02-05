#include "esp32-hal.h"
#ifndef REQUESTS_H
#define REQUESTS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "functions_calls_handeling.h"


void SendNotifyToServer(char* msg_str, int msg_type, NimBLERemoteCharacteristic* pRemoteCharacteristic){
  int total_msg_num = ceil(((float)strlen(msg_str))/((float)(MAX_MSG_LEN-1)));
  if (total_msg_num>1){Serial.println("The message is too long, dividing into multiple sends");}
  for (int msg_num=1;msg_num<=total_msg_num;msg_num++){
    uint8_t* msg_bytes = str_to_byte_msg(msg_type, msg_str,msg_num, total_msg_num);
    uint16_t len = sizeof(struct msg_interp);
    print_msg((struct msg_interp*)msg_bytes);
    pRemoteCharacteristic->writeValue(msg_bytes, len);
      //TO DO- error handling
    free(msg_bytes);
  }
}

void SimulateGestureRun(char* msg_str, NimBLERemoteCharacteristic* pRemoteCharacteristic){
  delay(1500);
  call_function(msg_str);
  Serial.printf("Done playing %s, sending acknoweldge", msg_str);
  SendNotifyToServer(msg_str, GEST_ANS, pRemoteCharacteristic);
}

#endif //REQUESTS_H