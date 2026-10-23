#ifndef STM32_FLS_H
#define STM32_FLS_H
#include <stdint.h>
#define FLS_BANK0 0x08020000U
#define FLS_BANK1 0x08040000U
#define FLS_SIZE0 0x20000U
#define FLS_SIZE1 0x20000U
int Fls_Read(uint32_t address, void *data, uint32_t size);
int Fls_Program(uint32_t address, const void *data, uint32_t size);
int Fls_EraseBank(uint8_t bank);
#endif
