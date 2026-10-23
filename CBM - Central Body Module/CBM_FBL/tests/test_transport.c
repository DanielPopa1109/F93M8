#define FBL_REAL_TRANSPORT
#define main programming_suite
#include "test_programming.c"
#undef main
#include "../BSW/Com/Can/Can.h"
#include "CanIf.h"
#include "CanTp.h"
#include "PduR.h"
static uint8_t wire[8];static unsigned sent;
int Test_PhysicalTransmit(const uint8_t *p){memcpy(wire,p,8);++sent;return 0;}
static void input(const uint8_t *p,uint8_t n){Can_FrameType f={0};f.id=0x710;f.dlc=n;memcpy(f.data,p,n);Can_RxIndication(&f);Fbl_MainFunction();}
static void confirm(void){Can_TxConfirmation(1);Fbl_MainFunction();}
int main(void){
 uint8_t f[8]={CANTP_TARGET_ADDRESS,2,0x10,2};unsigned before;
 memset(flash,255,sizeof(flash));Can_Init();CanIf_Init();CanTp_Init();PduR_Init();Fbl_Init();
 input(f,4);CHECK(wire[0]==0xF0&&wire[2]==0x50);CHECK(CanTp_IsBusy());confirm();CHECK(!CanTp_IsBusy());
 /* Reassemble FCD's eleven-byte RequestDownload exactly; its address is rejected safely without prior erase. */
 {uint8_t req[11]={0x34,0,0x44};put32(req+3,FBL_APP_START);put32(req+7,1024);
 f[0]=CANTP_TARGET_ADDRESS;f[1]=0x10;f[2]=11;memcpy(f+3,req,5);input(f,8);CHECK(wire[1]==0x30&&wire[2]==8);confirm();
 f[1]=0x21;memcpy(f+2,req+5,6);input(f,8);CHECK(wire[2]==0x7F&&wire[3]==0x34&&wire[4]==0x22);confirm();}
 before=PduR_GetRxErrorCount();f[0]=CANTP_TARGET_ADDRESS;f[1]=0x10;f[2]=20;input(f,8);confirm();f[1]=0x22;input(f,8);CHECK(PduR_GetRxErrorCount()==before+1);
 /* No physical completion within N_As must become a failed TP confirmation. */
 f[0]=CANTP_TARGET_ADDRESS;f[1]=2;f[2]=0x3E;f[3]=0;input(f,4);before=PduR_GetTxErrorCount();for(unsigned i=0;i<201;++i)Fbl_MainFunction();CHECK(PduR_GetTxErrorCount()==before+1);
 puts("FBL transport: PASS routed SF/FF/CF/FC, sequence rejection, physical completion and timeout");return 0;
}
