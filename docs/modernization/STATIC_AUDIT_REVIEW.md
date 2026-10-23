# Current implementation continuation — 2026-10-19

**PARTIAL, with executed native host tests and complete Cortex-M FBL links.** The [remaining-gaps closure report](F93_REMAINING_GAPS_CLOSURE.md), [verification snapshot](F93_VERIFICATION_RESULTS.md), and [source manifest](F93_SOURCE_MANIFEST.md) supersede conflicting current-state claims in the historical material below. F01–F12 are reconciled individually; original F13/F14 text is unavailable and is not treated as closed. No hardware/deployment/compliance claim is made.

Implemented: queued NvM/MemIf/Crc/STM32 flash journals and migration; complete configured DEM record persistence with durable asynchronous ClearDTC; full-payload DCM and physical-response completion; configured COM signals/PDUs/groups/timeouts; actual CAN TX callback paths; both routed FBL stacks and FCD programming cores; guarded RAM-only FBL candidate staging. Full APP builds, full SWS behavior, bus/electrical testing, sparse-image packaging, and safe resident-FBL activation remain open.

---

# F93 second static audit â€” 2026-10-18

This review inspects the current F93 working tree against the current ZGW application and FCD source. It is source evidence, not a Cortex-M build, bench trace, flash test, or vehicle validation. The earlier dirty working tree was preserved. No hardware was changed.

## Finding disposition

| ID | Classification | Current source evidence and remaining gap |
| --- | --- | --- |
| F01 DMU DEM bounds | **Complete and statically verified** for the reported out-of-bounds defect | `DMU_APPL/BSW/Diag/Dem/Dem.c` maps the nine configured IDs `0x0E..0x16` to nine bounded pairs. `Dem_ValidateStoredDtc` checks each 18-word persisted pair after `Nvm_ReadAll`; `Dem_MainFunction` aggregates fault arrays before setting one event result. `tests/test_dem.c` compiles for AArch64 but could not execute here. Full UDS status semantics remain under F06. |
| F02 CBM FlowControl | **Complete and statically verified** for the reported offsets and bounds | `CBM_APPL/BSW/Com/CanTp/CanTp.c` reads `[EA, PCI, BS, STmin]` at indices `0..3`, requires DLC 4, handles CTS/WAIT/OVERFLOW, limits WAIT count, validates STmin, and retains 16-bit remaining lengths. `tests/test_com_stack.c` includes BS, pacing, WAIT, OVERFLOW, 256-byte reassembly, sequence and timeout cases. Executable tests remain unrun. |
| F03 APP/FBL transport | **Defective** | Both applications accept `0x710` with ECU EA `0x54`/`0x56`, response EA `0xF0`, and gateway FC EA `0x41` (`CanTp.h`, `CanIf.h`). CBM FBL `Core/Src/main.c` still filters/dispatches `0x700/0x701`; DMU FBL still uses `0x702/0x703`. Neither FBL implements extended-address multi-frame RequestDownload/TransferData. ZGW `ParallelFlashSwc.c` maps CBM and DMU to `0x54` and `0x56`; FCD uses those ECU addresses and parses a RequestDownload max-block response. |
| F04 erase acknowledgments | **Partial** | Both FBL `Core/Src/main.c` erase routines now check unlock, erase result, lock, and erased words before positive `0x71`; failure returns NRC `0x72`. Operations still run synchronously with interrupts disabled, without a transmitted-and-confirmed NRC `0x78` ResponsePending sequence. P2 compliance is unverified. |
| F05 NM3 | **Partial** | The current interoperability contract uses standard `0x3FF`, PN1 at byte 0, active value `0xFF`, and a 1 s remote timeout; CBM and DMU now accept the current one-byte frame. Each application owns separate Com, CanNm, ComM and CanSM state modules, invokes them every 5 ms, and notifies CanNm from its MCU-specific receive path. Legacy application receive logic remains in parallel, and controller/transceiver transitions, bus-off recovery and bench sleep/wake behavior are unverified. |
| F06 DCM/DEM | **Partial** | DMU DCM has bounded TP request copy and a 5 ms task, but service dispatch remains a switch and `19 0A` returns legacy raw event/status bytes. CBM DCM routes through PduR yet still adapts a legacy single-frame switch with internal `0x700/0x701` frames. CBM DEM remains legacy. Neither has ZGW's configured DCM/DEM, complete UDS DTC status, asynchronous services, or P2/P2* handling. DMU programming/reset/clear paths now reject failed NvM writes. |
| F07 memory architecture | **Partial** | Both APP NvM implementations still store legacy raw words in fixed sectors. There are no project-local MemIf or Crc modules, validated block headers, atomic commit or power-loss recovery. DMU keeps the 18-word DEM layout and avoids erasing unchanged blocks; CBM keeps its existing DTC/calibration/coding layout. |
| F08 flash-controller paths | **Partial** | Both APP NvM write functions validate sector/start/length, check unlock and erase/program result, compare programmed words and call `HAL_FLASH_Lock` on every post-unlock path. DMU reports write failure and retries with a bound; CBM retains its dirty DTC cache on failure. Erase-first sector rewriting is still vulnerable to interrupted writes; no CRC or redundant slot exists. |
| F09 FBL self-updater | **Blocked** | Neither FBL has a safe self-update implementation. Their resident boot code starts in sector 0 (`STM32F411RETX_FLASH.ld`, `STM32F407VETX_FLASH.ld`); current write bounds accept only APP/coding/calibration regions. ZGW's TC37x RAM updater and ELC's STM32F103 RAM blob cannot be used as an STM32F411/F407 recovery guarantee. An interrupted sector-0 erase could remove all executable boot code. No unsafe self-erase entry was introduced. |
| F10 linker overlap | **Complete and statically verified** for the reported overlap | DMU APP `STM32F407VETX_FLASH.ld` now places 63 KiB CCMRAM at `0x10000000..0x1000FBFF` and 1 KiB NCR at `0x1000FC00..0x1000FFFF`, both aligned. Startup copies only `.ccmram`, leaving `.ncr` reset-retained. Parsed MEMORY regions of all four FLASH linker scripts have no intra-project overlap. No ELF/map was produced. |
| F11 transmission semantics | **Defective** | `Can_Write` returns success when `HAL_CAN_AddTxMessage` accepts a bxCAN mailbox or `CanSpi_Transmit` issues MCP2515 RTS. `CanTp` then reports TP success after the last frame is queued; `PduR_CanTpTxConfirmation` has no physical TX callback. Hardware TX completion, errors and UDS operation completion are still conflated. |
| F12 project integration | **Partial** | All four `.cproject`/`.project` XML files parse. CBM APP Debug and Release source entries include independent Can/CanIf/CanTp/PduR. Its checked-in Debug makefile, sources list and module rules include the four modules. DMU APP `BSW/Com` includes them in Debug and Release; its generated Debug rules list them. Both checked-in Debug `objects.list` files include the four module objects, and local include/linker paths use their own checkouts. No cross-project source references were found. STM32CubeIDE, `arm-none-eabi-gcc`, GNU make, and the configured STM32Cube FW F4 v1.28.1 repository were not found, so no target build or map check was possible. |

