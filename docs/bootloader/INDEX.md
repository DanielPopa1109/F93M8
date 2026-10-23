> Current 2026-10-19 ELC-port implementation: [porting matrix](ELC_FBL_PORTING_MATRIX.md), [RAM execution and verification](RAM_UPDATER_VERIFICATION.md), [memory](MEMORY_AND_LINKER.md) and [executed results](../modernization/F93_VERIFICATION_RESULTS.md). Both FBLs have real independently executing SRAM updaters and verified default/controlled ARM ELFs. Default deployment is interlocked; full CubeIDE/APP builds and all hardware/recovery evidence remain unverified. Older staging-only/legacy statements below are historical.

# F93 flashloader and bootloader reference

**Historical baseline below (superseded by the current implementation links):** source-only analysis. Neither loader has been demonstrated on hardware here. The active STM32 FBLs are minimal lab flashloaders, not safe turnkey or ZGW-compatible bootloaders.

See the [current modernization implementation report](../modernization/IMPLEMENTATION_REPORT.md) and [second static audit](../modernization/STATIC_AUDIT_REVIEW.md) for receive-filter, flash-bound, erase-response and erase-scope fixes. Some older pages predate those changes; `MEMORY_AND_LINKER.md` and `FLASH_PROGRAMMING.md` reflect the current source.

## Read by task

- [Architecture and evidence](BOOTLOADER_ARCHITECTURE.md): identify which behavior is in CBM vs DMU.
- [Memory and linker](MEMORY_AND_LINKER.md): partition addresses, sector erase scope and linker conflicts.
- [Boot and handover](BOOT_AND_HANDOVER.md): reset decision, retained RAM flags and application jump.
- [Flash programming](FLASH_PROGRAMMING.md): erase, write, verify, bounds and recovery findings.
- [Diagnostic protocol](DIAGNOSTIC_PROTOCOL.md): CAN frame shape and implemented UDS subset.
- [State machine and recovery](STATE_MACHINE_AND_RECOVERY.md): update lifecycle and missing safeguards.
- [STM32 portability](STM32_PORTABILITY.md): MCU/board/application-specific decisions.
- [Implementation guide](BOOTLOADER_IMPLEMENTATION_GUIDE.md): actionable design for a new target.
- [Validation](BOOTLOADER_VALIDATION.md): build, host, bench and interruption acceptance plan.

## Source roots

CBM: `CBM - Central Body Module/CBM_FBL/`; DMU: `DMU - Digital Main Unit/DMU_FBL/`. Main implementation is each `Core/Src/main.c`; memory is each `STM32..._FLASH.ld`; peripheral setup is under `Core/Src/` and `.ioc`. Compare the associated APP linker files. For ZGW/FCD compatibility use [the compatibility index](../compatibility/INDEX.md).
