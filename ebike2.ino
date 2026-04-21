
#include <Arduino.h>
//#include <BleGamepad.h>
#include <NimBLEDevice.h>
#include "IndoorBike.h"
int sensor = 19;
const uint16_t FLAGS = 0b0000000001000100;//0x0044;


// Adjustable range
const int MIN_POWER = 50;  // Minimum power in watts
const int MAX_POWER = 500;  // Maximum power in watts

// Potentiometer pin
const int POT_PIN = 34;

void sendIndoorBikeData();

// BLE objects
NimBLEServer* server;
NimBLEService* powerService;
NimBLECharacteristic* powerChar;
NimBLECharacteristic* pIndoorBikeData = nullptr;
NimBLEService* indoorBikeService;

void setup()
{
    pinMode (sensor, INPUT_PULLUP);
    Serial.begin(115200);
    Serial.println("Starting BLE Fake Meter!");

    NimBLEDevice::init("EBike Juncowski");

    // Create BLE server and service
    server = NimBLEDevice::createServer();
  //powerService = server->createService("1818");  // Cycling Power Service UUID
    indoorBikeService = server->createService("1826"); 
  
    // Create characteristic for Cycling Power Measurement
    // Mapa de notificações blueetooth  https://github.com/nordicsemi/bluetooth-numbers-database/blob/master/v1/characteristic_uuids.json 
//    powerChar = powerService->createCharacteristic(
//      "2A63",  // Cycling Power Measurement UUID
//      NIMBLE_PROPERTY::NOTIFY
////    );
    pIndoorBikeData = indoorBikeService->createCharacteristic(
    "2AD2",
    NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::WRITE
  );
  
    //powerService->start();
    indoorBikeService->start();
  
    // Start advertising
    NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();
    //advertising->addServiceUUID("1818");
    advertising->addServiceUUID("1826");
    advertising->start();
    Serial.println("E-bike power meter started");
//    bleGamepad.begin();
    // The default bleGamepad.begin() above enables 16 buttons, all axes, one hat, and no simulation controls or special buttons
}

boolean anterior = false;
boolean atual = false;
unsigned long voltas = 0;
unsigned long microsAnterior = 0;
unsigned long microsAtual = 0;
unsigned long tempoPercorrido = 0;
unsigned long microsPulsoAnterior = 0;
unsigned long tempoVoltaMicrosegundos = 1;
unsigned long MICROMETRO_POR_VOLTA = 4400000;
float METRO_POR_VOLTA = MICROMETRO_POR_VOLTA * 0.0000001;
float velocidade = 0;

void calculaVelocidade() {
  atual = digitalRead(sensor) == false;
  microsAtual = micros();
  tempoPercorrido = microsAtual - microsAnterior;
  
  if (atual != anterior && tempoPercorrido >= 1000){
    anterior = atual;
    microsAnterior = microsAtual;
    if (atual){
      voltas++;
      tempoVoltaMicrosegundos = (microsAtual - microsPulsoAnterior);
      microsPulsoAnterior = microsAtual;
      velocidade =  float(MICROMETRO_POR_VOLTA) / float(tempoVoltaMicrosegundos);
      // Serial.print(" MILLIS=");Serial.print(tempoVoltaMicrosegundos);  
//      Serial.print(" VELOCIDADE[m/s]=");Serial.print(velocidade);
//      Serial.print(" VELOCIDADE[km/h]=");Serial.print(velocidade * 3.6);
//      Serial.print(" VOLTAS=");Serial.print(voltas);
//      Serial.print(" DISTANCIA=");Serial.print(voltas * METRO_POR_VOLTA);
//      Serial.print("\n");
    }
  }

  if(tempoPercorrido > 3000000 && velocidade > 0) {
    Serial.println("RESET VELOCIDADE");
    velocidade = 0;
  }
}


unsigned long ultimoEnvio = 0;
void fake() {
  unsigned long int agora = millis();
  if(agora - ultimoEnvio >= 1000) {
    ultimoEnvio = agora;
    sendIndoorBikeData();
   // sendPower();
    
  }
}

//void sendPower() {
//  //https://github.com/camilom40/zwift-fake-powermeter/blob/main/zwift-fake-powermeter.ino
//  // 1️Read the potentiometer
//  int fakePower = int(velocidade * 40); //100 + random(0, 200);
//  //  Prepare BLE notification packet
//  uint8_t data[4];
//  data[0] = 0x00;  // Flags LSB
//  data[1] = 0x00;  // Flags MSB
//  data[2] = fakePower & 0xFF;
//  data[3] = (fakePower >> 8) & 0xFF;
//// 6 Send notification
//    powerChar->setValue(data, 4);
//    powerChar->notify(); 
//    Serial.print("NOTIFICA");
//    Serial.println(fakePower);

//}

void sendIndoorBikeData() {
  IndoorBike bike;

  bike.setSpeed(velocidade);
  bike.setTempoPorVoltaMicros(tempoVoltaMicrosegundos);
  uint8_t buf[20];
  uint8_t len = bike.build(buf);
  buf[0] = 'A';
  buf[1] = 'B';
  buf[2] = 'C';
  buf[3] = 'D';
  pIndoorBikeData->setValue(buf, len);
  pIndoorBikeData->notify();
  bike.print();
}


void loop()
{
  calculaVelocidade();
  fake();
}
