# STM32 portability boundaries

## Family-specific source from F93

The loaders depend on STM32F4 HAL conventions (`HAL_FLASH_*`, sector erase IDs, `SCB->VTOR`, `__set_MSP`, RCC reset flags), Cortex-M4 vector/startup semantics, and either external MCP2515 (CBM) or STM32F4 bxCAN (DMU). F4 sector erase geometry and word programming are not generic STM32 behaviors. Even F4 subfamilies vary in density, banks, sector sizes, voltage range, cache/ART behavior, dual-bank support, and flash error handling.

## Must be re-derived for each target

- Exact MCU order code, revision and reference manual flash map, erase units, programming width, alignment, protection/option bytes and errata.
- Core architecture, VTOR alignment, security/TrustZone state (where applicable), vector count, stack/SRAM/CCM domains, cache/FPU state and reset semantics.
- Clock source tree, boot-time oscillator safety, watchdog ownership and behavior through erase/program/reset.
- CAN controller vs external transceiver, pins, bit timing, standard/extended IDs, filters, interrupts, DMA and bus recovery.
- Boot ROM/DFU, external storage, dual-bank, secure boot or physical recovery options.
- Application linker vectors and startup/SystemInit contract, RTOS stop/confirmation hooks and retained metadata layout.

## Reusable behavior, not reusable source

Potentially reusable design concepts: explicit image bounds, UDS state machine, target profile, staged metadata, verify-before-commit, readback, centralized NRC mapping, boot confirmation and fault injection. Do not copy F93 hard-coded addresses, HAL calls, CAN driver, flash sector tables, retained SRAM sentinels, security (none evidenced), or handover routine into another MCU unchanged. The TLE9893 vendor package under TSMaster is a separate Infineon family and is not a portability bridge.
