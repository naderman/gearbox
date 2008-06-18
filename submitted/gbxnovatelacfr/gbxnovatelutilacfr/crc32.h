#include <cstddef> //for size_t
#include <stdint.h> //for fixed size integer types

namespace gbxnovatelutilacfr {
//calculate and return a 32bit CRC for buf[bufLen]
uint32_t crc(uint8_t *buf, size_t bufLen);
}
