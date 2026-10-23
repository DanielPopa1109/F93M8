#include "NvM_Engine.h"
#include "Fls/Fls.h"
#include "../Diag/Dem/Dem.h"
#include <string.h>
uint8_t NvM_DemImage[512];
extern uint32 DataRecorder_KilometerTotal,DataRecorder_KilometerPerDcy;
uint32 Nvm_DataRecorder_CommonBlock[2];
const NvM_BlockDescriptorType NvM_BlockDescriptor[NVM_BLOCK_COUNT]={
 {512,2,NvM_DemImage,0,1,1}, {8,1,(uint8_t *)Nvm_DataRecorder_CommonBlock,0,1,1}
};
void NvM_ApplicationMigrate(uint16_t b) {
 uint32 first,address=b==1U?0x0800C000U:0x08008000U;
 if(Fls_Read(address,&first,4U)||first==0xFFFFFFFFU) return;
 if(b==1U) { (void)Fls_Read(address,Dem_DTCArray,sizeof(Dem_DTCArray)); Dem_ValidateStoredDtc(); }
 else (void)Fls_Read(address,Nvm_DataRecorder_CommonBlock,8U);
}
void NvM_ApplicationRestore(void) {
 Dem_Init(&Dem_Config);
 if(Dem_Deserialize(NvM_DemImage)==0U) {
  /* Reset starts a new operation cycle while retaining confirmed/pending history. */
  Dem_StartRestoredCycle();
 }
 DataRecorder_KilometerTotal=Nvm_DataRecorder_CommonBlock[0];
 DataRecorder_KilometerPerDcy=Nvm_DataRecorder_CommonBlock[1];
}
void NvM_ApplicationCapture(void) {
 Dem_Serialize(NvM_DemImage);
 Nvm_DataRecorder_CommonBlock[0]=DataRecorder_KilometerTotal;
 Nvm_DataRecorder_CommonBlock[1]=DataRecorder_KilometerPerDcy;
}
