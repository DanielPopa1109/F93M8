#ifndef NVM_H
#define NVM_H
#include "Std_Types.h"
#include "../NvM_Engine.h"
extern uint32 Nvm_LastWriteResult;
void Nvm_WriteAll(void);
void Nvm_MainFunction(void);
#endif
