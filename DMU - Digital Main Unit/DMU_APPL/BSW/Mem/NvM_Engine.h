#ifndef NVM_ENGINE_H
#define NVM_ENGINE_H
#include <stdint.h>
#define NVM_BLOCK_COUNT 2U
#define NVM_BLOCK_DEM 1U
#define NVM_MAX_BLOCK_LENGTH 512U
#define NVM_REQ_OK 0U
#define NVM_REQ_NOT_OK 1U
#define NVM_REQ_PENDING 2U
#define NVM_REQ_RESTORED_FROM_ROM 3U
#define NVM_REQ_NV_INVALIDATED 4U
typedef uint16_t NvM_BlockIdType;
typedef uint8_t NvM_RequestResultType;
typedef struct { uint16_t length,version; uint8_t *ram; const uint8_t *defaults; uint8_t readAll,writeAll; } NvM_BlockDescriptorType;
extern const NvM_BlockDescriptorType NvM_BlockDescriptor[NVM_BLOCK_COUNT];
void NvM_Init(void);
void NvM_ReadAll(void);
void NvM_WriteAll(void);
uint8_t NvM_ReadBlock(NvM_BlockIdType block,void *dst);
uint8_t NvM_WriteBlock(NvM_BlockIdType block,const void *src);
uint8_t NvM_SetRamBlockStatus(NvM_BlockIdType block,uint8_t changed);
uint8_t NvM_GetErrorStatus(NvM_BlockIdType block,NvM_RequestResultType *result);
uint8_t NvM_InvalidateNvBlock(NvM_BlockIdType block);
uint8_t NvM_RestoreBlockDefaults(NvM_BlockIdType block,void *dst);
void NvM_EngineMainFunction(void);
uint8_t NvM_IsIdle(void);
void NvM_ApplicationCapture(void);
void NvM_MainFunction(void);
void NvM_ApplicationRestore(void);
void NvM_ApplicationMigrate(uint16_t block);
extern uint8_t NvM_DemImage[512];
extern uint8_t NvM_Ready;
extern uint32_t NvM_LastWriteResult;
#endif
