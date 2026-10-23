#include "Crc.h"
uint32_t Crc_CalculateCRC32(const uint8_t *data, uint32_t length, uint32_t start, uint8_t first)
{
    uint32_t crc = first ? 0xFFFFFFFFU : ~start, i;
    uint8_t bit;
    for (i=0; i<length; ++i) {
        crc ^= data[i];
        for (bit=0; bit<8; ++bit) crc=(crc>>1)^((crc&1U)?0xEDB88320U:0U);
    }
    return ~crc;
}
