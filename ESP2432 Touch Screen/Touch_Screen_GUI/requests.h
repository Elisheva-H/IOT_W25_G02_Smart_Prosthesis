#include "HardwareSerial.h"
#include "Print.h"
#ifndef REQUESTS_H
#define REQUESTS_H
#include "shared_com_vars.h"
#include "shared_yaml_parser.h"

// Initialize YAML flags
static bool is_yml_general_ready = false;
static bool is_yml_sensors_ready = false;
static bool is_yml_motors_ready = false;
static bool is_yml_functions_ready = false;

int isMsgCorrupted(struct msg_interp* struct_val){
  if ((struct_val->msg_length!=strlen(struct_val->msg))||
      calculateChecksum(struct_val->msg,struct_val->msg_length)!=(struct_val->checksum)){
            Serial.printf("got msg %d out of %d.\n",struct_val->cur_msg_count,struct_val->tot_msg_count);
            Serial.printf("Received MSG length and desired length are %s, (received length: %d, desired length: %d)\n",
                            struct_val->msg_length == strlen(struct_val->msg) ? "equal" : "not equal",
                            strlen(struct_val->msg), struct_val->msg_length);
            Serial.printf("Calculate MSG checksum is %s to desired cheksum!\n",
                (struct_val->checksum) == (calculateChecksum(struct_val->msg,struct_val->msg_length)) ? "equal" : "not equal");
    return 0;
    }
  return 1;
}

void ReciveYAMLField(uint8_t** buffer_to_use,struct msg_interp struct_val){
  Serial.println("Recive YAML Field");
  //print_msg(struct_val);
  Serial.printf("%d\n",(struct_val.cur_msg_count) );
  if ((struct_val.cur_msg_count)==1) {
    Serial.println("1");
    *buffer_to_use=(uint8_t*)calloc((struct_val.tot_msg_count)*MAX_MSG_LEN, sizeof(uint8_t));    
    Serial.println("2");
    if(!(*buffer_to_use)){
      Serial.println("calloc failed");
    }
    Serial.printf("%d\n",*buffer_to_use);
  } 
  Serial.println("3");
  Serial.printf("(struct_val.cur_msg_count-1)%d\n",(struct_val.cur_msg_count-1));
  Serial.printf("3\n");
  Serial.printf("printing byte array\n");
  print_byte_array((struct_val.tot_msg_count)*(MAX_MSG_LEN-1) , *buffer_to_use);

  memcpy( (*buffer_to_use) + ((struct_val.cur_msg_count-1)*(MAX_MSG_LEN-1)) , struct_val.msg,  struct_val.msg_length );
  Serial.println("4");

  if (struct_val.cur_msg_count==struct_val.tot_msg_count){
    Serial.println("5");

    int tot_length= (struct_val.tot_msg_count-1)*(MAX_MSG_LEN-1)+(struct_val.msg_length)-1;
    //(*buffer_to_use)[tot_length-1]=0x00;
    print_byte_array(tot_length+2 , *buffer_to_use);
    is_yml_sensors_ready=true;
  }
}

void processYAMLField(int yml_filed_type, uint8_t* buffer_to_parse) {
  Serial.println("yaml is ready for splitting and parsing");
  switch (yml_filed_type) {
      // case YML_GENERAL_ANS:
      //   parseYAML(GENERAL_FIELD, (char*) buffer_to_parse);
      //   break;

      case YML_SENSOR_ANS:
        if (*buffer_to_parse) {
          Serial.printf("\n#2:  %s\n",(char*)buffer_to_parse);

          parseYAML(SENSORS_FIELD, (char*)buffer_to_parse);

          // Print Sensor details
          Serial.println("=== Sensors ===");
          for (const auto& sensor : sensors) {
              printSensor(sensor);
              Serial.println();
          }
        } else {Serial.println("Null address");}
        break;

      case YML_MOTORS_ANS:
        //parseYAML(MOTORS_FIELD, (char*) buffer_to_parse);

        // Print Motor details
        Serial.println("=== Motors ===");
        for (const auto& motor : motors) {
            printMotor(motor);
            Serial.println();
        }
        break;

      case YML_FUNC_ANS:
        //parseYAML(FUNCTIONS_FIELD, (char*) buffer_to_parse);

        // Print Function details
        Serial.println("=== Functions ===");
        for (const auto& function : functions) {
            printFunction(function);
            Serial.println();
        } 
      break;

      default:
        Serial.println("Unknown field type.");
        break;
  }
}

// void recived_YAML(int filed_type){
//       Serial.println("yaml is ready for splitting and parsing");
//       char *general_splited_field, *sensors_splited_field, *motors_splited_field, *functions_splited_field;
      
//       // Call the function to split YAML
//       splitYaml(&general_splited_field, &sensors_splited_field, &motors_splited_field, &functions_splited_field);

//     if (general_splited_field ) {
//       parseYAML(GENERAL_FIELD, (char*) general_splited_field);
//       printf("General:\n%s\n", general_splited_field);
//     }

//     if (sensors_splited_field) {
//       parseYAML(SENSORS_FIELD, (char*) sensors_splited_field);
//       printf("\nSensors:\n%s\n", sensors_splited_field);

//       // Print Sensor details
//       Serial.println("=== Sensors ===");
//       for (const auto& sensor : sensors) {
//         printSensor(sensor);
//         Serial.println();
//       }
//     }

//     if (motors_splited_field) {
//       parseYAML(MOTORS_FIELD, (char*) motors_splited_field);
//       printf("\nMotors:\n%s\n", motors_splited_field);

//       // Print Motor details
//       Serial.println("=== Motors ===");
//       for (const auto& motor : motors) {
//       printMotor(motor);
//       Serial.println();
//       }
//     }

//     if (functions_splited_field) {
//       parseYAML(FUNCTIONS_FIELD, (char*) functions_splited_field);
//       printf("\nFunctions:\n%s\n", functions_splited_field);

//       // Print Function details
//       Serial.println("=== Functions ===");
//       for (const auto& function : functions) {
//         printFunction(function);
//         Serial.println();
//     }
// }

#endif //REQUESTS_H
