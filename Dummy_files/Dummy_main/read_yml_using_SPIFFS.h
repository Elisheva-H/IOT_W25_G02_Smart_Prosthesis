#include <FS.h>
#include <SPIFFS.h>
#include <Arduino.h>

const String config_yaml="/config.yaml";

void listAllFiles(){
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while(file){
      Serial.print("FILE: ");
      Serial.println(file.name());
      file = root.openNextFile();
  }
}

void writeYAMLFile() {
  // Open or create a file on SPIFFS in write mode
  File file = SPIFFS.open(config_yaml, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  // Write YAML content to the file
  String yamlContent = R"(
file_type: hand_system_configuration

communications:
  - name: 'WiFi_server'
    status: 'off'
    ssid: 'user_HAND'
    password: 'Haifa3D'
  - name: 'BLE_client'
    status: 'on'
    mac: '11-11-11-11'
    SERVICE1_UUID: 'e0198002-7544-42c1-0000-b24344b6aa70'
    CHARACTERISTIC1_UUID: 'e0198002-7544-42c1-0001-b24344b6aa70'

sensors:
  - name: 'leg_pressure_sensor'
    status: 'on'
    type: 'BLE_input'
    function:
      name: 'leg_function'
      parameters:
        max: 100
        high_thld: 80
        low_thld: 20
  - name: 'shoulder_sensor'
    status: 'off'
    type: 'BLE_input'
    function:
      name: 'shoulder_function'
      parameters:
        max: 100
        high_thld: 70
        low_thld: 30

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
    safety_threshold: 20
  - name: 'finger2_dc'
    type: 'DC_motor'
    pins:
      - type: 'in1_pin'
        pin_number: 23
      - type: 'in2_pin'
        pin_number: 22
      - type: 'sense_pin'
        pin_number: 35
    safety_threshold: 20
  - name: 'finger3_dc'
    type: 'DC_motor'
    pins:
      - type: 'in1_pin'
        pin_number: 4
      - type: 'in2_pin'
        pin_number: 16
      - type: 'sense_pin'
        pin_number: 32
    safety_threshold: 20
  - name: 'finger4_dc'
    type: 'DC_motor'
    pins:
      - type: 'in1_pin'
        pin_number: 18
      - type: 'in2_pin'
        pin_number: 17
      - type: 'sense_pin'
        pin_number: 33
    safety_threshold: 20
  - name: 'turn_dc'
    type: 'DC_motor'
    pins:
      - type: 'in1_pin'
        pin_number: 26
      - type: 'in2_pin'
        pin_number: 27
      - type: 'sense_pin'
        pin_number: 36
    safety_threshold: 20
)";

  file.print(yamlContent);
  // Close the file
  file.close();
}

void checkFile() {
    if (SPIFFS.exists(config_yaml)) {
    File file = SPIFFS.open(config_yaml, FILE_READ);
    Serial.println("Reading from "+ config_yaml +":");
    while (file.available()) {
      Serial.write(file.read());
    }
    file.close();
  } else {
    Serial.println("Failed to open file for reading");
    return;
  } 
}

void ReadYmlUsingSPIFFS() {
  if (!SPIFFS.begin(true)) {
      Serial.println("Failed to initialize SPIFFS!");
      return;
  }
  Serial.println("SPIFFS initialized successfully!");
  if ( SPIFFS.exists(config_yaml)) {
    Serial.println("File "+ config_yaml +" found!");
  } else {
    Serial.println("Failed to fine config file, writing new " +config_yaml+ " file!");
  // Create and write to a file
    writeYAMLFile();
  } 
  listAllFiles();
  checkFile();
  File file = SPIFFS.open(config_yaml, FILE_READ);
  String yamlContent = file.readString();
  file.close();
  parseYAML(yamlContent);
  // Print parsed data
  Serial.println("Parsed Data:");
  Serial.println("File Type: " + fileType);
  
  for (auto& comm : communications) {
    Serial.println("Communication: " + comm.name + ", Status: " + comm.status);
    if (comm.name == "WiFi_server") {
      Serial.println("SSID: " + comm.ssid + ", Password: " + comm.password);
    } else if (comm.name == "BLE_client") {
      Serial.println("MAC: " + comm.mac + ", Service UUID: " + comm.serviceUUID + ", Characteristic UUID: " + comm.characteristicUUID);
    }
  }

  for (auto& sensor : sensors) {
    Serial.println("Sensor: " + sensor.name + ", Status: " + sensor.status + ", Type: " + sensor.type);
    Serial.println("Function: " + sensor.function.name + ", Max: " + String(sensor.function.max) +
                  ", High Thld: " + String(sensor.function.highThld) +
                  ", Low Thld: " + String(sensor.function.lowThld));
  }

  for (auto& motor : motors) {
    Serial.println("Motor: " + motor.name + ", Type: " + motor.type + ", Safety Threshold: " + String(motor.safetyThreshold));
    for (auto& pin : motor.pins) {
      Serial.println("  Pin Type: " + pin.type + ", Pin Number: " + String(pin.pinNumber));

    }
  }
}