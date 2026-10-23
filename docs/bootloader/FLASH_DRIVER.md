# Flash driver

## Resident application writer

`FBL_NvM_EraseFlash_APPL` disables IRQs, unlocks flash, asks `Nvm_GetPage` for page base of `0x0800F7FF`, erases 1-KiB pages from `0x08003C00` through that page, locks flash and re-enables IRQs. `FBL_NvM_EraseFlash_PARAM` similarly erases the one parameter page at `0x08003800). `FBL_NvM_FlashWriteData` uses HAL word programming and increments destination by four bytes. It disables interrupts around writes. Error paths return HAL error values, but key callers discard them.

## RAM self-programmer

`ram_blob.c` raw-register driver unlocks using STM32F1 keys, waits on FLASH_SR.BSY, clears EOP/errors, page-erases in 1024-byte steps, and programs two halfwords per 32-bit word. It rejects addresses outside `0x08000000..0x080037FF` and refuses non-erased halfwords. Erase helper loops through pages and locks flash at end. Flash function results are ignored in UDS processing; transmit response can claim success despite controller failure.

## Porting risk

Both paths encode STM32F1-specific page size/register behavior. Do not carry register addresses, key sequences, voltage assumptions, page geometry or programming granularity to a different STM32 family. Implement a family-specific driver from that MCU's reference manual and errata; guard overflow, alignment, range, timeout, write status and power conditions.
