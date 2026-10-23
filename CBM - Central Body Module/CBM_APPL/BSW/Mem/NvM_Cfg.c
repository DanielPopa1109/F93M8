#include "NvM_Engine.h"
#include "Fls/Fls.h"
#include "../Diag/Dem/Dem.h"
#include <string.h>
uint8_t NvM_DemImage[512];
#include "InputCtrl.h"
#include "ActCtrl.h"
#include "EcuM.h"
uint32 NvMBlock_Calibration[10],NvMBlock_Coding[6];
static const uint32 defaultsCal[10]={1,100,1,20,255,5,25,15,5,0};
static const uint32 defaultsCoding[6]={2,2,2,2,75,4};
const NvM_BlockDescriptorType NvM_BlockDescriptor[NVM_BLOCK_COUNT]={
 {512,2,NvM_DemImage,0,1,1},
 {40,1,(uint8_t *)NvMBlock_Calibration,(const uint8_t *)defaultsCal,1,1},
 {24,1,(uint8_t *)NvMBlock_Coding,(const uint8_t *)defaultsCoding,1,1}
};
void NvM_ApplicationMigrate(uint16_t b) {
 uint32 first,address=b==1U?0x08010000U:(b==2U?0x08008000U:0x0800C000U);
 if(Fls_Read(address,&first,4U) || first==0xFFFFFFFFU) return;
 if(b==1U) { (void)Fls_Read(address,Dem_DTCStoreArray,sizeof(Dem_DTCStoreArray)); Dem_ValidateStoredDtc(); }
 else (void)Fls_Read(address,NvM_BlockDescriptor[b-1].ram,NvM_BlockDescriptor[b-1].length);
}
void NvM_ApplicationRestore(void) {
 Dem_Init(&Dem_Config);
 if(Dem_Deserialize(NvM_DemImage)==0U) {
  /* Reset starts a new operation cycle while retaining confirmed/pending history. */
  Dem_StartRestoredCycle();
 }
 for(uint8 i=0;i<4U;++i) Param_Input_Calibration[i]=(uint16)NvMBlock_Calibration[i];
 Param_ErrorSettingDebouceThreshold_Calibration=NvMBlock_Calibration[4];
 Param_UndervoltageThreshold_Calibration=NvMBlock_Calibration[5];
 Param_WindshieldWiperFast_Calibration=NvMBlock_Calibration[6];
 Param_WindshieldWiperSlow_Calibration=NvMBlock_Calibration[7];
 Param_DoorLockPwmIncrement=NvMBlock_Calibration[8];
 for(uint8 i=0;i<4U;++i) InputCtrl_CodingData[i]=NvMBlock_Coding[i];
 WindshieldWashing_CodingData=NvMBlock_Coding[4]; WashingWipingCounter_CodingData=NvMBlock_Coding[5];
}
void NvM_ApplicationCapture(void) {
 Dem_Serialize(NvM_DemImage);
 for(uint8 i=0;i<4U;++i) { NvMBlock_Calibration[i]=Param_Input_Calibration[i]; NvMBlock_Coding[i]=InputCtrl_CodingData[i]; }
 NvMBlock_Calibration[4]=Param_ErrorSettingDebouceThreshold_Calibration;
 NvMBlock_Calibration[5]=Param_UndervoltageThreshold_Calibration;
 NvMBlock_Calibration[6]=Param_WindshieldWiperFast_Calibration;
 NvMBlock_Calibration[7]=Param_WindshieldWiperSlow_Calibration;
 NvMBlock_Calibration[8]=Param_DoorLockPwmIncrement;
 NvMBlock_Coding[4]=WindshieldWashing_CodingData; NvMBlock_Coding[5]=WashingWipingCounter_CodingData;
}
