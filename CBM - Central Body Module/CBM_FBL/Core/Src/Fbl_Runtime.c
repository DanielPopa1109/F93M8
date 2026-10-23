/* Freestanding FBL constructor initialization, independent of a hosted C runtime. */
typedef void (*Fbl_Constructor)(void);
extern Fbl_Constructor __preinit_array_start[],__preinit_array_end[];
extern Fbl_Constructor __init_array_start[],__init_array_end[];
void Fbl_InitRuntime(void) {
 Fbl_Constructor *p;
 for(p=__preinit_array_start;p<__preinit_array_end;++p)(*p)();
 for(p=__init_array_start;p<__init_array_end;++p)(*p)();
}
