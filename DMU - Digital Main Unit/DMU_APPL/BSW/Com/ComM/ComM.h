#ifndef DMU_COMM_H
#define DMU_COMM_H
#include <stdint.h>
typedef enum { COMM_NO_COMMUNICATION = 0, COMM_SILENT_COMMUNICATION, COMM_FULL_COMMUNICATION } ComM_ModeType;
void ComM_Init(void);
void ComM_MainFunction(void);
uint8_t ComM_RequestComMode(ComM_ModeType mode);
ComM_ModeType ComM_GetCurrentComMode(void);
#endif
