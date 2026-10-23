> Current 2026-10-19 ELC-port implementation: [porting matrix](ELC_FBL_PORTING_MATRIX.md), [RAM execution and verification](RAM_UPDATER_VERIFICATION.md), [memory](MEMORY_AND_LINKER.md) and [executed results](../modernization/F93_VERIFICATION_RESULTS.md). Both FBLs have real independently executing SRAM updaters and verified default/controlled ARM ELFs. Default deployment is interlocked; full CubeIDE/APP builds and all hardware/recovery evidence remain unverified. Older staging-only/legacy statements below are historical.

# Bootloader architecture and evidence

## What exists

Both ECU folders include separate CubeIDE FBL and APP projects. FBL code is mostly one `Core/Src/main.c` plus generated HAL startup/peripheral files. There is no standalone transport, UDS, image metadata or flash abstraction module in either FBL. The implementation appears hand-written/generated hybrid and should be treated as legacy experimental source until built and bench-verified.

| Behavior | CBM FBL | DMU FBL |
|---|---|---|
| CAN PHY/controller | External MCP2515 over SPI; inline register driver in `main.c` | STM32F407 CAN1 via HAL (`Core/Src/can.c`) |
| RX handling | Polls MCP2515 in diagnostic/session functions | RX FIFO0 HAL callbacks |
| App address | linked app/jump `0x08060000`; stale `APPL_START_ADDRESS 0x0800C000` also defined | `0x08020000` |
| Erase | sector 7 app; sector 3 coding; sector 2 calibration | sectors 5–7 app |
| Extra update states | coding/calibration | none |
| Transport | raw 8-byte CAN handling in custom MCP layer | raw classic CAN frame handling |
| Verified CRC/auth | not present in active handler | not present in active handler |

## Observed startup pipeline

Both call `HAL_Init`, `SystemClock_Config`, GPIO and CAN-related init, then choose application or remain in FBL based on RAM sentinel/session values and whether the first app word is erased (`0xFFFFFFFF`). Reset-source code clears portions of reserved RAM on POR/BOR. The app vector's initial stack pointer and reset handler are not robustly range-checked. See `main()` and `FBL_JumpToAppl()` in each FBL `main.c`.

## Proven vs missing

Source demonstrates address constants, generated peripheral initialization, UDS-like SID branches, HAL flash erase/program calls and direct vector-table jump code. It does not establish correct UDS conformance, ISO-TP segmentation, bounded flash writes, image authenticity/integrity, interrupted-update recovery, target response under load, app boot success, hardware timing or a reproducible build. These gaps are requirements for a new implementation, not behavior to copy.
