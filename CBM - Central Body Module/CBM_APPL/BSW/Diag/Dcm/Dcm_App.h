#ifndef DCM_APP_H
#define DCM_APP_H
#include "Dcm.h"
uint16 Dcm_AppReadDid(uint16 did,uint8 *out);
uint16 Dcm_AppRoutine(const uint8 *req,uint16 len,uint8 *out);
uint16 Dcm_AppPeriodic(const uint8 *req,uint16 len,uint8 *out);
void Dcm_AppCommunication(uint8 control);
void Dcm_AppReset(uint8 session);
void Dcm_AppClearSources(void);
#endif
