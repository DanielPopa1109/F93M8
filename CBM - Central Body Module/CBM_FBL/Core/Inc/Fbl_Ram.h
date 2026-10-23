#ifndef FBL_RAM_H
#define FBL_RAM_H
#include <stdint.h>
#include "Fbl.h"
#ifdef RAM_HOST_TEST
#define RAM_CODE
#define RAM_DATA
#define RAM_BSS
#else
#define RAM_CODE __attribute__((section(".ram_blob_text"),noinline))
#define RAM_DATA __attribute__((section(".ram_blob_data")))
#define RAM_BSS __attribute__((section(".ram_blob_bss")))
#endif
#define RAM_TARGET 0x54U
#define RAM_RESIDENT_VECTOR_WORDS 102U
#define RAM_MCU_ID 0x0411U
#define RAM_IMAGE_TAG_OFFSET 512U
#define RAM_IMAGE_TAG_BYTES 32U
#define RAM_IMAGE_MAGIC 0x46424C34U
#define RAM_BASE 0x20000000U
#define RAM_STACK_BOTTOM 0x2000F000U
#define RAM_STACK_TOP 0x20010000U
#define RAM_RESIDENT_STACK_TOP 0x2001FC00U
#define RAM_FLASH_BASE 0x08000000U
#define RAM_SECTORS (FBL_RESIDENT_SIZE/16384U)
/* Deployment interlock: controlled builds explicitly override this definition.
 * Production enablement additionally requires independently demonstrated SWD/ROM recovery. */
#ifndef FBL_UPDATER_ACTIVATION_ALLOWED
#define FBL_UPDATER_ACTIVATION_ALLOWED 0
#endif
int FblUpdater_ValidateLayout(uint32_t load,uint32_t loadEnd,uint32_t start,uint32_t end,
                              uint32_t sp,uint32_t pc);
int FblUpdater_Prepare(void);
RAM_CODE void Ram_Init(void);
RAM_CODE void Ram_OnCan(uint16_t id,uint8_t dlc,const uint8_t *p);
RAM_CODE void Ram_Tick(uint32_t elapsedMs);
RAM_CODE void Ram_Start(void);
RAM_CODE int Ram_CanInit(void);
RAM_CODE int Ram_CanReceive(uint16_t *id,uint8_t *dlc,uint8_t *p);
/* Success means physical TX completion, including controller error checks. */
RAM_CODE int Ram_CanSend(const uint8_t *p);
RAM_CODE int Ram_Erase(void);
RAM_CODE int Ram_Program(uint32_t at,const uint8_t *p,uint32_t n);
RAM_CODE int Ram_Read(uint32_t at,uint8_t *p,uint32_t n);
RAM_CODE void Ram_Reset(void);
RAM_CODE uint32_t Ram_Millis(void);
RAM_CODE int Ram_KeepAlive(void);
#endif
