#ifndef FBL_UPDATER_H
#define FBL_UPDATER_H
#include <stdint.h>
#include "Fbl_Ram.h"
int FblUpdater_Begin(uint32_t size);
int FblUpdater_Write(uint32_t offset,const uint8_t *data,uint16_t length);
int FblUpdater_Validate(uint32_t crc);
int FblUpdater_Activate(void);
void FblUpdater_Init(void);
#endif
