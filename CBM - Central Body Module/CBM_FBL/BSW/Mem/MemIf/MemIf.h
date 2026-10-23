#ifndef MEMIF_H
#define MEMIF_H
#include <stdint.h>
#define MEMIF_IMAGE_SIZE 1024U
#define MEMIF_JOB_OK 0U
#define MEMIF_JOB_FAILED 1U
#define MEMIF_JOB_PENDING 2U
#define MEMIF_BLOCK_INVALID 3U
void MemIf_Init(void);
int MemIf_Read(uint8_t device,uint16_t block,uint16_t offset,uint8_t *data,uint16_t length);
int MemIf_Write(uint8_t device,uint16_t block,const uint8_t *data);
int MemIf_InvalidateBlock(uint8_t device,uint16_t block);
uint8_t MemIf_GetJobResult(uint8_t device);
uint8_t MemIf_GetStatus(uint8_t device);
void MemIf_MainFunction(void);
#endif