## Interface and implementation gates

| Gate | CBM APP | DMU APP | FBLs |
| --- | --- | --- | --- |
| Diagnostic address and extended ISO-TP | `CanTp.h` `0x54`, `0x41`, `0xF0`; SF/FF/CF/FC source path | `CanTp.h` `0x56`, `0x41`, `0xF0`; SF/FF/CF/FC source path | Legacy single-frame CAN IDs; incompatible with ZGW forwarding |
| PDU routing and callbacks | `CanIf.c` has diagnostic PDU only; `PduR.c` routes DCM only | Same | No CanIf/CanTp/PduR |
| Application COM path | Body frames remain in `CanSpi.c` | Body/cluster frames remain in `CanH.c` | Not applicable |
| Initialization and 5 ms schedule | `freertos.c` initializes Can/CanIf/CanTp/PduR and runs CanSpi/CanTp/DCM in `QM_DIAG` | `freertos.c` initializes the stack; Can RX, CanTp, DCM in 5 ms task | Polling main loops, no matching TP schedule |
| NM3 and sleep | `CanSpi.c` and `EcuM.c` consume new NM3 payload; legacy state machine | `CanH.c` consumes new NM3, has 5 s diagnostic hold, and resets from sleep on addressed request | NM3 not integrated |
| DEM/NvM | Legacy DEM; sector writes checked but no block CRC | Bounded DEM pairs, validated restoration, checked sector writes | Program/erase only, no transactional image validation |
| APP/FBL flash partition | APP `0x08060000..0x0807FFFF`; FBL sector 0/1, APP erase sector 7 | APP `0x08020000..0x0805FFFF`; FBL sector 0, APP erase sectors 5â€“6 | No image-wide integrity proof before handover |

## Tests and environment

- The independent CBM and DMU `test_network_control.c` harnesses compiled and linked with AArch64 GCC and `-std=c11 -Wall -Wextra -Werror`. They statically exercise one-byte NM3 activation, full-communication promotion, timeout and bus-sleep return, but the AArch64 executables were not run.
- AArch64 GCC compiled and linked `CBM_APPL/tests/test_com_stack.c`, `DMU_APPL/tests/test_com_stack.c`, and `DMU_APPL/tests/test_dem.c` with `-std=c11 -Wall -Wextra -Werror`. The executables target AArch64 Linux and were **not run**. No QEMU runner was installed; WSL returned access denied.
- `make -C DMU_APPL/Debug all` was attempted and failed before compilation because `make` is unavailable. An AArch64 Linux compiler is not an STM32 Cortex-M compiler. The installed ST directory did not contain STM32CubeIDE, and the configured HAL/CMSIS repository path is missing.
- Four `.cproject` and `.project` XML files parsed. A MEMORY-region script found no within-script overlaps. The application source-rule-object chain was checked for all four new communication modules in both projects, and no cross-project source references were found. `git diff --check` passed; these are static checks, not a build or functional test.
- No ECU was flashed, no vehicle or bench tests ran, and no CM5 was modified or rebooted.

## Remaining acceptance work

The next implementation needs independent CBM/DMU Com, CanSM, CanNm and ComM paths; DCM service/session tables and UDS DEM status; MemIf/Crc and recoverable NvM blocks; FBL extended-address ISO-TP plus FCD's download/block-sequence/CRC contract; physical CAN TX confirmation; and a proven interrupted-FBL-update recovery strategy. Each requires a Cortex-M build, host execution, and later controlled hardware validation before a completion claim.
