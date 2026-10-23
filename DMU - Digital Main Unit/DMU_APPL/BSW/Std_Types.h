#ifndef DMU_STD_TYPES_H
#define DMU_STD_TYPES_H

#include "Platform_Types.h"
typedef uint8 Std_ReturnType;

typedef struct {
    uint16 vendorID;
    uint16 moduleID;
    uint8 sw_major_version;
    uint8 sw_minor_version;
    uint8 sw_patch_version;
} Std_VersionInfoType;

#define TRUE ((boolean)1U)
#define FALSE ((boolean)0U)
#define E_OK ((Std_ReturnType)0U)
#define E_NOT_OK ((Std_ReturnType)1U)
#define NULL_PTR ((void *)0)

#endif
