#include <Arduino.h>
#include <NimBLEDevice.h>

#include "shared_com_vars.h"
#include "requests.h"
#include "create_yaml_file.h"
#include "shared_yaml_parser.h"
#include "functions_calls_handeling.h"

static const NimBLEAdvertisedDevice* advDevice;
static bool                          doConnect  = false;
static uint32_t                      scanTimeMs = 5000; /** scan time in milliseconds, 0 = scan forever */
#define SERVICE_UUID        "12345678-1234-5678-1234-56789abcdef0"
#define CHARACTERISTIC_UUID "12345678-1234-5678-1234-56789abcdef1"

/**  None of these are required as they will be handled by the library with defaults. **
 **                       Remove as you see fit for your needs                        */
class ClientCallbacks : public NimBLEClientCallbacks {
    void onConnect(NimBLEClient* pClient) override { Serial.printf("Connected\n"); }

    void onDisconnect(NimBLEClient* pClient, int reason) override {
        Serial.printf("%s Disconnected, reason = %d - Starting scan\n", pClient->getPeerAddress().toString().c_str(), reason);
        NimBLEDevice::getScan()->start(scanTimeMs, false, false);
    }

    /********************* Security handled here *********************/
    void onPassKeyEntry(NimBLEConnInfo& connInfo) override {
        Serial.printf("Server Passkey Entry\n");
        /**
         * This should prompt the user to enter the passkey displayed
         * on the peer device.
         */
        NimBLEDevice::injectPassKey(connInfo, 123456);
    }

    void onConfirmPasskey(NimBLEConnInfo& connInfo, uint32_t pass_key) override {
        Serial.printf("The passkey YES/NO number: %" PRIu32 "\n", pass_key);
        /** Inject false if passkeys don't match. */
        NimBLEDevice::injectConfirmPasskey(connInfo, true);
    }

    /** Pairing process complete, we can check the results in connInfo */
    void onAuthenticationComplete(NimBLEConnInfo& connInfo) override {
        if (!connInfo.isEncrypted()) {
            Serial.printf("Encrypt connection failed - disconnecting\n");
            /** Find the client with the connection handle provided in connInfo */
            NimBLEDevice::getClientByHandle(connInfo.getConnHandle())->disconnect();
            return;
        }
    }
} clientCallbacks;

/** Define a class to handle the callbacks when scan events are received */
class ScanCallbacks : public NimBLEScanCallbacks {
    void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override {
        Serial.printf("Advertised Device found: %s\n", advertisedDevice->toString().c_str());
        Serial.printf("Address: %s\n", advertisedDevice->getAddress().toString().c_str());        
        if (advertisedDevice->isAdvertisingService(NimBLEUUID(SERVICE_UUID))){
          Serial.printf("Found Our Service\n");
          /** stop scan before connecting */
          NimBLEDevice::getScan()->stop();
          /** Save the device reference in a global for the client to use*/
          advDevice = advertisedDevice;
          /** Ready to connect now */
          doConnect = true;
        }
    }

    /** Callback to process the results of the completed scan or restart it */
    void onScanEnd(const NimBLEScanResults& results, int reason) override {
        Serial.printf("Scan Ended, reason: %d, device count: %d; Restarting scan\n", reason, results.getCount());
        NimBLEDevice::getScan()->start(scanTimeMs, false, true);
    }
} scanCallbacks;

