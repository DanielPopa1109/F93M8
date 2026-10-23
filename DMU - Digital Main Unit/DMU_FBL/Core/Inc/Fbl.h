#ifndef FBL_H
#define FBL_H
#include <stdint.h>
#define FBL_HAS_PARAMETER_BLOCKS 0
#define FBL_APP_START 0x8020000U
#define FBL_APP_END 0x8060000U
#define FBL_METADATA (FBL_APP_END-1024U)
#define FBL_RESIDENT_SIZE 32768U
#define FBL_BOOT_MAILBOX 0x1000FFF0U
void Fbl_Init(void);
void Fbl_SetInitialSession(uint8_t session);
void Fbl_MainFunction(void);
void Fbl_RxIndication(const uint8_t *p,uint16_t n);
void Fbl_TxConfirmation(uint8_t success);
uint8_t Fbl_ApplicationValid(void);
int Fbl_BoardRead(uint32_t address,void *p,uint32_t length);
int Fbl_BoardProgram(uint32_t address,const void *p,uint32_t length);
int Fbl_BoardErase(void);
int Fbl_BoardCanTransmit(const uint8_t *p);
void Fbl_BoardReset(void);
void Fbl_BoardJump(void);
#endif
