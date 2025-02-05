#include "HardwareSerial.h"
#ifndef SHARED_YAMEL_PARSER_H
#define SHARED_YAMEL_PARSER_H

#include <vector>
#include <map>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <YAMLDuino.h>
#include "create_yaml_file.h"

#include <stdio.h>
#include <string.h>

char* motors_splited_field; 
char *general_splited_field; 
char *sensors_splited_field; 
char*functions_splited_field;

// Communication struct
struct Communication {
    String name;
    String status;
    String ssid;
    String password;
    String mac;
    String serviceUUID;
    String characteristicUUID;
};

// General struct
struct General {
    String name;
    int code;
};

// Parameter struct
struct Parameter {
    int current_val;
    int min;
    int max;
    bool modify_permission;
};

// Function struct
struct SensorFunction {
    String name;
    std::map<String, Parameter> parameters;  // Key: Parameter name
};

// Sensor struct
struct Sensor {
    String name;
    String status;
    String type;
    SensorFunction function;
};


// MotorPin struct
struct MotorPin {
    String type;
    int pin_number;
};

// Motor struct
struct Motor {
    String name;
    String type;
    std::vector<MotorPin> pins;
    Parameter safety_threshold;
};

// Function struct
struct Function {
    String name;
    String protocol_type;
};


// Global vectors for parsed data
String fileType;
std::vector<General> generalEntries;
std::vector<Communication> communications;
std::vector<Sensor> sensors;
std::vector<Motor> motors;
std::vector<Function> functions;


const char* create_default_yaml_string(){
  const char* yamlContent = R"(
file_type: hand_system_configuration

general:
  - name: 'Technician_code'
    code: 222
  - name: 'Debug_code'
    code: 66666

communications:
  - name: 'WiFi_server'
    status: 'off'
    ssid: 'user_HAND'
    password: 'Haifa3D'

  - name: 'BLE_client'
    status: 'on'
    mac: '11-11-11-11'
    SERVICE1_UUID: "6c09a8a9-be78-4596-9557-3c4bb4965058"
    CHARACTERISTIC1_UUID: "6c09a8a9-be78-4596-9558-3c4bb4965058"

sensors:
  - name: 'leg_pressure_sensor'
    status: 'on'
    type: 'BLE_input'
    function:
      name: 'leg_function'
      parameters:
        param_1: [80,20,100,true]
        high_thld: [90,20,100,false]
        low_thld: [60,20,100,false]

  - name: 'shoulder_sensor'
    status: 'off'
    type: 'BLE_input'
    function:
      name: 'shoulder_function'
      parameters:
        param_1: [60,20,100,true]
        high_thld: [30,20,100,true]
        low_thld: [80,20,100,true]

  - name: 'another_sensor'
    status: 'off'
    type: 'BLE_input'
    function:
      name: 'another_function'
      parameters:
        param_1: [60,20,100,true]
        high_thld: [30,20,100,true]
        low_thld: [80,20,100,true]

motors:
  - name: 'finger1_dc'
    type: 'DC_motor'
    pins:
      - type: 'in1_pin'
        pin_number: 19
      - type: 'in2_pin'
        pin_number: 21
      - type: 'sense_pin'
        pin_number: 34
    safety_threshold: [20,10,50,true]

  - name: 'finger2_dc'
    type: 'DC_motor'
    pins:
      - type: 'in1_pin'
        pin_number: 23
      - type: 'in2_pin'
        pin_number: 22
      - type: 'sense_pin'
        pin_number: 35
    safety_threshold: [20,10,50,true]

  - name: 'finge3_dc'
    type: 'DC_motor'
    pins:
      - type: 'in1_pin'
        pin_number: 4
      - type: 'in2_pin'
        pin_number: 16
      - type: 'sense_pin'
        pin_number: 32
    safety_threshold: [20,10,50,true]

  - name: 'finge4_dc'
    type: 'DC_motor'
    pins:
      - type: 'in1_pin'
        pin_number: 18
      - type: 'in2_pin'
        pin_number: 17
      - type: 'sense_pin'
        pin_number: 33
    safety_threshold: [20,10,50,true]

  - name: 'turn_dc'
    type: 'DC_motor'
    pins:
      - type: 'in1_pin'
        pin_number: 26
      - type: 'in2_pin'
        pin_number: 27
      - type: 'sense_pin'
        pin_number: 36
    safety_threshold: [20,10,50,false]


functions:
  - name: 'send_debug_data'
    protocol_type: 'return_data'

  - name: 'run_motors'
    protocol_type: 'modify_only'

  - name: 'rock'
    protocol_type: 'gesture'

  - name: 'scissors'
    protocol_type: 'gesture'

  - name: 'paper'
    protocol_type: 'gesture'

  - name: 'rest'
    protocol_type: 'gesture'

)";
  return yamlContent;
}