/** Notification / Indication receiving handler callback */
void notifyCB(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    std::string str  = (isNotify == true) ? "Notification" : "Indication";
    Serial.println("received request");
    struct msg_interp* received_data = (struct msg_interp*)malloc(sizeof(struct msg_interp));
    *received_data = *((struct msg_interp*)pData);
    print_msg(received_data);
    char* received_msg;
    switch (received_data->req_type) {
      case EMERGENCY_STOP:
        Serial.println("Recived emergency stop request");
        call_function("EmergencyStop");
        break;
    
    case GEST_REQ:
      SimulateGestureRun(received_data->msg, pRemoteCharacteristic);
      break;
    
    case YAML_REQ:
      Serial.println("Recivied yaml request, sending sensors data");
      char* sensors_splited_field2;
      splitYaml(readYAML().c_str(), NULL, &sensors_splited_field2, NULL, NULL);
      // Sending sensors data
      SendNotifyToServer(sensors_splited_field2, YML_SENSOR_ANS, pRemoteCharacteristic);
      break;

    case YML_MOTORS_REQ:
      // Sending motors data
      char* motors_splited_field2;
      splitYaml(readYAML().c_str(), NULL, NULL, &motors_splited_field2, NULL);
      SendNotifyToServer(motors_splited_field2, YML_MOTORS_ANS, pRemoteCharacteristic);
      break;

    case YML_FUNC_REQ:
      // Sending functions data
      char* functions_splited_field2;
      splitYaml(readYAML().c_str(), NULL, NULL, NULL, &functions_splited_field2);
      SendNotifyToServer(functions_splited_field2, YML_FUNC_ANS, pRemoteCharacteristic);
      break;

    case YML_GENERAL_REQ:
      // Sending general data
      char* general_splited_field2;
      splitYaml(readYAML().c_str(), &general_splited_field2, NULL, NULL, NULL);
      SendNotifyToServer(general_splited_field2, YML_GENERAL_ANS, pRemoteCharacteristic);
      Serial.println("Finished sending yaml data");
      break;

    case CHANGE_SENSOR_STATE_REQ:
      // Handling requests to change sensor status on <=> off
      received_msg = (char*)malloc(MAX_MSG_LEN * (received_data->tot_msg_count));
      if (received_msg != NULL) {
        strcpy(received_msg, received_data->msg);
        char* tokened_msg;
        tokened_msg = strtok(received_msg, "|");
        int i = 0;
        while (tokened_msg != NULL) {
          if (i == 2 || i == 0) {
            i = 0;
            current_sensor_id = atoi(tokened_msg);
            Serial.printf("New sensor ID is %d, status is %s.\n", current_sensor_id, sensors[current_sensor_id].name.c_str());
          } else {
            sensor_status = tokened_msg;
            call_function("ChangeSensorState");
          }
          tokened_msg = strtok(NULL, "|");
          i++;
        }

        SendNotifyToServer(received_data->msg, CHANGE_SENSOR_STATE_ANS, pRemoteCharacteristic);
        free(received_msg);
      }
      break;

    case CHANGE_SENSOR_PARAM_REQ:
      received_msg = (char*)malloc(MAX_MSG_LEN * received_data->tot_msg_count);
      if (received_msg) {
        strcpy(received_msg, received_data->msg);
        char* tokened_msg;
        tokened_msg = strtok(received_msg, "|");
        int i = 0;
        int parameter_id;
        while (tokened_msg != NULL) {
          if (i == 3 || i == 0) {
            i = 0;
            current_sensor_id = atoi(tokened_msg);
            Serial.printf("New sensor ID is %d, sensor name is %s.\n", current_sensor_id, sensors[current_sensor_id].name.c_str());
          } else if (i == 1) {
            parameter_id = atoi(tokened_msg);
          } else {
            int new_parameter_val = atoi(tokened_msg);
            // Finding the corresponding map key inside the struct
            std::map<String, Parameter> parameter_map = sensors[current_sensor_id].function.parameters;
            int j = 0;
            for (const auto& [paramName, param] : parameter_map) {
              if (j == parameter_id) {
                int max_val = parameter_map[paramName].max;
                int min_val = parameter_map[paramName].min;
                if ((new_parameter_val <= max_val) && (new_parameter_val >= min_val) && (parameter_map[paramName].modify_permission==true)) {
                  parameter_map[paramName].current_val = new_parameter_val;
                  Serial.printf("New val %d for key %s in sensor ID %d\n", new_parameter_val, paramName, current_sensor_id);
                }
                else{
                  Serial.printf("New val is not in range or nor permitted\n");
                }
                break;
              }
              j++;
            }
          }
          tokened_msg = strtok(NULL, "|");
          i++;
        }
        SendNotifyToServer(received_data->msg, CHANGE_SENSOR_PARAM_ANS, pRemoteCharacteristic);
        if (received_msg) { free(received_msg); }
      }
      break;
    
    case CHANGE_MOTOR_PARAM_REQ:
      received_msg = (char*)malloc(MAX_MSG_LEN * received_data->tot_msg_count);
      int current_motor_id;
      if (received_msg){
        strcpy(received_msg, received_data->msg);
        char* tokened_msg ;
        tokened_msg=strtok(received_msg, "|");
        int i=0;
        int parameter_id;
        while(tokened_msg != NULL) {
          if (i==2|| i==0){
            i=0;
            current_motor_id=atoi(tokened_msg);
            Serial.printf("new motor id is %d, motor name is %s.\n",current_motor_id,motors[current_motor_id].name.c_str());
          } 
          else {
            int  new_parameter_val = atoi(tokened_msg);
            // Finding the corrosponding map  key in inside the struct
            int j=0;
            int max_val =   motors[current_motor_id].safety_threshold.max;
            int min_val =   motors[current_motor_id].safety_threshold.min;
            if (( new_parameter_val <= max_val ) && ( new_parameter_val >= min_val ) && (motors[current_motor_id].safety_threshold.modify_permission==true)) {
              motors[current_motor_id].safety_threshold.current_val=new_parameter_val;
              Serial.printf("new safety treshold is %d for motors id %d\n",new_parameter_val, current_motor_id);
            }
            else {
              Serial.printf( "Parameter cant be changed! allowed range: [%d, %d], modification premission: %s\n" , 
                motors[current_motor_id].safety_threshold.min,  motors[current_motor_id].safety_threshold.max,
                (motors[current_motor_id].safety_threshold.modify_permission)? "true" :"false");
            }
          }
          tokened_msg = strtok(NULL, "|");
          i++;
        }
      }
      SendNotifyToServer(received_data->msg, CHANGE_MOTOR_PARAM_ANS, pRemoteCharacteristic);
      if (received_msg){free(received_msg);}
      break;

    case READ_REQ:{
      char* received_msg= (char*)malloc(MAX_MSG_LEN);
      int is_motor;
      int hardware_id;
      if (received_msg){
        strcpy(received_msg,received_data->msg);
        char* tokened_msg ;
        tokened_msg=strtok(received_msg, "|");
        int i=0;
        while(tokened_msg != NULL) {
          if (i==0){
            is_motor=atoi(tokened_msg);
            Serial.printf("received real time data request for %s.\n",
            is_motor==1 ? "motor" : "sensor");
            i++;
          } 
          else {
            hardware_id = atoi(tokened_msg);
            break;
          }
          tokened_msg = strtok(NULL, "|");
        }
        int sampled_data=GetRealTimeData(is_motor, hardware_id);
        String sampled_data_str = String(sampled_data);
        Serial.printf("sampled data str %s. msg length %d \n",sampled_data_str.c_str(),received_data->msg_length);
        strcpy(received_msg,received_data->msg);
        strcpy(&(received_msg[received_data->msg_length]),"|");
        strcpy(&(received_msg[received_data->msg_length+1]),sampled_data_str.c_str());
        SendNotifyToServer(received_msg, READ_ANS, pRemoteCharacteristic); // Send response
        Serial.printf("msg: %s\n",received_msg);
        if (received_msg){free(received_msg);}
      }   
      break;}

    
    default:
        Serial.println("unrecognized respone");
        break;
  } 
  free(received_data);
}



