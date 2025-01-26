#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "shared_com_vars.h"

char* YAML_JSON; // We need to add the actual file.

void send_YAML(){
  size_t length = strlen(YAML_JSON);
  if(!length){
    Serial.println("YAML_JSON file is not exist");
    return;
  }
  for(int i = 0; i<(int)length/MAX_MSG_LEN; i++){
      
  }
}

