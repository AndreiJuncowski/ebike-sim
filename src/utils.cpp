#include <Arduino.h>
#include "utils.h"

uint16_t readUint16LE(const uint8_t* d, size_t offset) {
    return static_cast<uint16_t>(d[offset]) |
            (static_cast<uint16_t>(d[offset + 1]) << 8);
}

int16_t readInt16LE(const uint8_t* d, size_t offset) {
    return static_cast<int16_t>(readUint16LE(d, offset));
}

uint32_t readUint24LE(const uint8_t* d, size_t offset) {
    return static_cast<uint32_t>(d[offset]) |
            (static_cast<uint32_t>(d[offset + 1]) << 8) |
            (static_cast<uint32_t>(d[offset + 2]) << 16);
}

void writeUint16LE(uint8_t* d, size_t offset, uint16_t v) {
    d[offset]     = static_cast<uint8_t>(v & 0xFF);
    d[offset + 1] = static_cast<uint8_t>((v >> 8) & 0xFF);
}

void writeInt16LE(uint8_t* d, size_t offset, int16_t v) {
    writeUint16LE(d, offset, static_cast<uint16_t>(v));
}

void writeUint24LE(uint8_t* d, size_t offset, uint32_t v) {
    d[offset]     = static_cast<uint8_t>(v & 0xFF);
    d[offset + 1] = static_cast<uint8_t>((v >> 8)  & 0xFF);
    d[offset + 2] = static_cast<uint8_t>((v >> 16) & 0xFF);
}
