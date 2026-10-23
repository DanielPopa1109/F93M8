#include "Com.h"
extern uint8_t CanH_RxSig_Ignition,CanH_RxSig_Gear,CanH_RxSig_IrSensStat,CanH_RxSig_Rpm,CanH_RxSig_Speed;
extern uint8_t CanH_RxSig_Recirculation,CanH_RxSig_FogLights,CanH_RxSig_HighBeam,CanH_RxSig_TemperatureSensor,CanH_RxSig_TurnSignal,CanH_RxSig_AutoClimate;
extern uint8_t CanH_RxSig_FanValue,CanH_RxSig_DisplayMode,CanH_RxSig_ClimaTemp,CanH_RxSig_RotaryLightSwitch,CanH_RxSig_CcmId,CanH_RxSig_DrivecycleStatus;
void Com_AppRx(uint16_t id) {
 uint32_t v;uint16_t i;uint8_t *targets[]={&CanH_RxSig_Ignition,&CanH_RxSig_Gear,&CanH_RxSig_IrSensStat,&CanH_RxSig_Rpm,&CanH_RxSig_Speed,&CanH_RxSig_Recirculation,&CanH_RxSig_FogLights,&CanH_RxSig_HighBeam,&CanH_RxSig_TemperatureSensor,&CanH_RxSig_TurnSignal,&CanH_RxSig_AutoClimate,&CanH_RxSig_FanValue,&CanH_RxSig_DisplayMode,&CanH_RxSig_ClimaTemp,&CanH_RxSig_RotaryLightSwitch,&CanH_RxSig_CcmId,&CanH_RxSig_DrivecycleStatus};
 for(i=0;i<17;++i)if(Com_Pdus[Com_Signals[i].pdu].canId==id){(void)Com_ReceiveSignal(i,&v);*targets[i]=(uint8_t)v;}
}
extern uint32_t DataRecorder_KilometerTotal,DataRecorder_KilometerPerDcy;
void Com_AppTx(void) {
 static uint16_t ticks;static uint8_t alive;uint32_t v;uint8_t i;
 if(++ticks>=207U){ticks=0;++alive;}v=alive;(void)Com_SendSignal(20,&v);
 for(i=0;i<4;++i){v=(DataRecorder_KilometerTotal>>(24U-i*8U))&255U;(void)Com_SendSignal((uint16_t)(21U+i),&v);v=(DataRecorder_KilometerPerDcy>>(24U-i*8U))&255U;(void)Com_SendSignal((uint16_t)(25U+i),&v);}
}
