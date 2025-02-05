## Smart Prosthesis Management Tool Project

### By:
**Avigail Yampolsky, Elisheva Hammer, and May Abraham**

---

## Project Overview
This project focuses on developing a touchscreen-based server for managing a smart prosthesis controller using an ESP32. The touchscreen interface serves as the UI, enabling users to:
- Customize prosthetic parameters
- Execute preprogrammed movements
- Adjust sensor behavior

However, it does not directly control the prosthesis itself.

The prosthesis controller (see prosthesis mock files under the **ESP32** folder) sends its current system settings to the touchscreen server in **YAML format**. This YAML file contains all relevant data, including:
- Available sensors and motors
- Screen access passwords
- Functions to be executed

When no BLE connection is available, the screen provides an option to load a **mock YAML file**, allowing users to add, modify, and debug different screens.

The YAML file contains **five major fields**:
- General
- Sensors
- Motors
- Functions  
- Communications - not required for the tool's operation.

An example YAML file can be found in the **assets** folder.

---

## Manager Tool Modes
<p align="center">
    <img src="https://github.com/Elisheva-H/IOT_W25_G02_Smart_Prosthesis/blob/main/opening_screen.gif" width="450" />
</p>

The manager tool operates in three distinct modes:

<p align="center">
    <img src="https://github.com/user-attachments/assets/126c853b-01d0-4720-8fe2-71e0360abe49" width="300" />
</p>

### 1. **Daily Mode**
Designed for regular use, this mode is divided into three tabs:
- **Home Tab**: Allows users to execute preprogrammed movements.
- **Status Tab**: Displays the current status of all sensors.
- **Setup Tab**: Enables turning sensors on and off.
  
<p align="center">
    <img src="https://github.com/user-attachments/assets/4f9d6b1e-300f-4c94-a7e3-ccd735f24412" width="300" />
    <img src="https://github.com/user-attachments/assets/8e1cb333-fe7f-4135-9ab5-994010cf73db" width="300" />
    <img src="https://github.com/user-attachments/assets/c3a374d4-496a-42fe-8370-d0a732563c24" width="300" />
</p>

### 2. **Tech Mode**
This mode is password-protected (defined in the YAML file). It includes all three tabs from **Daily Mode**, plus an additional **Tech Tab**, which allows:
- Reading and modifying motor and sensor-related data.
- Manually turning motors on and off for testing.

<p align="center">
    <img src="https://github.com/user-attachments/assets/c5631387-82bd-491d-bbae-ddb5e8079693" width="300" />
</p>

### 3. **Debug Mode**
Also password-protected, this mode includes all four tabs from **Tech Mode** plus a **Debug Tab**, which allows live plotting of sensor or motor output data for debugging purposes.

<p align="center">
    <img src="https://github.com/user-attachments/assets/d67ce858-80f6-480c-bb71-0c9377bab425" width="300" />
    <img src="https://github.com/user-attachments/assets/92cd46e0-28c5-4172-a5a3-16990916cf6d" width="300" />
</p>

---

## BLE Communication

### **Initialization & Connection**
- On startup, the management controller displays a **BLE connection screen** and attempts to connect using the predefined **UUID**.
- The prosthesis controller parses the **YAML file** and sends the parsed data to the **management controller**, which stores it in a structured dictionary.
- If reconnection is needed, a button on this screen allows restarting the connection process.

<p align="center">
    <img src="https://github.com/user-attachments/assets/7466baf1-56f7-42ad-84a3-cca5d23511e0" width="300" />
</p>



### **Request's interpretation**
All requests are transmitted as a **byte array** in hexadecimal format. The byte array includes additional metadata for handling and parsing. It follows the structure below:

- **First 4 bytes**: Request or response type, using predefined enums (see Request Types section). This determines how the request is processed.
- **Bytes 5-8**: Current message number. If a message is too long to send in one part, it is split into multiple messages.
- **Bytes 9-12**: Total number of messages expected for the request. Used to track message sequences.
- **Bytes 13-16**: Length of the actual message payload (not the total byte array length). This represents the size of the `char*` message.
- **Bytes 17-(MAX_MSG_LEN+17)**: The message itself, parsed as a `char*`.
- **Last 4 bytes**: Expected checksum value for data integrity verification.

The byte array is interpreted using the following predefined structure:

```cpp
struct msg_interp {
  int req_type;         // Type of request.  
  int cur_msg_count;    // Current message index in a sequence  
  int tot_msg_count;    // Total number of messages in a sequence  
  int msg_length;       // Length of the message data  
  char msg[MAX_MSG_LEN]; // Message payload  
  int checksum;         // Error-checking value  
};
```

Where **MAX_MSG_LEN** is a predefined maximum message size. If a message exceeds this limit, it will be split into multiple messages and sent sequentially.
Additionally, when needed, each **motor, sensor, and parameter** is identified by its respective index in the corresponding vector.

### **Request Types**

- **EMERGENCY_STOP** – A high-priority request running on a separate task, triggered by pressing the **BOOT button** on the management controller. When activated, the management tool sends a request to halt all motors. This remains functional as long as a BLE connection is active.

- **CHANGE_SENSOR_STATE_REQ** – Requests enabling or disabling specific sensors. Multiple sensors and states (1 = ON, 0 = OFF) can be updated simultaneously based on user input in **Daily Mode**.

- **CHANGE_SENSOR_PARAM_REQ** – Requests updating a sensor's parameter value. Multiple parameter modifications can be sent at once, specifying the **sensor ID, parameter ID, and desired value**, based on user input in **Tech Mode**.

- **CHANGE_MOTOR_PARAM_REQ** – Requests changing a motor’s **safety threshold** value. This request requires the **motor ID** and is initiated based on user input in **Tech Mode**.

- **GEST_REQ** – Requests executing a **predefined movement (gesture)**. The movement name is retrieved from the YAML file under the **function field** and categorized as a "gesture" type. The prosthesis must have a matching gesture defined with the same name.

- **YML_SENSOR_REQ, YML_MOTORS_REQ, YML_FUNC_REQ, YML_GENERAL_REQ** – These requests are sent sequentially upon establishing a connection. To accommodate larger YAML files, each YAML field is transmitted separately. Each request is sent **only after** the previous one has been fully processed to prevent data loss.

---

## Folder Description
- **ESP32**: Source code for the ESP32 firmware. Contains source code for managment tool and the mock prostheis code. 
- **Unit Tests**: Tests for individual hardware components (input/output devices).
- **Parameters**: Contains descriptions of configurable parameters.
- **Assets**: Contains resources and library configuration files.

---
## Firmware

### Management Tool (ESP32-3248S032 N/R/C)
The management tool includes the following features:
- USB Type-C
- ST7789 Display
- XPT2046 Touchscreen / GT911
- TF Card Interface
- I2C: 2 x JST1.0 4p connectors
- Power & Serial: JST 1.25 4p connector
- Speaker: JST 1.25 2p connector
- Battery Interface: JST 1.25 2p connector

### Mock Prosthesis
Any ESP32 with BLE connectivity.

---
Project was complied using core driver **ESP32** by Espressif, version 2.0.17. 
## Arduino/ESP32 Libraries Used
- **ArduinoJson** by Benoit Blanchon - 7.1.0
- **NimBLE-Arduino** by h2zero - 1.4.2
- **YAMLDuino** by tobozo - 1.4.2
- **TAMC_GT911** by TAMC - 1.0.2
- **lvgl** by kisvegabor - 8.3.3
- **GFX Library for Arduino** by Moon On Our Nation - 1.2.9
- **Touch_GT911** (Manually installed, see assetss)

---

## Project Poster
This project is part of **ICST - The Interdisciplinary Center for Smart Technologies**, Taub Faculty of Computer Science, Technion.

🔗 [ICST Website](https://icst.cs.technion.ac.il/)