void splitYaml(const char* yaml, char **general_splited_field=NULL, char **sensors_splited_field=NULL, char **motors_splited_field=NULL, char **functions_splited_field=NULL) {
    Serial.println("recived yamel buffer, splitting");
    const char *start_general = strstr(yaml, "general:");
    const char *start_sensors = strstr(yaml, "sensors:");
    const char *start_motors = strstr(yaml, "motors:");
    const char *start_functions = strstr(yaml, "functions:");


    // Find the end of each section by locating the next section or end of string
    const char *end_general = start_sensors ? strstr(start_sensors, "sensors:") : NULL;
    const char *end_sensors = start_motors ? strstr(start_motors, "motors:") : NULL;
    const char *end_motors = start_functions ? strstr(start_functions, "functions:") : NULL;
    const char *end_functions = NULL; // Functions section ends at the end of the string

    // Copy each section into the respective output strings
    if (start_general && general_splited_field) {
        size_t len = (end_general ? end_general : yaml + strlen(yaml)) - start_general;
        *general_splited_field = (char*) malloc((len+1)*sizeof(char));
        strncpy(*general_splited_field, start_general, len);
        (*general_splited_field)[len] = '\0';
    }

    if (start_sensors && sensors_splited_field) {
        size_t len = (end_sensors ? end_sensors : yaml + strlen(yaml)) - start_sensors;
        *sensors_splited_field = (char*) malloc((len+1)*sizeof(char));
        strncpy(*sensors_splited_field, start_sensors, len);
        (*sensors_splited_field)[len] = '\0';
    }

    if (start_motors && motors_splited_field) {
        size_t len = (end_motors ? end_motors : yaml + strlen(yaml)) - start_motors;
        *motors_splited_field = (char*) malloc((len+1)*sizeof(char));
        strncpy(*motors_splited_field, start_motors, len);
        (*motors_splited_field)[len] = '\0';
    }

    if (start_functions && functions_splited_field) {
        size_t len = (end_functions ? end_functions : yaml + strlen(yaml)) - start_functions;
        *functions_splited_field = (char*) malloc((len+1)*sizeof(char));
        strncpy(*functions_splited_field, start_functions, len);
        (*functions_splited_field)[len] = '\0';
    }
  Serial.println("yaml splited. ready for parsing");
}


