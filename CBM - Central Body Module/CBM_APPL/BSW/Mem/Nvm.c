#include "Nvm.h"
#include "EcuM.h"
static uint8 started,postrun;
void NvM_MainFunction(void) {
 if(!started) { NvM_Init(); NvM_ReadAll(); started=1U; }
 if(EcuM_State==RUN) postrun=0U;
 if(EcuM_State==POSTRUN && !postrun && NvM_IsIdle()) { NvM_WriteAll(); postrun=1U; }
 NvM_EngineMainFunction();
}
