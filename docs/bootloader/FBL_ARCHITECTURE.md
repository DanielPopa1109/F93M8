> Current 2026-10-19 implementation: [closure](../modernization/F93_REMAINING_GAPS_CLOSURE.md), [protocol](DIAGNOSTIC_PROTOCOL.md), [memory](MEMORY_AND_LINKER.md), [programming](FLASH_PROGRAMMING.md), [updater interlock](FBL_UPDATER.md), [executed validation](../modernization/F93_VERIFICATION_RESULTS.md). Both FBLs now have routed ISO-TP/programming and linked ARM ELFs. Older legacy-transport descriptions below are historical; hardware and updater activation remain unverified/blocked.

# FBL architecture

## Two programming executors

1. **Flash-resident receiver/application writer** — `ELC_FBL/Core/Src/main.c`. Cube HAL initializes clocks, GPIO, CAN, CRC and interrupts. RX interrupt receives CAN ID `0x702`; custom ISO-TP reassembles UDS. The parser erases/writes APP and calculates image CRC. Those flash operations run through code linked in FBL flash; during STM32F1 flash busy time CPU fetch stalls, so continuous diagnostic service during the operation is not demonstrated.
2. **RAM-resident FBL self-writer** — `FBL_RunRamUpdater` copies bytes `__ram_blob_load_start..end` to `__ram_blob_vma_start`, reads vector MSP/reset PC, sets VTOR/MSP and branches. `ram_blob.c` uses raw STM32F1 registers, polls CAN and erases/programs only `0x08000000..0x080037FF`. Its code/data/vector sections are linked in SRAM and loaded from FBL flash.

The RAM blob exists because self-erasing FBL flash cannot safely fetch its active code from the region being erased/programmed. It masks NVIC IRQs, stops SysTick and polls CAN; no RTOS/HAL services run in the blob. It also has its own UDS/ISO-TP implementation and state, so protocol parity must not be assumed.

References: linker output section `.ram_blob`; symbols in `STM32F103C8TX_FLASH.ld`; entry routine IDs and parser in `main.c` / `ram_blob.c`.
