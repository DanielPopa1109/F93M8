#ifndef DCM_CFG_H
#define DCM_CFG_H
#include "Dcm.h"
#define DCM_P2_TICKS 10U
#define DCM_P2STAR_TICKS 1000U
#define DCM_S3_TICKS 1000U
#define DCM_SERVICE_COUNT 11U
typedef struct { uint8 sid; uint16 minimum,maximum; uint8 sessions; } Dcm_ServiceConfig;
extern const Dcm_ServiceConfig Dcm_Services[DCM_SERVICE_COUNT];
#endif
