#include "Nvm.h"
uint8 Nvm_WriteAllPending,Nvm_ReadAllPending;
uint32 Nvm_LastWriteResult,Nvm_MainCounter;
static uint8 started,readStarted,writeStarted;
void Nvm_WriteAll(void) { NvM_WriteAll(); Nvm_LastWriteResult=2U; }
void Nvm_MainFunction(void) {
 if(!started) { NvM_Init(); started=1U; }
 if(Nvm_ReadAllPending==1U && !readStarted) { NvM_ReadAll(); readStarted=1U; }
 if(Nvm_WriteAllPending==1U && !writeStarted) { Nvm_WriteAll(); writeStarted=1U; }
 NvM_EngineMainFunction(); Nvm_LastWriteResult=NvM_LastWriteResult;
 if(NvM_IsIdle()) {
  if(readStarted) Nvm_ReadAllPending=2U;
  if(writeStarted) Nvm_WriteAllPending=NvM_LastWriteResult==0U?2U:3U;
 }
 if(Nvm_WriteAllPending==0U) writeStarted=0U;
 ++Nvm_MainCounter;
}

/* Canonical BSW entry; preserve the legacy spelling for existing ECU callers. */
void NvM_MainFunction(void) { Nvm_MainFunction(); }