void parseYAML(const int field_type, const char * yamlContent) {
  JsonDocument doc;
  DeserializationError error = deserializeYml(doc, yamlContent);
  if ( error ) {
    Serial.print("Failed to parse YAML: ");
    Serial.println(error.f_str());
    return;
  }
    // Parse General Entries
  switch (field_type) {
    case GENERAL_FIELD: {
        // Parse General
        JsonArray generalArray = doc["general"];
        for (JsonObject entry : generalArray) {
            General gen;
            gen.name = entry["name"].as<String>();
            gen.code = entry["code"].as<int>();
            generalEntries.push_back(gen);
        }
        break;
    }
    case SENSORS_FIELD: {
        // Parse Sensors
        JsonArray sensorArray = doc["sensors"];
        for (JsonObject entry : sensorArray) {
            Sensor sensor;
            sensor.name = entry["name"].as<String>();
            
            //strcpy(sensor.status ,entry["status"].as<String>().c_str()); ///AFTER CHANGING STATUS TYPE
            sensor.status = entry["status"].as<String>().c_str(); ///BEFORE CHANGING STATUS TYPE
           
            sensor.type = entry["type"].as<String>();
            JsonObject funcObj = entry["function"];
            sensor.function.name = funcObj["name"].as<String>();

            JsonObject params = funcObj["parameters"];
            for (JsonPair param : params) {
                Parameter paramData;
                JsonArray paramArray = param.value().as<JsonArray>();
                paramData.current_val = paramArray[0];
                paramData.min = paramArray[1];
                paramData.max = paramArray[2];
                paramData.modify_permission = paramArray[3];
                sensor.function.parameters[param.key().c_str()] = paramData;
            }
            sensors.push_back(sensor);
        }
        break;
    }
      case MOTORS_FIELD:
        {
        // Parse Motors
        JsonArray motorArray = doc["motors"];
        for (JsonObject entry : motorArray) {
            Motor motor;
            motor.name = entry["name"].as<String>();
            motor.type = entry["type"].as<String>();

            JsonArray pinsArray = entry["pins"];
            for (JsonObject pin : pinsArray) {
                MotorPin motorPin;
                motorPin.type = pin["type"].as<String>();
                motorPin.pin_number = pin["pin_number"].as<int>();
                motor.pins.push_back(motorPin);
            }

            JsonArray threshold = entry["safety_threshold"].as<JsonArray>();
            motor.safety_threshold.current_val = threshold[0];
            motor.safety_threshold.min = threshold[1];
            motor.safety_threshold.max = threshold[2];
            motor.safety_threshold.modify_permission = threshold[3];
            motors.push_back(motor);
        }
        break;
        }
      case FUNCTIONS_FIELD: {
        // Parse Functions
        JsonArray actionArray = doc["functions"];
        for (JsonObject entry : actionArray) {
            Function function;
            function.name = entry["name"].as<String>();
            function.protocol_type = entry["protocol_type"].as<String>();
            functions.push_back(function);
        }
        break;
      }
    default:
        Serial.println("Unknown field type!");
        break;
  }
}

void printGeneral(const General& general) {
    Serial.println("General:");
    Serial.print("  Name: ");
    Serial.println(general.name);
    Serial.print("  Code: ");
    Serial.println(general.code);
}

void printParameter(const Parameter& parameter, const String& parameterName = "") {
    if (!parameterName.isEmpty()) {
        Serial.print("  Parameter Name: ");
        Serial.println(parameterName);
    }
    Serial.print("    Current Value: ");
    Serial.println(parameter.current_val);
    Serial.print("    Min: ");
    Serial.println(parameter.min);
    Serial.print("    Max: ");
    Serial.println(parameter.max);
    Serial.print("    Modify Permission: ");
    Serial.println(parameter.modify_permission ? "true" : "false");
}

void printSensorFunction(const SensorFunction& function) {
    Serial.println("  Function:");
    Serial.print("    Name: ");
    Serial.println(function.name);

    Serial.println("    Parameters:");
    for (const auto& [paramName, param] : function.parameters) {
        printParameter(param, paramName);
    }
}

void printSensor(const Sensor& sensor) {
    Serial.println("Sensor:");
    Serial.print("  Name: ");
    Serial.println(sensor.name);
    Serial.print("  Status: ");
    Serial.println(sensor.status);
    Serial.print("  Type: ");
    Serial.println(sensor.type);
    printSensorFunction(sensor.function);
}

void printMotorPin(const MotorPin& pin) {
    Serial.print("    Pin Type: ");
    Serial.println(pin.type);
    Serial.print("    Pin Number: ");
    Serial.println(pin.pin_number);
}

void printMotor(const Motor& motor) {
    Serial.println("Motor:");
    Serial.print("  Name: ");
    Serial.println(motor.name);
    Serial.print("  Type: ");
    Serial.println(motor.type);

    Serial.println("  Pins:");
    for (const auto& pin : motor.pins) {
        printMotorPin(pin);
    }

    Serial.println("  Safety Threshold:");
    printParameter(motor.safety_threshold);
}

