#ifndef REQUESTS_H
#define REQUESTS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "shared_com_vars.h"

char* YAML_JSON; // We need to add the actual file.

void SendNotiyToClient(char* msg_str, int msg_type, NimBLERemoteCharacteristic* pRemoteCharacteristic){
  int total_msg_num = ceil(((float)strlen(msg_str))/((float)(MAX_MSG_LEN-1)));
  size_t total_msg_len = strlen(msg_str);
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


          //TO DO- error handling

#endif //REQUESTS_H