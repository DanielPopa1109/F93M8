/* STM32F411 sector controller; no STM32F1 page erase logic or HAL calls. */
#include "Fbl_Ram.h"
#include "stm32f4xx.h"
#define ERRORS (FLASH_SR_SOP|FLASH_SR_WRPERR|FLASH_SR_PGAERR|FLASH_SR_PGPERR|FLASH_SR_PGSERR)
static RAM_CODE int waitReady(uint32_t timeout) {
 uint32_t begin=Ram_Millis(),budget=100000000U;
 while(FLASH->SR&FLASH_SR_BSY)if((uint32_t)(Ram_Millis()-begin)>timeout || !--budget || Ram_KeepAlive())return 1;
 return (FLASH->SR&ERRORS)?1:0;
}
static RAM_CODE int unlock(void) {
 if(FLASH->SR&FLASH_SR_BSY)return 1;
 FLASH->SR=ERRORS|FLASH_SR_EOP;
 if(FLASH->CR&FLASH_CR_LOCK){FLASH->KEYR=0x45670123;FLASH->KEYR=0xCDEF89AB;}
 return (FLASH->CR&FLASH_CR_LOCK)?1:0;
}
static RAM_CODE void invalidate(void) {
 /* Disable and reset both ART caches before any readback of modified Flash. */
 uint32_t acr=FLASH->ACR;
 FLASH->ACR=acr&~(FLASH_ACR_ICEN|FLASH_ACR_DCEN);
 FLASH->ACR=(acr&~(FLASH_ACR_ICEN|FLASH_ACR_DCEN))|FLASH_ACR_ICRST|FLASH_ACR_DCRST;
 FLASH->ACR=acr;__asm volatile("dsb\n isb":::"memory");
}
RAM_CODE int Ram_Erase(void) {
 int error=0;if(unlock())return 1;
 for(uint32_t sector=0;sector<RAM_SECTORS;sector++){
  /* x8 PSIZE works across the documented programming supply range. */
  FLASH->CR=FLASH_CR_SER|(sector<<FLASH_CR_SNB_Pos);
  FLASH->CR|=FLASH_CR_STRT;
  if(waitReady(5000)){error=1;break;}
  FLASH->CR=0;
 }
 if(!(FLASH->SR&FLASH_SR_BSY))FLASH->CR=FLASH_CR_LOCK;
 if(error)return 1;invalidate();
 for(uint32_t at=RAM_FLASH_BASE;at<RAM_FLASH_BASE+FBL_RESIDENT_SIZE;at+=4)
  if(*(volatile uint32_t *)(uintptr_t)at!=0xFFFFFFFFU)return 1;
 return 0;
}
RAM_CODE int Ram_Program(uint32_t at,const uint8_t *p,uint32_t n) {
 int error=0;
 if(!p || !n || at<RAM_FLASH_BASE || at>=RAM_FLASH_BASE+FBL_RESIDENT_SIZE || n>RAM_FLASH_BASE+FBL_RESIDENT_SIZE-at)return 1;
 if(unlock())return 1;
 for(uint32_t i=0;i<n;i++){
  FLASH->CR=FLASH_CR_PG;*(volatile uint8_t *)(uintptr_t)(at+i)=p[i];
  if(waitReady(100)){error=1;break;}
  FLASH->CR=0;
 }
 if(!(FLASH->SR&FLASH_SR_BSY))FLASH->CR=FLASH_CR_LOCK;
 if(error)return 1;invalidate();
 for(uint32_t i=0;i<n;i++)if(*(volatile uint8_t *)(uintptr_t)(at+i)!=p[i])return 1;
 return 0;
}
RAM_CODE int Ram_Read(uint32_t at,uint8_t *p,uint32_t n) {
 if(!p || at<RAM_FLASH_BASE || at>=RAM_FLASH_BASE+FBL_RESIDENT_SIZE || n>RAM_FLASH_BASE+FBL_RESIDENT_SIZE-at || (FLASH->SR&FLASH_SR_BSY))return 1;
 for(uint32_t i=0;i<n;i++)p[i]=*(volatile uint8_t *)(uintptr_t)(at+i);return 0;
}
