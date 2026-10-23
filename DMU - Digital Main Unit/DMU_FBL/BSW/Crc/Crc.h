#ifndef CRC_H
#define CRC_H
#include <stdint.h>
uint32_t Crc_CalculateCRC32(const uint8_t *data, uint32_t length, uint32_t start, uint8_t first);
#endif
