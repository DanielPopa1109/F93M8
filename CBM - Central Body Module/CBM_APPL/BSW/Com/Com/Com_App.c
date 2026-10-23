#include "Com.h"
#include "InputCtrl.h"
#include "ActCtrl.h"
#include "main.h"
extern uint8 StatusBodyControl_FanValue,StatusBodyControl_Temperature,StatusBodyControl_OutsideTemp,StatusBodyControl_Auto,StatusBodyControl_Recirc;
extern uint32 CanSpi_StatusBodyControl2_MissCnt,CanSpi_StatusBodyControl_MissCnt,CanSpi_VehicleState_MissCnt;
void Com_AppRx(uint16_t id) {
 uint32_t v;
 if(id==0x97) {(void)Com_ReceiveSignal(0,&v);VehicleSpeed_VehicleState=(uint8)v;if(Com_IsSignalValid(0))CanSpi_VehicleState_MissCnt=0;}
 if(id==0x98) {(void)Com_ReceiveSignal(1,&v);StatusBodyControl_OutsideTemp=(uint8)v;(void)Com_ReceiveSignal(2,&v);StatusBodyControl_Auto=(uint8)v;(void)Com_ReceiveSignal(3,&v);StatusBodyControl_Recirc=(uint8)v;if(Com_IsSignalValid(1))CanSpi_StatusBodyControl_MissCnt=0;}
 if(id==0x99) {(void)Com_ReceiveSignal(4,&v);StatusBodyControl_FanValue=(uint8)v;(void)Com_ReceiveSignal(5,&v);StatusBodyControl_Temperature=(uint8)v;(void)Com_ReceiveSignal(6,&v);WiperStock_VehicleState=(uint8)v;if(Com_IsSignalValid(4))CanSpi_StatusBodyControl2_MissCnt=0;}
}
void Com_AppTx(void) {
 uint32_t v;v=StatusList_ComOutValue[0];(void)Com_SendSignal(7,&v);v=StatusList_ComOutValue[2];(void)Com_SendSignal(8,&v);
 v=CmdList_ActualValue[2];(void)Com_SendSignal(9,&v);v=CmdList_ActualValue[3];(void)Com_SendSignal(10,&v);
 v=StatusList_ComOutValue[4];(void)Com_SendSignal(11,&v);v=HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0);(void)Com_SendSignal(12,&v);v=StatusList_ComOutValue[1];(void)Com_SendSignal(13,&v);
}
