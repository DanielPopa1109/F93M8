#include "NvM_Engine.h"
static uint8_t dem[512],calibration[40],coding[24];
const NvM_BlockDescriptorType NvM_BlockDescriptor[NVM_BLOCK_COUNT]={
 {512,2,dem,0,1,1},{40,1,calibration,0,1,1},{24,1,coding,0,1,1}
};
