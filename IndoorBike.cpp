#include "IndoorBike.h"

IndoorBike::IndoorBike() {    
  instantaneousSpeed   = IBD_NO_SPEED;
  averageSpeed         = IBD_NO_SPEED;
  instantaneousCadence = IBD_NO_CADENCE;
  averageCadence       = IBD_NO_CADENCE;
  totalDistance        = IBD_NO_DISTANCE;
  resistanceLevel      = IBD_NO_RESISTANCE;
  instantaneousPower   = IBD_NO_POWER;
  averagePower         = IBD_NO_POWER;
  totalEnergy          = IBD_NO_ENERGY;
  energyPerHouX        = IBD_NO_ENERGY;
  energyPerMinute      = IBD_NO_ENERGY;
  heartRate            = IBD_NO_HEART_RATE;
  metabolicEquivalent  = IBD_NO_MET;
  elapsedTime          = IBD_NO_TIME;
  remainingTime        = IBD_NO_TIME;
}

void IndoorBike::setSpeed(uint16_t speed) {
  instantaneousSpeed =  speed*3.6;
  instantaneousPower = pow(speed,3)*0.15;
}

void IndoorBike::setTempoPorVoltaMicros(unsigned long microsegundos) {
  instantaneousCadence = 60000000 / microsegundos;
}

void IndoorBike::print() {
  Serial.printf("Sent speed=%.2f  cadence=%.1f  power=%d  hr=%d  dist=%d  elapsed=%ds\n",
    instantaneousSpeed,
    instantaneousCadence,
    instantaneousPower,
    heartRate,
    totalDistance,
    elapsedTime
  );
}
  
uint8_t IndoorBike::build(uint8_t* buf) {
  uint16_t flags = 0;
  uint8_t  idx   = 2; // reserve first 2 bytes for flags

  // --- helpers: write little-endian integers into buf ---
  auto writeU8  = [&](uint8_t  v){ buf[idx++] = v; };
  auto writeU16 = [&](uint16_t v){ buf[idx++] = v & 0xFF; buf[idx++] = (v >> 8) & 0xFF; };
  auto writeS16 = [&](int16_t  v){ writeU16((uint16_t)v); };
  auto writeU24 = [&](uint32_t v){ buf[idx++] = v & 0xFF;
                                   buf[idx++] = (v >> 8) & 0xFF;
                                   buf[idx++] = (v >> 16) & 0xFF; };

  // Bit 0 — instantaneous speed (0 = present, inverted logic)
  if (!isnan(instantaneousSpeed)) {
    flags &= ~(1 << 0); // bit 0 = 0  →  field IS present
    writeU16((uint16_t)(instantaneousSpeed * 100));
  } else {
    flags |= (1 << 0);  // bit 0 = 1  →  field NOT present
  }

  // Bit 1 — average speed
  if (!isnan(averageSpeed)) {
    flags |= (1 << 1);
    writeU16((uint16_t)(averageSpeed * 100));
  }

  // Bit 2 — instantaneous cadence
  if (!isnan(instantaneousCadence)) {
    flags |= (1 << 2);
    writeU16((uint16_t)(instantaneousCadence * 2));
  }

  // Bit 3 — average cadence
  if (!isnan(averageCadence)) {
    flags |= (1 << 3);
    writeU16((uint16_t)(averageCadence * 2));
  }

  // Bit 4 — total distance (uint24)
  if (totalDistance >= 0) {
    flags |= (1 << 4);
    writeU24((uint32_t)totalDistance);
  }

  // Bit 5 — resistance level (sint16)
  if (resistanceLevel != INT16_MIN) {
    flags |= (1 << 5);
    writeS16(resistanceLevel);
  }

  // Bit 6 — instantaneous power (sint16)
  if (instantaneousPower != INT16_MIN) {
    flags |= (1 << 6);
    writeS16(instantaneousPower);
  }

  // Bit 7 — average power (sint16)
  if (averagePower != INT16_MIN) {
    flags |= (1 << 7);
    writeS16(averagePower);
  }

  // Bit 8 — expended energy: 3 fields written together (total / per hour / per minute)
  if (totalEnergy != INT16_MIN && energyPerHouX != INT16_MIN && energyPerMinute != INT16_MIN) {
    flags |= (1 << 8);
    writeS16(totalEnergy);
    writeS16(energyPerHouX);
    writeU8((uint8_t)energyPerMinute); // per-minute is uint8 per spec
  }

  // Bit 9 — heart rate (uint8)
  if (heartRate != IBD_NO_HEART_RATE) {
    flags |= (1 << 9);
    writeU8(heartRate);
  }

  // Bit 10 — metabolic equivalent (uint16, resolution 0.1)
  if (!isnan(metabolicEquivalent)) {
    flags |= (1 << 10);
    writeU16((uint16_t)(metabolicEquivalent * 10));
  }

  // Bit 11 — elapsed time (uint16, seconds)
  if (elapsedTime >= 0) {
    flags |= (1 << 11);
    writeU16((uint16_t)elapsedTime);
  }

  // Bit 12 — remaining time (uint16, seconds)
  if (remainingTime >= 0) {
    flags |= (1 << 12);
    writeU16((uint16_t)remainingTime);
  }

  // Write flags into the reserved first 2 bytes
  buf[0] = flags & 0xFF;
  buf[1] = (flags >> 8) & 0xFF;

  return idx; // total frame length
}
