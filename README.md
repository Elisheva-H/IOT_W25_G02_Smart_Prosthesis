## Smart Prosthesis Project by:
Avigail Yampolsky, Elisheva Hammer and May Abraham
  
## Details about the project:
This project focuses on developing a touchscreen-based server for managing a smart prosthesis controller using an ESP32. The touchscreen interface serves as the UI, enabling users to customize prosthetic parameters, execute preprogrammed movements, adjust sensor behavior, and more. However, it does not directly control the prosthesis itself.

To facilitate management, the prosthesis controller (see prosthesis mock files) sends its current system settings to the touchscreen server using YAML format. This YAML file contains all relevant data, including available sensors and motors, screen access passwords, and the functions that should be executed. When no BLE connection is available, the screen provides an option to load a mock YAML file, allowing users to add, modify, and debug different screens.

**Manager Tool Modes**
The manager tool operates in three distinct modes:

1. Daily Mode – Designed for regular use, this mode is divided into three tabs:

Home Tab: Allows users to execute preprogrammed movements.
Status Tab: Displays the current status of all sensors.
Setup Tab: Enables turning sensors on and off.

2. Tech Mode – This mode is protected by a password defined in the YAML file. It includes all three tabs from Daily Mode and adds a Tech Tab, which provides:

The ability to read and modify motor and sensor-related data.
An option to manually turn motors on and off for testing.

3. Debug Mode – Also password-protected (as defined in the YAML file), this mode includes all four tabs from Tech Mode and introduces a Debug Tab, which allows:

Live plotting of sensor or motor output data for debugging purposes.

Example for such YAML file:
file_type: hand_system_configuration

general:
  - name: 'Technician_code'
    code: 28282
  - name: 'Debug_code'
    code: 2024

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
  - name: 'leg_pressure_sensor1'
    status: 'on'
    type: 'BLE_input'
    function:
      name: 'leg_function'
      parameters:
        param_1: [80,20,100,true]
        high_thld: [90,20,100,true]
        low_thld: [60,20,100,true]

  - name: 'shoulder_sensor1'
    status: 'off'
    type: 'BLE_input'
    function:
      name: 'shoulder_function'
      parameters:
        param_1: [60,20,100,true]
        high_thld: [30,20,100,true]
        low_thld: [80,20,100,true]

  - name: 'leg_pressure_sensor2'
    status: 'on'
    type: 'BLE_input'
    function:
      name: 'leg_function'
      parameters:
        param_1: [80,20,100,true]
        high_thld: [90,20,100,true]
        low_thld: [60,20,100,true]

  - name: 'shoulder_sensor2'
    status: 'off'
    type: 'BLE_input'
    function:
      name: 'shoulder_function'
      parameters:
        param_1: [60,20,100,true]
        high_thld: [30,20,100,true]
        low_thld: [80,20,100,true]

  - name: 'leg_pressure_sensor3'
    status: 'on'
    type: 'BLE_input'
    function:
      name: 'leg_function'
      parameters:
        param_1: [80,20,100,true]
        high_thld: [90,20,100,true]
        low_thld: [60,20,100,true]

  - name: 'shoulder_sensor3'
    status: 'off'
    type: 'BLE_input'
    function:
      name: 'shoulder_function'
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
    safety_threshold: [20,10,50,true]

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

  - name: 'rock'
    protocol_type: 'gesture'

  - name: 'scissors'
    protocol_type: 'gesture'


## Folder description:
* ESP32: source code for the esp side (firmware).
* Documentation: wiring diagram + basic operating instructions
* Unit Tests: tests for individual hardware components (input / output devices)
* Parameters: contains description of configurable parameters 

## Arduino/ESP32 libraries used in this project:
* Servo by Michael Margolis, Arduino - 1.2.2
* Adafruit BusIO by Adafruit - 1.16.1
* Adafruit GFX Library by Adafruit - 1.11.9
* Adafruit NeoMatrix by Adafruit - 1.3.2
* Adafruit NeoPixel by Adafruit - 1.12.3
* Arduinojson by Benoit Blanchon - 7.1.0
* ESP32Servo by Kevin Harrington, John K. Bennett - 3.0.5
* NimBLE-Arduino by h2zero - 1.4.2
* YAMLDuino by tobozo - 1.4.2
* TAMC_GT911 by TAMC - 1.0.2
* Ivgl by kisvegabor - 8.3.3
* GFX Library for Arduino by Moon On Our Nation - 1.2.9
* Touch_GT911 (Manually installed, ADD REFERENCE)


## Arduino/ESP libraries installed for the project:




## Project Poster:
 
This project is part of ICST - The Interdisciplinary Center for Smart Technologies, Taub Faculty of Computer Science, Technion
https://icst.cs.technion.ac.il/
