#ifndef FUNCTIONS_CALLS_HANDELING_H
#define FUNCTIONS_CALLS_HANDELING_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "shared_yaml_parser.h"
 
int current_sensor_id;
char* sensor_status;

// Simulating gestures functions
void scissors(void) { printf("Run gesture scissors\n"); }
void rock(void) { printf("Run gesture rock\n"); }
void paper(void) { printf("Run gesture paper \n"); }
void rest(void) { printf("Returning to rest position\n"); }

// Struct to hold function pointers with parameters
// Simulating sensor state
void ChangeSensorState(int current_sensor_id, const char* sensor_status) { 
  Serial.printf("old status is %s.\n",sensors[current_sensor_id].status.c_str());
  sensors[current_sensor_id].status=strcmp(sensor_status,"1")==0 ? "on" : "off";
  Serial.printf("new status is %s.\n",sensors[current_sensor_id].status.c_str());
}

int GetRealTimeData(int is_motor, int hardware_id) { 
  //// WE ARE USING RAND() TO SIMULATE MOTOR AND SENSOR VALUES. 
  /////HOWEVER, FOR THE REAL PROSTHESIS INSERT HERE THE MOTOR\SENSOR DATA SAMPLING USING ID

  int min;
  int max;
  if(is_motor){
    min = 5;
    max = 30;
  } else{
    min = 10;
    max = 90;
  }
  return (rand() % (max - min + 1) + min);
}

// Wrapper function for ChangeSensorState
void ChangeSensorStateWrapper(void) {
    ChangeSensorState(current_sensor_id, sensor_status);
}

// Function map
const static struct {
    const char *name;
    void (*func)(void);
} function_map[] = {
    { "scissors", scissors },
    { "rock", rock },
    { "paper", paper },
    { "rest", rest },
    { "ChangeSensorState", ChangeSensorStateWrapper }, // Use wrapper function
};

// Function caller
int call_function(const char *name) {
    for (int i = 0; i < (sizeof(function_map) / sizeof(function_map[0])); i++) {
        if (!strcmp(function_map[i].name, name) && function_map[i].func) {
            function_map[i].func();
            return 0;
        }
    }
    return -1;
}

#endif //FUNCTIONS_CALLS_HANDELING_H