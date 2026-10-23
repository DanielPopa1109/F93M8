# Build and memory

## Toolchain

Projects are STM32CubeIDE/Eclipse managed GNU Arm builds (`.project`, `.cproject`, `.mxproject`, `.ioc`) with external STM32CubeF4 HAL/CMSIS and FreeRTOS paths recorded as absolute Windows paths. Debug/Release make fragments and objects exist under `Debug/`; a build output is not evidence of a current clean build. Confirm tool versions, external package versions, paths and linker script per project before reproducing.

## Linker regions (configured)

| Project | FLASH | RAM / reserved |
|---|---|---|
| CBM FBL | `0x08000000`, 32 KiB | RAM `0x20010000`, 4 KiB |
| CBM APP | `0x08060000`, 128 KiB | RAM `0x20010000`, 63 KiB; NCR `0x2001FDA8`, 600 B |
| DMU FBL | `0x08000000`, 16 KiB | SRAM `0x20000000`, 128 KiB; CCM `0x10000000`, 63 KiB; 1 KiB reserved |
| DMU APP | `0x08020000`, 256 KiB | SRAM `0x20000000`, 128 KiB; CCM `0x10000000`, 63 KiB; 1 KiB reserved |

Sources: each project's `STM32..._FLASH.ld` (`MEMORY`, `.isr_vector`, `ENTRY(Reset_Handler)`) and matching RAM linker script. CBM FBL main also defines `APPL_START_ADDRESS 0x0800C000`, while the linked CBM APP and `FBL_JumpToAppl` use `0x08060000`; treat this as a stale/conflicting constant requiring correction before any updater reuse.

## Flash geometry and erase

FBL code maps STM32F4 sectors 0–7 in `FBL_NvM_GetSector`; actual geometry must be confirmed for the exact die. For these 512 KiB F4 maps, sectors 0–3 are 16 KiB, sector 4 is 64 KiB, sectors 5–7 are 128 KiB. CBM FBL erases sector 7 for app, sector 3 for coding, sector 2 for calibration. DMU FBL erases sectors 5–7 for app. FBL linker sizes (32/16 KiB) are smaller than their nominal F4 sectors, so reserved space sharing within the first sector must be considered. No runtime range validation makes a `0x34` address safe; preserve FBL partitions and do not erase based solely on the client request.

The APP linker images are not yet integrated with a generated FBL/application shared memory contract. Compare APP origin, FBL jump address, erase routine and image-generation output before changing any map. SRAM reserved symbols/sections must also be reconciled with hard-coded NVM-like pointers.