/** Handles the provisioning of clients and connects / interfaces with the server */
bool connectToServer() {
    NimBLEClient* pClient = nullptr;

    /** Check if we have a client we should reuse first **/
    if (NimBLEDevice::getCreatedClientCount()) {
        /**
         *  Special case when we already know this device, we send false as the
         *  second argument in connect() to prevent refreshing the service database.
         *  This saves considerable time and power.
         */
        pClient = NimBLEDevice::getClientByPeerAddress(advDevice->getAddress());
        if (pClient) {
            if (!pClient->connect(advDevice, false)) {
                Serial.printf("Reconnect failed\n");
                return false;
            }
            Serial.printf("Reconnected client\n");
        } else {
            /**
             *  We don't already have a client that knows this device,
             *  check for a client that is disconnected that we can use.
             */
            pClient = NimBLEDevice::getDisconnectedClient();
        }
    }

    /** No client to reuse? Create a new one. */
    if (!pClient) {
        if (NimBLEDevice::getCreatedClientCount() >= NIMBLE_MAX_CONNECTIONS) {
            Serial.printf("Max clients reached - no more connections available\n");
            return false;
        }

        pClient = NimBLEDevice::createClient();

        Serial.printf("New client created\n");

        pClient->setClientCallbacks(&clientCallbacks, false);
        /**
         *  Set initial connection parameters:
         *  These settings are safe for 3 clients to connect reliably, can go faster if you have less
         *  connections. Timeout should be a multiple of the interval, minimum is 100ms.
         *  Min interval: 12 * 1.25ms = 15, Max interval: 12 * 1.25ms = 15, 0 latency, 150 * 10ms = 1500ms timeout
         */
        pClient->setConnectionParams(12, 12, 0, 150);

        /** Set how long we are willing to wait for the connection to complete (milliseconds), default is 30000. */
        pClient->setConnectTimeout(5 * 1000);

        if (!pClient->connect(advDevice)) {
            /** Created a client but failed to connect, don't need to keep it as it has no data */
            NimBLEDevice::deleteClient(pClient);
            Serial.printf("Failed to connect, deleted client\n");
            return false;
        }
    }

    if (!pClient->isConnected()) {
        if (!pClient->connect(advDevice)) {
            Serial.printf("Failed to connect\n");
            return false;
        }
    }

    Serial.printf("Connected to: %s RSSI: %d\n", pClient->getPeerAddress().toString().c_str(), pClient->getRssi());

    /** Now we can read/write/subscribe the characteristics of the services we are interested in */
    NimBLERemoteService*        pSvc = nullptr;
    NimBLERemoteCharacteristic* pChr = nullptr;

    pSvc = pClient->getService(SERVICE_UUID);
    if (pSvc) {
        pChr = pSvc->getCharacteristic(CHARACTERISTIC_UUID);
    }

    if (pChr) {
        if (pChr->canRead()) {
            Serial.printf("%s Value: %s\n", pChr->getUUID().toString().c_str(), pChr->readValue().c_str());
        }

        if (pChr->canWrite()) {
            if (pChr->writeValue("")) {
            } else {
                pClient->disconnect();
                return false;
            }

            if (pChr->canRead()) {
                Serial.printf("The value is now: %s\n", pChr->readValue().c_str());
            }
        }

        if (pChr->canNotify()) {
            if (!pChr->subscribe(true, notifyCB)) {
                pClient->disconnect();
                return false;
            }
        } else if (pChr->canIndicate()) {
            /** Send false as first argument to subscribe to indications instead of notifications */
            if (!pChr->subscribe(false, notifyCB)) {
                pClient->disconnect();
                return false;
            }
        }
    } else {
        Serial.printf("service not found.\n");
    }

    pSvc = pClient->getService(SERVICE_UUID);
    if (pSvc) {
        pChr = pSvc->getCharacteristic(CHARACTERISTIC_UUID);
        if (pChr) {
            if (pChr->canRead()) {
                Serial.printf("Value: %s\n", pChr->readValue().c_str());
            }

            if (pChr->canWrite()) {
                if (pChr->writeValue("")) {
                } else {
                    pClient->disconnect();
                    return false;
                }

                if (pChr->canRead()) {
                    
                }
            }

            if (pChr->canNotify()) {
                if (!pChr->subscribe(true, notifyCB)) {
                    pClient->disconnect();
                    return false;
                }
            } else if (pChr->canIndicate()) {
                /** Send false as first argument to subscribe to indications instead of notifications */
                if (!pChr->subscribe(false, notifyCB)) {
                    pClient->disconnect();
                    return false;
                }
            }
        }
    } else {
        Serial.printf("service not found.\n");
    }

    Serial.printf("Done with this device!\n");
    return true;
}

void setup() {
    Serial.begin(115200);
    delay(3000);
    Serial.printf("Starting NimBLE Client\n");
    init_yaml();
    /** Initialize NimBLE and set the device name */
    NimBLEDevice::init("NimBLE-Client");
    NimBLEScan* pScan = NimBLEDevice::getScan();
    /** Set the callbacks to call when scan events occur, no duplicates */
    pScan->setScanCallbacks(&scanCallbacks, false);
    /** Set scan interval (how often) and window (how long) in milliseconds */
    pScan->setInterval(100);
    pScan->setWindow(100);
    /** Start scanning for advertisers */
    pScan->start(scanTimeMs);
    Serial.printf("Scanning for peripherals\n");
}

void loop() {
  /** Loop here until we find a device we want to connect to */
  delay(100);
  if (doConnect) {
    doConnect = false;
    /** Found a device we want to connect to, do it now */
    if (connectToServer()) {
        Serial.printf("Success! we should now be getting notifications, scanning for more!\n");
    } else {
        Serial.printf("Failed to connect, starting scan\n");
        NimBLEDevice::getScan()->start(scanTimeMs, false, false);      
    }
  }
}