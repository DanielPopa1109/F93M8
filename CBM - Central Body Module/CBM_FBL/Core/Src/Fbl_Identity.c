#include "Fbl_Ram.h"
/* Fixed-offset raw image descriptor. It is checked from the SRAM candidate,
 * never referenced by SRAM code through this Flash symbol. Not a signature. */
__attribute__((section(".fbl_identity"),used))
const uint32_t Fbl_ImageIdentity[8]={RAM_IMAGE_MAGIC,1,RAM_TARGET,RAM_MCU_ID,
 FBL_RESIDENT_SIZE,FBL_APP_START,FBL_METADATA,RAM_RESIDENT_VECTOR_WORDS};
