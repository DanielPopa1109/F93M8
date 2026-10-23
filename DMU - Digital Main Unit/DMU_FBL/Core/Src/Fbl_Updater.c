#include "Fbl_Updater.h"
#include "Fbl.h"
#include "../../BSW/Crc/Crc.h"
#include <string.h>
/* Legacy staging API retained for engineering clients. The independently
 * executing ram_blob receives/revalidates its own replacement, never this buffer. */
static uint8_t staged[FBL_RESIDENT_SIZE];
static uint32_t size,written;
static uint8_t verified;
void FblUpdater_Init(void) { size=written=0;verified=0; }
int FblUpdater_Begin(uint32_t n) { if(n<8 || n>sizeof(staged)) return 1;size=n;written=0;verified=0;return 0; }
int FblUpdater_Write(uint32_t off,const uint8_t *p,uint16_t n) {
 if(!p || !n || off!=written || off>size || n>size-off) return 1;
 memcpy(staged+off,p,n);written+=n;return 0;
}
int FblUpdater_Validate(uint32_t crc) {
 uint32_t sp,pc; if(!size || written!=size) return 1;
 memcpy(&sp,staged,4);memcpy(&pc,staged+4,4);
 if(sp<0x20000000U || sp>0x20020000U || (sp&7U) || !(pc&1U) ||
  (pc&~1U)<0x08000000U || (pc&~1U)>=0x08000000U+size ||
  Crc_CalculateCRC32(staged,size,0,1)!=crc) return 1;
 verified=1;return 0;
}
int FblUpdater_ValidateLayout(uint32_t load,uint32_t loadEnd,uint32_t start,uint32_t end,uint32_t sp,uint32_t pc) {
 if(load<RAM_FLASH_BASE || loadEnd<=load || loadEnd>RAM_FLASH_BASE+FBL_RESIDENT_SIZE ||
    start!=RAM_BASE || (start&511U) || end<=start || end>RAM_STACK_BOTTOM ||
    loadEnd-load!=end-start || end-start<448U || sp!=RAM_STACK_TOP || (sp&7U) ||
    !(pc&1U) || (pc&~1U)<start+448U || (pc&~1U)>=end)return 1;
 return 0;
}
int FblUpdater_Prepare(void) {
#if defined(__arm__) || defined(__thumb__)
 extern uint8_t __ram_blob_load_start[],__ram_blob_load_end[],__ram_blob_vma_start[],__ram_blob_vma_end[];
 uint32_t exception,options=*(volatile uint32_t *)0x40023C14U;
 __asm volatile("mrs %0,ipsr":"=r"(exception));
 /* Require thread context, software watchdog policy, unprotected resident
  * sectors and an idle controller. No option-byte changes are performed. */
 if(exception || !(options&(1U<<5)) || (options&0x30000U)!=0x30000U ||
    (*(volatile uint32_t *)0x40002C00U&(1U<<7)) ||
    (*(volatile uint32_t *)0x40023C0CU&(1U<<16)))return 1;
 uint8_t *src=__ram_blob_load_start,*dst=__ram_blob_vma_start;
 uint32_t n=(uint32_t)(__ram_blob_load_end-src),sp,pc;
 /* Validate location/size before dereferencing or copying the image. */
 if((uintptr_t)src<RAM_FLASH_BASE || (uintptr_t)__ram_blob_load_end>RAM_FLASH_BASE+FBL_RESIDENT_SIZE ||
    (uintptr_t)dst!=RAM_BASE || (uintptr_t)__ram_blob_vma_end>RAM_STACK_BOTTOM || n<448 ||
    n!=(uint32_t)(__ram_blob_vma_end-dst))return 1;
 memcpy(&sp,src,4);memcpy(&pc,src+4,4);
 if(FblUpdater_ValidateLayout((uint32_t)src,(uint32_t)__ram_blob_load_end,(uint32_t)dst,(uint32_t)__ram_blob_vma_end,sp,pc))return 1;
 for(uint32_t i=2;i<112;i++){
  uint32_t handler;memcpy(&handler,src+4*i,4);
  if(!(handler&1U) || (handler&~1U)<RAM_BASE+448U || (handler&~1U)>=(uint32_t)__ram_blob_vma_end)return 1;
 }
 for(uint32_t i=0;i<n;i++)dst[i]=src[i];
 for(uint32_t i=0;i<n;i++)if(dst[i]!=src[i])return 1;
 __asm volatile("dsb\n isb":::"memory");return 0;
#else
 /* Native tests execute layout/protocol logic; host cannot copy Cortex-M VMAs. */
 return 1;
#endif
}
int FblUpdater_Activate(void) {
#if FBL_UPDATER_ACTIVATION_ALLOWED && (defined(__arm__) || defined(__thumb__))
 if(FblUpdater_Prepare())return 1;
 volatile uint32_t *systick=(volatile uint32_t *)0xE000E010U;
 __asm volatile("cpsid i":::"memory");systick[0]=systick[1]=systick[2]=0;
 for(uint32_t i=0;i<8;i++){
  ((volatile uint32_t *)0xE000E180U)[i]=0xFFFFFFFFU;
  ((volatile uint32_t *)0xE000E280U)[i]=0xFFFFFFFFU;
 }
 *(volatile uint32_t *)0xE000ED04U=(1U<<25)|(1U<<27);
 *(volatile uint32_t *)0xE000ED08U=RAM_BASE;
 uint32_t sp=*(volatile uint32_t *)RAM_BASE,pc=*(volatile uint32_t *)(RAM_BASE+4);
 __asm volatile("dsb\n isb\n msr msp, %0\n movs r0,#0\n msr control,r0\n msr basepri,r0\n msr faultmask,r0\n isb\n cpsie i\n bx %1"::"r"(sp),"r"(pc):"r0","memory");
 __builtin_unreachable();
#else
 return 1; /* Explicit deployment interlock, functional ARM path compiled separately. */
#endif
}
