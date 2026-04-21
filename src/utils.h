
#ifndef UTILS_H2
#define UTILS_H2
#include<Arduino.h>

uint16_t readUint16LE(const uint8_t* d, size_t offset);
int16_t readInt16LE(const uint8_t* d, size_t offset);
uint32_t readUint24LE(const uint8_t* d, size_t offset);
void writeUint16LE(uint8_t* d, size_t offset, uint16_t v);
void writeInt16LE(uint8_t* d, size_t offset, int16_t v);
void writeUint24LE(uint8_t* d, size_t offset, uint32_t v);
#endif