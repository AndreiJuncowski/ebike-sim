#include <Arduino.h>
#include <NimBLEDevice.h>
#include "ServerCallbacks.h"
#include "IndoorBike.h"

static NimBLEServer* pServer;
static NimBLECharacteristic* pIndoorBikeCaracterisic;
static NimBLEService* pFITMachineService;
ServerCallbacks serverCallbacks;


/** Handler class for characteristic actions */
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        Serial.printf("%s : onRead(), value: %s\n",
                      pCharacteristic->getUUID().toString().c_str(),
                      pCharacteristic->getValue().c_str());
    }

    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        Serial.printf("%s : onWrite(), value: %s\n",
                      pCharacteristic->getUUID().toString().c_str(),
                      pCharacteristic->getValue().c_str());
    }

    /**
     *  The value returned in code is the NimBLE host return code.
     */
    void onStatus(NimBLECharacteristic* pCharacteristic, int code) override {
        Serial.printf("Notification/Indication return code: %d, %s\n", code, NimBLEUtils::returnCodeToString(code));
    }

    /** Peer subscribed to notifications/indications */
    void onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) override {
        std::string str  = "Client ID: ";
        str             += connInfo.getConnHandle();
        str             += " Address: ";
        str             += connInfo.getAddress().toString();
        if (subValue == 0) {
            str += " Unsubscribed to ";
        } else if (subValue == 1) {
            str += " Subscribed to notifications for ";
        } else if (subValue == 2) {
            str += " Subscribed to indications for ";
        } else if (subValue == 3) {
            str += " Subscribed to notifications and indications for ";
        }
        str += std::string(pCharacteristic->getUUID());

        Serial.printf("%s\n", str.c_str());
    }
} chrCallbacks;

void sendIndoorBikeData() {
  IndoorBike indoorBikeData;
  float velocidade = 5;
  float tempoVoltaMicrosegundos = 1000000;

  indoorBikeData.setSpeed(velocidade);
  indoorBikeData.setTempoPorVoltaMicros(tempoVoltaMicrosegundos);
  uint8_t buf[20];
  uint8_t len = indoorBikeData.build(buf);
  pIndoorBikeCaracterisic->setValue(buf, len);
  pIndoorBikeCaracterisic->notify();
  indoorBikeData.print();
}

unsigned long lastSent = 0;
void send() {
  unsigned long int now = millis();
  if(now - lastSent >= 1000) {
    lastSent = now;
    sendIndoorBikeData();    
  }
}

unsigned long lastCheck = 0;
void checkClients() {
  unsigned long int now = millis();
  if(now - lastCheck >= 2000) {
    lastCheck = now;
    if (pServer->getConnectedCount()) {
        NimBLEService* pSvc = pServer->getServiceByUUID("1826");
        // if (pSvc) {
        //     NimBLECharacteristic* pChr = pSvc->getCharacteristic("F00D");
        //     if (pChr) {
        //         pChr->notify();
        //     }
        // }
    }    
  }
}

void setup(void) {
    Serial.begin(115200);
    Serial.printf("Starting NimBLE Server\n");

    NimBLEDevice::init("EBike Juncowski");

    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(&serverCallbacks); 

    pFITMachineService = pServer->createService("1826");
    pIndoorBikeCaracterisic = pFITMachineService->createCharacteristic("2AD2", NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::WRITE);

    // pIndoorBikeCaracterisic->setValue("Burger");
    pIndoorBikeCaracterisic->setCallbacks(&chrCallbacks);


    /** Create an advertising instance and add the services to the advertised data */
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->setName("EBike Juncowski");
    pAdvertising->addServiceUUID(pFITMachineService->getUUID());
    /**
     *  If your device is battery powered you may consider setting scan response
     *  to false as it will extend battery life at the expense of less data sent.
     */
    pAdvertising->enableScanResponse(true);
    pAdvertising->start();

    Serial.printf("Advertising Started\n");
}

void loop() {
    send();
    checkClients();
}