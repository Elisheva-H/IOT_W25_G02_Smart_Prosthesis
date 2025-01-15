#include <Arduino.h>
#include <ArduinoJson.h>
#include <YAMLDuino.h>
#include <cstdio>
#include <vector>
#include <string>
#include <filesystem>


// Define structures to hold data
struct Communication {
  String name;
  String status;
  String ssid;
  String password;
  String mac;
  String serviceUUID;
  String characteristicUUID;
};

struct SensorFunction {
  String name;
  int max;
  int highThld;
  int lowThld;
};

struct Sensor {
  String name;
  String status;
  String type;
  SensorFunction function;
};

struct MotorPin {
  String type;
  int pinNumber;
};

struct Motor {
  String name;
  String type;
  std::vector<MotorPin> pins;
  int safetyThreshold;
};

// Variables to store parsed data
String fileType;
std::vector<Communication> communications;
std::vector<Sensor> sensors;
std::vector<Motor> motors;

void parseYAML(const String& yamlContent) {
  JsonDocument doc; // Adjust size as needed
  DeserializationError error = deserializeYml(doc, yamlContent.c_str());
  if ( error ) {
    Serial.print("Failed to parse YAML: ");
    Serial.println(error.f_str());
    return;
  }
  serializeJson(doc, Serial);
  Serial.println();
  
  // Parse file type
  fileType = doc["file_type"].as<String>();

  // Parse communications
  JsonArray commArray = doc["communications"];
  for ( JsonObject commObj : commArray ) {
    Communication comm;
    comm.name = commObj["name"].as<String>();
    comm.status = commObj["status"].as<String>();
    if (comm.name == "WiFi_server") {
      comm.ssid = commObj["ssid"].as<String>();
      comm.password = commObj["password"].as<String>();
    } else if (comm.name == "BLE_client") {
      comm.mac = commObj["mac"].as<String>();
      comm.serviceUUID = commObj["SERVICE1_UUID"].as<String>();
      comm.characteristicUUID = commObj["CHARACTERISTIC1_UUID"].as<String>();
    }
    communications.push_back(comm);
  }

  // Parse sensors
  JsonArray sensorArray = doc["sensors"];
  for (JsonObject sensorObj : sensorArray) {
    Sensor sensor;
    sensor.name = sensorObj["name"].as<String>();
    sensor.status = sensorObj["status"].as<String>();
    sensor.type = sensorObj["type"].as<String>();
    JsonObject funcObj = sensorObj["function"];
    sensor.function.name = funcObj["name"].as<String>();
    JsonObject params = funcObj["parameters"];
    sensor.function.max = params["max"];
    sensor.function.highThld = params["high_thld"];
    sensor.function.lowThld = params["low_thld"];
    sensors.push_back(sensor);
  }

  // Parse motors
  JsonArray motorArray = doc["motors"];
  for (JsonObject motorObj : motorArray) {
    Motor motor;
    motor.name = motorObj["name"].as<String>();
    motor.type = motorObj["type"].as<String>();
    motor.safetyThreshold = motorObj["safety_threshold"];
    JsonArray pinsArray = motorObj["pins"];
    for (JsonObject pinObj : pinsArray) {
      MotorPin pin;
      pin.type = pinObj["type"].as<String>();
      pin.pinNumber = pinObj["pin_number"];
      motor.pins.push_back(pin);
    }
    motors.push_back(motor);
  }
}