void printFunction(const Function& function) {
    Serial.println("Function:");
    Serial.print("  Name: ");
    Serial.println(function.name);
    Serial.print("  Protocol Type: ");
    Serial.println(function.protocol_type);
}


void splitSensorsField(const char* yaml){
     Serial.println("splitting sensors");
    ////yamel sensors field to split and parse
      const char* sensors_start = strstr(yaml, "sensors:");
    if (!sensors_start) {
        Serial.println("No sensors section found.");
        return;
    }

    // Skip past the "sensors:" line
    sensors_start = strstr(sensors_start, "- name:");

    // Loop to process each sensor entry
    while (strstr(sensors_start, "- name:")) { 
        char* str_title = "sensors:\n  "; // each sensor will have the field title to maintain yaml format
        
        // Find the next "- name:" to mark the end of the current sensor
        const char* sensor_end = strstr(sensors_start + 1, "- name:");
        if (sensor_end == nullptr) {
            sensor_end = yaml + strlen(yaml);  // End of the string
        }

        // Allocate memory for the single sensor block, including the header "sensors:"
        char* single_sensor = (char*)malloc((sensor_end - sensors_start + strlen(str_title) + 1) * sizeof(char));  // +1 for null terminator
        if (single_sensor == nullptr) {
            Serial.println("Memory allocation failed.");
            return;
        }

        // Copy "sensors:" header and the current sensor block while maintaining yaml format
        strncpy(single_sensor, str_title, strlen(str_title));
        strncpy(&single_sensor[strlen(str_title)], sensors_start, sensor_end - sensors_start);
        single_sensor[sensor_end - sensors_start + strlen(str_title)] = '\0';  // Null-terminate the string

        // Parsing the sensor using yaml parser and saving in a struct
        parseYAML(SENSORS_FIELD, single_sensor);
        
        // Move to the next sensor entry
        sensors_start = sensor_end;

        // Free the allocated memory after processing
        free(single_sensor);
    }
      // Print Sensor details
  Serial.println("=== Sensors ===");
  for (const auto& sensor : sensors) {
      printSensor(sensor);
      Serial.println();
  }
  return;
}


void splitFunctionsField(const char* yaml ){
  Serial.println("splitting functions");
    const char* functions_start = strstr(yaml, "functions:");
    if (!functions_start) {
        Serial.println("No functions section found.");
        return;
    }

    // Skip past the "functions:" line
    functions_start = strstr(functions_start, "- name:");

    // Loop to process each function entry
    while (strstr(functions_start, "- name:")) {
        char* str_title="functions:\n  "; //each function will have the field title to maintain yaml format for parsing
        
        // Find the next "- name:" to mark the end of the current function
        const char* function_end = strstr(functions_start + 1, "- name:");
        if (function_end == nullptr) {
            function_end = yaml + strlen(yaml);  // End of the string
        }

        // Allocate memory for the single function block, including the header "functions:"
        char* single_function = (char*)malloc((function_end - functions_start + strlen(str_title) + 1) * sizeof(char));  // +1 for null terminator
        if (single_function == nullptr) {
            Serial.println("Memory allocation failed.");
            return;
        }

        // Copy "functions:" header and the current function block while maintaining yaml format
        strncpy(single_function, str_title, strlen(str_title));
        strncpy(&single_function[strlen(str_title)], functions_start, function_end - functions_start);
        single_function[function_end - functions_start + strlen(str_title)] = '\0';  // Null-terminate the string

        //Parsing the function using yaml parser and saving in a struct
        parseYAML(FUNCTIONS_FIELD, single_function);
        
        // Move to the next function entry
        functions_start = function_end;

        // Free the allocated memory after processing
        free(single_function);
    }
    Serial.println("=== Functions ===");
    for (const auto& function : functions) {
        printFunction(function);
        Serial.println();
    }
    return;
}

