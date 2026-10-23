#include "Fls.h"
#include "main.h"
#include <string.h>
/* Only journal sectors are writable. Legacy and application regions are read-only here. */
static int writable(uint32_t a,uint32_t n) {
    return n!=0U && ((a>=FLS_BANK0 && a<FLS_BANK0+FLS_SIZE0 && n<=FLS_BANK0+FLS_SIZE0-a) ||
        (a>=FLS_BANK1 && a<FLS_BANK1+FLS_SIZE1 && n<=FLS_BANK1+FLS_SIZE1-a));
}
int Fls_Read(uint32_t address,void *data,uint32_t size) {
    if (!data || address<0x08000000U || address>=0x08080000U || size>0x08080000U-address) return 1;
    memcpy(data,(const void *)(uintptr_t)address,size); return 0;
}
int Fls_Program(uint32_t address,const void *data,uint32_t size) {
    uint32_t offset,word; int error=0;
    if (!data || (address&3U) || (size&3U) || !writable(address,size)) return 1;
    if (HAL_FLASH_Unlock()!=HAL_OK) { (void)HAL_FLASH_Lock(); return 1; }
    for(offset=0;offset<size;offset+=4U) {
        memcpy(&word,(const uint8_t *)data+offset,4U);
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,address+offset,word)!=HAL_OK ||
            *(volatile uint32_t *)(uintptr_t)(address+offset)!=word) { error=1; break; }
    }
    if(HAL_FLASH_Lock()!=HAL_OK) error=1;
    return error;
}
int Fls_EraseBank(uint8_t bank) {
    FLASH_EraseInitTypeDef erase={0}; uint32_t sectorError=0,off,base,size; int error=0;
    if(bank>1U) return 1;
    base=bank?FLS_BANK1:FLS_BANK0; size=bank?FLS_SIZE1:FLS_SIZE0;
    erase.TypeErase=FLASH_TYPEERASE_SECTORS; erase.VoltageRange=FLASH_VOLTAGE_RANGE_3;
    erase.Sector=bank?6U:5U; erase.NbSectors=1U;
    if(HAL_FLASH_Unlock()!=HAL_OK) { (void)HAL_FLASH_Lock(); return 1; }
    if(HAL_FLASHEx_Erase(&erase,&sectorError)!=HAL_OK || sectorError!=0xFFFFFFFFU) error=1;
    if(HAL_FLASH_Lock()!=HAL_OK) error=1;
    if(!error) for(off=0;off<size;off+=4U)
        if(*(volatile uint32_t *)(uintptr_t)(base+off)!=0xFFFFFFFFU) { error=1; break; }
    return error;
}
