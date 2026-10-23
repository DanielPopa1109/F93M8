#ifndef CBM_CANNM_H
#define CBM_CANNM_H

#include <stdint.h>

typedef enum { CANNM_BUS_SLEEP = 0, CANNM_REPEAT_MESSAGE, CANNM_NORMAL_OPERATION, CANNM_READY_SLEEP } CanNm_StateType;

void CanNm_Init(void);
void CanNm_RxIndication(uint16_t canId, const uint8_t *data, uint8_t dlc);
void CanNm_MainFunction(void);
void CanNm_NetworkRequest(void);
void CanNm_NetworkRelease(void);
CanNm_StateType CanNm_GetState(void);

#endif