void splitGeneralField(const char* yaml) {
    Serial.println("splitting general");
    const char* general_start = strstr(yaml, "general:");
    if (!general_start) {
        Serial.println("No general section found.");
        return;
    }

    // Skip past the "general:" line
    general_start = strstr(general_start, "- name:");

    // Loop to process each general entry
    while (strstr(general_start, "- name:")) {  
        char* str_title = "general:\n  ";  // Each general entry will have the field title to maintain YAML format
        
        // Find the next "- name:" to mark the end of the current general entry
        const char* general_end = strstr(general_start + 1, "- name:");
        if (general_end == nullptr) {
            general_end = yaml + strlen(yaml);  // End of the string
        }

        // Allocate memory for the single general block, including the header "general:"
        char* single_general = (char*)malloc((general_end - general_start + strlen(str_title) + 1) * sizeof(char));  // +1 for null terminator
        if (single_general == nullptr) {
            Serial.println("Memory allocation failed.");
            return;
        }

        // Copy "general:" header and the current general block while maintaining YAML format
        strncpy(single_general, str_title, strlen(str_title));
        strncpy(&single_general[strlen(str_title)], general_start, general_end - general_start);
        single_general[general_end - general_start + strlen(str_title)] = '\0';  // Null-terminate the string

        // Parsing the general using yaml parser and saving in a struct
        parseYAML(GENERAL_FIELD, single_general);

        // Move to the next general entry
        general_start = general_end;

        // Free the allocated memory after processing
        free(single_general);
    }
    return;
}

void splitMotorsField(const char* yaml) {
    Serial.println("splitting motors");
    const char* motors_start = strstr(yaml, "motors:");
    if (!motors_start) {
        Serial.println("No motors section found.");
        return;
    }

    // Skip past the "motors:" line
    motors_start = strstr(motors_start, "- name:");

    // Loop to process each motor entry
    while (strstr(motors_start, "- name:")) {  
        char* str_title = "motors:\n  ";  // Each motor entry will have the field title to maintain YAML format
        
        // Find the next "- name:" to mark the end of the current motor entry
        const char* motors_end = strstr(motors_start + 1, "- name:");
        if (motors_end == nullptr) {
            motors_end = yaml + strlen(yaml);  // End of the string
        }

        // Allocate memory for the single motor block, including the header "motors:"
        char* single_motor = (char*)malloc((motors_end - motors_start + strlen(str_title) + 1) * sizeof(char));  // +1 for null terminator
        if (single_motor == nullptr) {
            Serial.println("Memory allocation failed.");
            return;
        }

        // Copy "motors:" header and the current motor block while maintaining YAML format
        strncpy(single_motor, str_title, strlen(str_title));
        strncpy(&single_motor[strlen(str_title)], motors_start, motors_end - motors_start);
        single_motor[motors_end - motors_start + strlen(str_title)] = '\0';  // Null-terminate the string

        // Parsing the motor using yaml parser and saving in a struct
        parseYAML(MOTORS_FIELD, single_motor);

        // Move to the next motor entry
        motors_start = motors_end;

        // Free the allocated memory after processing
        free(single_motor);
    }
        Serial.println("=== Motors ===");
    for (const auto& motor : motors) {
        printMotor(motor);
        Serial.println();
    }
    return;
}

void init_yaml() {
  String DefaultYamlContent = create_default_yaml_string();
  const char* yamlContent = ReadYmlUsingSPIFFS(DefaultYamlContent).c_str();
  splitYaml(yamlContent, &general_splited_field, &sensors_splited_field, &motors_splited_field, &functions_splited_field);
  splitMotorsField((char*)motors_splited_field);
  free(motors_splited_field);
  splitSensorsField((char*)sensors_splited_field);
  free(sensors_splited_field);
  splitGeneralField((char*)general_splited_field);
  free(general_splited_field);
  splitFunctionsField((char*)functions_splited_field);
  free(functions_splited_field);
}

#endif //SHARED_YAMEL_PARSER_H