#ifndef INDOOR_BIKE_H
#define INDOOR_BIKE_H
#include    <Arduino.h>

#define IBD_NO_SPEED        NAN
#define IBD_NO_CADENCE      NAN
#define IBD_NO_MET          NAN
#define IBD_NO_DISTANCE     (-1)
#define IBD_NO_RESISTANCE   INT16_MIN
#define IBD_NO_POWER        INT16_MIN
#define IBD_NO_ENERGY       INT16_MIN
#define IBD_NO_HEART_RATE   (0)
#define IBD_NO_TIME         (-1)


class IndoorBike {
  public:
    IndoorBike();
    uint8_t build(uint8_t* buf);
    void setSpeed(uint16_t speed);
    void setCadence(float cadence);
    void print();
    
   private:
    float   instantaneousSpeed;   // km/h  — resolution 0.01
    float   averageSpeed;         // km/h  — resolution 0.01
    float   instantaneousCadence; // rpm   — resolution 0.5
    float   averageCadence;       // rpm   — resolution 0.5
    int32_t totalDistance;        // m     — uint24, -1 to omit
    int16_t resistanceLevel;      // unitless sint16, INT16_MIN to omit
    int16_t instantaneousPower;   // W     — INT16_MIN to omit
    int16_t averagePower;         // W     — INT16_MIN to omit
    int16_t totalEnergy;          // kJ    — INT16_MIN to omit  
    int16_t energyPerHouX;        // kJ/h  — INT16_MIN to omit   > all 3 or none
    int16_t energyPerMinute;      // kJ/min— INT16_MIN to omit  
    uint8_t heartRate;            // bpm   — 0 to omit
    float   metabolicEquivalent;  // MET   — resolution 0.1, NAN to omit
    int16_t elapsedTime;          // s     — -1 to omit
    int16_t remainingTime;        // s     — -1 to omit
};

#endif
