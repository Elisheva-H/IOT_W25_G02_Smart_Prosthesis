#include <Arduino.h>
#include <NimBLEDevice.h>

#include "shared_com_vars.h"
#include "requests.h"
#include "create_yaml_file.h"
#include "shared_yaml_parser.h"

static const NimBLEAdvertisedDevice* advDevice;
static bool                          doConnect  = false;
static uint32_t                      scanTimeMs = 5000; /** scan time in milliseconds, 0 = scan forever */
#define SERVICE_UUID        "12345678-1234-5678-1234-56789abcdef3"
#define CHARACTERISTIC_UUID "12345678-1234-5678-1234-56789abcdef4"

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
        
        //if(strcmp(advertisedDevice->getAddress().toString().c_str(), "e0:5a:1b:9f:ed:12") == 0 ){/*Avigail*/
        if (advertisedDevice->isAdvertisingService(NimBLEUUID(SERVICE_UUID))){/*May*/
          Serial.printf("Found Our Service\n");
          /** stop scan before connecting */
          NimBLEDevice::getScan()->stop();
          /** Save the device reference in a global for the client to use*/
          advDevice = advertisedDevice;
          /** Ready to connect now */
          doConnect = true;
        }
        
        // if (advertisedDevice->isAdvertisingService(NimBLEUUID(SERVICE_UUID))) {
        //     Serial.printf("Found Our Service\n");
        //     /** stop scan before connecting */
        //     NimBLEDevice::getScan()->stop();
        //     /** Save the device reference in a global for the client to use*/
        //     advDevice = advertisedDevice;
        //     /** Ready to connect now */
        //     doConnect = true;
        // }
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
    //str             += ": Service = " + pRemoteCharacteristic->getRemoteService()->getUUID().toString();
    //str             += ", Value = " + std::string((char*)pData, length);
    
    print_byte_array(length, pData);

    struct msg_interp* msg = (struct msg_interp*)malloc(sizeof(struct msg_interp));
    *msg = *((struct msg_interp*)pData);
    char* msg_to_calc=msg->msg;
    int checksum_result=calculateChecksum(msg_to_calc, (size_t)msg->msg_length);
    Serial.printf("calculate checksum result is: %d, desired checksum is: %d\n",checksum_result,msg->checksum);
    print_msg(msg);
    if (msg->req_type==GEST_REQ) {
      uint8_t* msg_bytes = str_to_byte_msg(GEST_ANS,msg->msg);
      uint16_t len = sizeof(struct msg_interp);
      print_msg((struct msg_interp*)msg_bytes);
      //TODO: add function call to use gesture 
      delay(3000);
      // the delay mimic function operation to move the hand...........
      pRemoteCharacteristic->writeValue(msg_bytes,sizeof(struct msg_interp));
      Serial.println("sent acknowledge");
    }
    if (msg->req_type==YAML_REQ) {
      Serial.println("Received yaml reuqest, sendind yaml");
      //const char *msg_str=(readYAML()).c_str();
      const char *msg_str=create_default_yaml_string();
      Serial.printf("\nYAML:\n%s\n",msg_str);
      int total_msg_num = ceil(((float)strlen(msg_str))/((float)(MAX_MSG_LEN-1)));
      size_t total_msg_len = strlen(msg_str);
      if (total_msg_num>1){Serial.println("The message is too long, dividing into multiple sends");}
      for (int msg_num=1;msg_num<=total_msg_num;msg_num++){
        uint8_t* msg_bytes = str_to_byte_msg(YML_SENSOR_ANS,msg_str,msg_num, total_msg_num);
        uint16_t len = sizeof(struct msg_interp);
        print_msg((struct msg_interp*)msg_bytes);
        pRemoteCharacteristic->writeValue(msg_bytes, len);
          //TO DO- error handling
        free(msg_bytes);
        print_msg((struct msg_interp*)msg_bytes);

      }
      Serial.println("Sent yaml");

    }


    ///////////////////////////////////////////////
    //////   EDIT REQ - STILL IN PROCESS    ///////
    ///////////////////////////////////////////////
    /*
    if (msg->req_type==EDIT_REQ) {
      Serial.println("Received edit reuqest");
      char msg_copy[MAX_MSG_LEN];
      strcpy(msg_copy, msg->msg);

      // Tokenize the message using "->" as the delimiter
      char* sens_name = strtok(msg_copy, "->");
      char* fun_name = strtok(nullptr, "->");
      char* param_name = strtok(nullptr, "->");
      char* val_str = strtok(nullptr, "->");

      int val_int = atoi(val_str);

      for (auto& sensor : sensors) {
        if (sensor.name == sens_name && sensor.function.name == fun_name) { 
          auto it = sensor.function.parameters.find(param_name);
          it->second.current_val = val_int; // Update the current value
          Serial.printf("Updated %s in %s -> %s to %d\n", param_name, sens_name, fun_name, val_int);
        } else {
          Serial.printf("Parameter %s not found in %s -> %s \n", param_name, sens_name, fun_name);
        }
      }
      /////////////////////////////
      //    ADD WRITE VALUE!!    //
      /////////////////////////////

      //pRemoteCharacteristic->writeValue(###### ADD EDIT CONFIRMATION #####, len);
      
      
      //TO DO- error handling
      Serial.println("Editted val");
    }
    */
    ///////////////////////////////////////////////
    //////   END OF EDIT REQ                ///////
    ///////////////////////////////////////////////

    ///////////////////////////////////////////////
    free(msg);
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
            if (pChr->writeValue("canWrite is true")) {
                Serial.printf("Wrote new value:\n");
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
                if (pChr->writeValue("canWrite is true")) {
                    Serial.printf("Wrote new value: ");
                } else {
                    pClient->disconnect();
                    return false;
                }

                if (pChr->canRead()) {
                    Serial.printf("The value is now: %s\n",
                                  pChr->readValue().c_str());
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
    delay(1000);
    Serial.printf("Starting NimBLE Client\n");
    init_yaml();
    /** Initialize NimBLE and set the device name */
    NimBLEDevice::init("NimBLE-Client");

    /**
     * Set the IO capabilities of the device, each option will trigger a different pairing method.
     *  BLE_HS_IO_KEYBOARD_ONLY   - Passkey pairing
     *  BLE_HS_IO_DISPLAY_YESNO   - Numeric comparison pairing
     *  BLE_HS_IO_NO_INPUT_OUTPUT - DEFAULT setting - just works pairing
     */
    // NimBLEDevice::setSecurityIOCap(BLE_HS_IO_KEYBOARD_ONLY); // use passkey
    // NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_YESNO); //use numeric comparison

    /**
     * 2 different ways to set security - both calls achieve the same result.
     *  no bonding, no man in the middle protection, BLE secure connections.
     *  These are the default values, only shown here for demonstration.
     */
    // NimBLEDevice::setSecurityAuth(false, false, true);

    NimBLEDevice::setSecurityAuth(/*BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM |*/ BLE_SM_PAIR_AUTHREQ_SC);

    /** Optional: set the transmit power */
    NimBLEDevice::setPower(3); /** 3dbm */
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
      }

      //NimBLEDevice::getScan()->start(scanTimeMs, false, true);
  }
}