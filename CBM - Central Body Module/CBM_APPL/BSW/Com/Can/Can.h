#ifndef CBM_CAN_H
#define CBM_CAN_H
#include <stdint.h>

#define CAN_OK 0U
#define CAN_NOT_OK 1U
typedef uint8_t Can_StatusType;
typedef struct { uint16_t id; uint8_t dlc; uint8_t data[8]; } Can_FrameType;

void Can_Init(void);
Can_StatusType Can_WriteApplication(uint16_t id,const uint8_t *p,uint8_t n);
void Can_TxConfirmation(uint8_t success);
void Can_BusOff(void);
void Can_MainFunction(void);
Can_StatusType Can_Write(uint16_t id, const uint8_t *data, uint8_t dlc);
void Can_RxIndication(const Can_FrameType *frame);

#endif
