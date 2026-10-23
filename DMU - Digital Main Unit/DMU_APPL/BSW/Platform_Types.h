#ifndef DMU_PLATFORM_TYPES_H
#define DMU_PLATFORM_TYPES_H

#include <stdint.h>

/* Cortex-M4 / GNU C data model used by this standalone STM32 project. */
#define CPU_TYPE_8 8U
#define CPU_TYPE_16 16U
#define CPU_TYPE_32 32U
#define CPU_TYPE CPU_TYPE_32
#define MSB_FIRST 0U
#define LSB_FIRST 1U
#define HIGH_BYTE_FIRST 0U
#define LOW_BYTE_FIRST 1U
#define CPU_BIT_ORDER LSB_FIRST
#define CPU_BYTE_ORDER LOW_BYTE_FIRST

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef int8_t sint8;
typedef int16_t sint16;
typedef int32_t sint32;
typedef uint8_t boolean;

#endif
