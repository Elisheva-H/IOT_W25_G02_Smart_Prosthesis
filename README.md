## Smart Prosthesis Managment Tool Project by:
Avigail Yampolsky, Elisheva Hammer and May Abraham
  
## Details about the project:
This project focuses on developing a touchscreen-based server for managing a smart prosthesis controller using an ESP32. The touchscreen interface serves as the UI, enabling users to customize prosthetic parameters, execute preprogrammed movements, adjust sensor behavior, and more. However, it does not directly control the prosthesis itself.

To facilitate management, the prosthesis controller (see prosthesis mock files under ESP32 folder) sends its current system settings to the touchscreen server using YAML format. This YAML file contains all relevant data, including available sensors and motors, screen access passwords, and the functions that should be executed. When no BLE connection is available, the screen provides an option to load a mock YAML file, allowing users to add, modify, and debug different screens. An example for such YAML file can be found in the assets folder.

**Manager Tool Modes**
The manager tool operates in three distinct modes:

1. Daily Mode – Designed for regular use, this mode is divided into three tabs:

Home Tab: Allows users to execute preprogrammed movements.
Status Tab: Displays the current status of all sensors.
Setup Tab: Enables turning sensors on and off.

2. Tech Mode – This mode is protected by a password defined in the YAML file. It includes all three tabs from Daily Mode and adds a Tech Tab, which provides:
The ability to read and modify motor and sensor-related data.
An option to manually turn motors on and off for testing.

3. Debug Mode – Also password-protected (as defined in the YAML file), this mode includes all four tabs from Tech Mode and introduces a Debug Tab, which allows live plotting of sensor or motor output data for debugging purposes.



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
