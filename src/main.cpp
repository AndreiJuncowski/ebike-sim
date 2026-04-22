#include <Arduino.h>
#include <NimBLEDevice.h>
#include "ServerCallbacks.h"
#include "utils.h"
#include "IndoorBike.h"
#include "FitnessMachine.h"
#include "FitnessMachineControlPoint.h"
#include "RotorSensor.h"

static NimBLEServer* pServer;
NimBLEAdvertising* pAdvertising;

static NimBLEService* pFITMachineService;
static NimBLECharacteristic* pIndoorBikeCharacterisic;
static NimBLECharacteristic* pFitnessMachineControlPointCharacterisic;
static NimBLECharacteristic* pFitnessMachineFeatureCharacteristic;

static NimBLEService* pCyclePowerService;
static NimBLECharacteristic* cyclePowerFeature;
static NimBLECharacteristic* cyclePowerMesuremento;
static NimBLECharacteristic* cycleSensorLocation;

ServerCallbacks serverCallbacks;
FitnessMachineControlPoint fitnessMachineControlPoint;

RotorSensor rotor(19, 4400);


/** Handler class for characteristic actions */
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        Serial.printf("%s : onRead(), value: %s\n",
                      pCharacteristic->getUUID().toString().c_str(),
                      pCharacteristic->getValue().c_str());
                      pCharacteristic->notify();
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
        // Serial.printf("Notification/Indication return code: %d, %s\n", code, NimBLEUtils::returnCodeToString(code));
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
  indoorBikeData.setSpeed(rotor.getVelocityMs());
  indoorBikeData.setCadence(rotor.getCadence());
  uint8_t buf[20];
  uint8_t len = indoorBikeData.build(buf);
  pIndoorBikeCharacterisic->setValue(buf, len);
  pIndoorBikeCharacterisic->notify();
  indoorBikeData.print();
}

void sendPower() {
    float velocidade = rotor.getVelocityMs();
    int fakePower = int(velocidade * 40);
    //  Prepare BLE notification packet
    uint8_t data[4];
    data[0] = 0x00;  // Flags LSB
    data[1] = 0x00;  // Flags MSB
    data[2] = fakePower & 0xFF;
    data[3] = (fakePower >> 8) & 0xFF;
   cyclePowerMesuremento->setValue(data, 4);
   cyclePowerMesuremento->notify(); 

}

unsigned long lastSent = 0;
void send() {
  unsigned long int now = millis();
  if(now - lastSent >= 1000) {
    lastSent = now;
    sendIndoorBikeData();
    sendPower();
  }
}

unsigned long lastPrint = 0;
void printData() {
  unsigned long int now = millis();
  if(now - lastPrint <= 5000) {
    return;
  }
  lastPrint = now;
  fitnessMachineControlPoint.print();
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

void setupCycleService() {
    pCyclePowerService = pServer->createService("1818");
    cyclePowerFeature = pFITMachineService->createCharacteristic("2A65", NIMBLE_PROPERTY::READ);
    cyclePowerMesuremento = pFITMachineService->createCharacteristic("2A63", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    cycleSensorLocation = pFITMachineService->createCharacteristic("2A5D", NIMBLE_PROPERTY::READ);

    uint8_t cycleFeatureData[8];
    cycleFeatureData[0] = 0x00;
    cycleFeatureData[1] = 0x00;
    cycleFeatureData[2] = 0x00;
    cycleFeatureData[3] = 0x08;

    cyclePowerFeature->setValue(cycleFeatureData, 4);
    cyclePowerFeature->setCallbacks(&chrCallbacks);
    
    cycleSensorLocation->setValue(0x0D);
    cycleSensorLocation->setCallbacks(&chrCallbacks);

    pAdvertising->addServiceUUID(pCyclePowerService->getUUID());
} 

void setupFitnessMachine() {
    pFITMachineService = pServer->createService("1826");
    pIndoorBikeCharacterisic = pFITMachineService->createCharacteristic("2AD2", NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::WRITE);
    pFitnessMachineControlPointCharacterisic = pFITMachineService->createCharacteristic("2AD9", NIMBLE_PROPERTY::WRITE);
    pFitnessMachineFeatureCharacteristic = pFITMachineService->createCharacteristic("2ACC", NIMBLE_PROPERTY::READ);

    // pIndoorBikeCaracterisic->setValue("Burger");
    pIndoorBikeCharacterisic->setCallbacks(&chrCallbacks);
    pFitnessMachineControlPointCharacterisic->setCallbacks(&fitnessMachineControlPoint);
    pFitnessMachineFeatureCharacteristic->setCallbacks(&chrCallbacks);


    uint32_t SupportedFeatures = 
        FitnessMachineFeatures::CadenceSupported |
        FitnessMachineFeatures::AverageSpeedSupported |
        FitnessMachineFeatures::PowerMeasurementSupported |
        FitnessMachineFeatures::HeartRateMeasurementSupported;


    uint32_t SupportedTargetSettingFeatures = 
        TargetSettingsFeatures::InclinationTargetSupported;
        
    uint8_t fitnessMachineFlags[8];

    fitnessMachineFlags[0] = (SupportedFeatures >> 0) & 0xFF;
    fitnessMachineFlags[1] = (SupportedFeatures >> 8) & 0xFF;
    fitnessMachineFlags[2] = (SupportedFeatures >> 16) & 0xFF;
    fitnessMachineFlags[3] = (SupportedFeatures >> 24) & 0xFF;
    fitnessMachineFlags[4] = (SupportedTargetSettingFeatures >> 0) & 0xFF;
    fitnessMachineFlags[5] = (SupportedTargetSettingFeatures >> 8) & 0xFF;
    fitnessMachineFlags[6] = (SupportedTargetSettingFeatures >> 16) & 0xFF;
    fitnessMachineFlags[7] = (SupportedTargetSettingFeatures >> 24) & 0xFF;

    pFitnessMachineFeatureCharacteristic->setValue(fitnessMachineFlags, 8);

    pAdvertising->addServiceUUID(pFITMachineService->getUUID());
}

void setup(void) {
    Serial.begin(115200);
    Serial.printf("Starting NimBLE Server\n");
    rotor.init();

    NimBLEDevice::init("EBike Juncowski");

    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(&serverCallbacks); 
    pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->setName("EBike Juncowski");
   
    setupFitnessMachine();
    setupCycleService();

    /**
     *  If your device is battery powered you may consider setting scan response
     *  to false as it will extend battery life at the expense of less data sent.
     */
    pAdvertising->enableScanResponse(true);
    pAdvertising->start();

    Serial.printf("Advertising Started\n");
}

void loop() {
    rotor.loop();
    send();
    checkClients();
    printData();
}