# Current implementation continuation � 2026-10-19

**PARTIAL, with executed native host tests and complete Cortex-M FBL links.** The [remaining-gaps closure report](F93_REMAINING_GAPS_CLOSURE.md), [verification snapshot](F93_VERIFICATION_RESULTS.md), and [source manifest](F93_SOURCE_MANIFEST.md) supersede conflicting current-state claims in the historical material below. F01�F12 are reconciled individually; original F13/F14 text is unavailable and is not treated as closed. No hardware/deployment/compliance claim is made.

Implemented: queued NvM/MemIf/Crc/STM32 flash journals and migration; complete configured DEM record persistence with durable asynchronous ClearDTC; full-payload DCM and physical-response completion; configured COM signals/PDUs/groups/timeouts; actual CAN TX callback paths; both routed FBL stacks and FCD programming cores; independent ELC-style SRAM FBL updaters with CAN/ISO-TP/UDS, F4 resident replacement, image identity and CRC/readback. Full APP builds, full SWS behavior, bus/electrical testing, sparse-image packaging, and independently usable board recovery/deployment enablement remain open.

---

# F93 modernization implementation report

## Current continuation — 2026-10-18

### Network-control continuation — 2026-10-19

CBM and DMU now each own independent `BSW/Com/{Com,CanNm,CanSM,ComM}` sources. Their MCU-specific receive paths accept the current one-byte standard `0x3FF` frame and notify CanNm; Com owns PN1 value/age and a 1 s timeout at the existing 5 ms schedule. CanNm implements bus-sleep, repeat-message, normal-operation and ready-sleep transitions, while ComM combines a local request with remote NM3 activity and CanSM exposes full/silent/no-communication state and TX permission. Initialization and cyclic calls are connected in each `freertos.c`. CBM Debug/Release source entries, both checked-in Debug source lists, makefile includes, object lists and module rules include all four modules; DMU's broad `BSW/Com` source entry already includes them and its checked-in Debug files were updated explicitly.

This slice does **not** yet move legacy application signal routing out of `CanSpi.c`/`CanH.c`, command physical controller or transceiver modes, implement bus-off recovery callbacks, or prove sleep/wake behavior. Those remain required before network-management completion. DET and MemMap remain explicitly out of scope.

Both per-ECU network-control harnesses compiled and linked with AArch64 GCC using `-std=c11 -Wall -Wextra -Werror`. They cover a one-byte active NM3 indication, full-communication promotion, remote timeout and return to bus sleep/no communication. The AArch64 binaries were not executed because no compatible runner is available. Both `.cproject` files parsed as XML and `git diff --check` passed. No STM32 target build, hardware test, deployment, flash, CM5 change, reboot, commit or push occurred.

**Overall status: Partial.** This execution corrected the confirmed DEM bounds, FlowControl, flash-locking, erase-response, NM3 and linker defects below, aligned CBM's Release source configuration, and added an independently owned network-control slice to each application. It did not complete ZGW's complete application BSW model or routed FBL programming. The [second static audit](STATIC_AUDIT_REVIEW.md) records source and configuration evidence for every finding. Earlier sections below are historical snapshots and are superseded by this section wherever they differ.

| Finding | Current classification | Result |
| --- | --- | --- |
| F01 DMU DEM array | **Complete and statically verified** | Nine explicit event slots, bounds validation, persisted-pair validation, aggregated faults; no event-ID-derived array index. |
| F02 CBM FC | **Complete and statically verified** | Correct EA/PCI/BS/STmin offsets, DLC/FS/STmin checks, bounded WAIT, 16-bit remaining length; expanded cross-compiled harness. |
| F03 FBL diagnostics | **Defective** | Both loaders still use legacy `0x700/0x701` or `0x702/0x703`, lack extended ISO-TP and FCD download contract. |
| F04 FBL erase responses | **Partial** | Erase, lock and full-range readback precede positive response; errors return NRC `0x72`. ResponsePending and P2 handling are absent. |
| F05 NM3 | **Partial** | Both applications accept the current one-byte ZGW `0x3FF` PN1 byte 0 `0xFF`, route it into per-ECU Com/CanNm, expire remote activity after 1 s, and coordinate CanNm/ComM/CanSM modes. Controller/transceiver mode control, bus-off recovery callbacks, application-PDU Com migration and bench timing remain open. |
| F06 DCM/DEM | **Partial** | DMU DEM bounds and restore fixed; DMU DCM blocks reset/programming if NvM write fails. Configured DCM/DEM, UDS status and async timing remain absent. |
| F07 memory stack | **Partial** | Raw-sector NvM remains; no MemIf, Crc, redundant block or migration header. |
| F08 flash handling | **Partial** | Both APP NvM write paths check bounds, unlock/erase/program/readback/lock. DMU has bounded retry and avoids unchanged-block erases. Interrupted erase-first writes remain unsafe. |
| F09 FBL updater | **Blocked** | Resident sector-0 replacement has no proven interrupted-update recovery path. No self-erase path was added. |
| F10 DMU linker | **Complete and statically verified** | NCR starts at aligned `0x1000FC00`; four FLASH linker MEMORY maps were parsed without within-script overlaps. |
| F11 CAN TX completion | **Defective** | `Can_Write` still reports mailbox acceptance, and PduR/CanTp lack physical TX confirmation. |
| F12 project/build | **Partial** | CBM Debug generated rules, Debug/Release project source lists and local includes now include its BSW. Both applications' checked-in Debug object lists include the new modules, and their linker rules use relative script paths. Target toolchain and HAL/CMSIS tree are missing, so target builds remain unverified. |

### Application module completion matrix

| Module/path | CBM APP | DMU APP |
| --- | --- | --- |
| Can, CanIf, CanTp, PduR diagnostic path | **Implemented but not verified** on STM32 hardware; AArch64 harness linked but did not run | **Implemented but not verified** on STM32 hardware; AArch64 harness linked but did not run |
| Com application signal routing | **Partial**: `CanSpi` still owns application frames | **Partial**: `CanH` still owns application frames |
| Com, CanSM, CanNm, ComM | **Partial**: independent configured NM3 receive/timeout and mode state machines are initialized and scheduled; legacy application-frame parsing remains | **Partial**: independent configured NM3 receive/timeout and mode state machines are initialized and scheduled; legacy application-frame parsing remains |
| DCM, DEM | **Partial**: legacy DCM adapter and DEM | **Partial**: bounded TP DCM and repaired DEM, without configured UDS status/async lifecycle |
| NvM, MemIf, Crc | **Partial**: checked sector NvM; MemIf/Crc missing | **Partial**: checked sector NvM; MemIf/Crc missing |

### FBL, updater and ZGW/FCD contract

CBM FBL and DMU FBL remain independent projects. Their erase handlers now verify completion before positive responses, and their flash-word write paths check unlock and programmed-word readback. Both FBL diagnostic transports remain **defective** against ZGW routing. ZGW `ParallelFlashSwc` and FCD identify CBM `0x54` and DMU `0x56` on classic CAN ID `0x710`; the applications implement that extended-address route, while the FBLs use legacy IDs and only small single frames. FCD expects RequestDownload maximum block length, BSC-controlled TransferData, TransferExit and CRC. Neither loader implements that complete sequence. The self-updater is **blocked** by absence of a recoverable sector-0 strategy on both MCUs. No claim of flash readiness is made.

### Exact files changed in this continuation

- CBM APP: `.cproject`, `Debug/makefile`, `Debug/sources.mk`, `Debug/objects.list`, `Debug/**/subdir.mk` (local paths and four new module rules), `BSW/Com/CanTp/CanTp.c`, `BSW/Com/CanSpi/CanSpi.c`, `BSW/Sys/EcuM/EcuM.c`, `BSW/Mem/Nvm.c`, `BSW/Mem/Nvm.h`, `tests/test_com_stack.c`.
- DMU APP: `BSW/Diag/Dem/{Dem.c,Dem.h}`, `BSW/Diag/Dcm/Dcm.c`, `BSW/Mem/Nvm/{Nvm.c,Nvm.h}`, `BSW/Com/CanTp/CanTp.c`, `BSW/Com/CanH/CanH.c`, `Core/Src/can.c`, `Debug/makefile`, `Debug/objects.list`, `STM32F407VETX_FLASH.ld`, `tests/test_com_stack.c`, `tests/test_dem.c`.
- FBLs: `CBM_FBL/Core/Src/main.c`, `DMU_FBL/Core/Src/main.c`.
- Reports and knowledge pages: this file, `STATIC_AUDIT_REVIEW.md`, and the affected knowledge/bootloader pages linked from their indexes.

The working tree already contained uncommitted prior CBM/DMU integration files and changes; the list above describes this continuation, not a clean-branch diff.

### Executed verification and limits

- AArch64 GCC **compiled and linked** CBM and DMU transport harnesses plus the new DMU DEM harness with `-std=c11 -Wall -Wextra -Werror`. The AArch64 binaries were **not executed**; no native C compiler or AArch64 runner was available.
- AArch64 GCC also **compiled and linked** `tests/test_network_control.c` independently for CBM and DMU with `-std=c11 -Wall -Wextra -Werror`; those binaries were likewise not executed.
- `make -C "DMU - Digital Main Unit/DMU_APPL/Debug" all` was attempted and failed before compilation because GNU make is absent. `arm-none-eabi-gcc`, STM32CubeIDE and the configured STM32Cube FW F4 v1.28.1 HAL/CMSIS tree were not found. No STM32 build, link or map validation is claimed.
- All four `.cproject`/`.project` files parsed; a script checked the four FLASH linker MEMORY maps for overlap; a source-rule-object check passed for each new module in both applications; no cross-project source references were found; `git diff --check` passed. These do not substitute for compilation or executed tests.
- Hardware-dependent limits: CAN filter/bit timing, MCP2515 and bxCAN TX confirmation, FCD response correlation, ISO-TP bus timing, erase duration, retained-RAM reset behavior and power-loss recovery remain untested.
- No software was deployed or flashed. CBM, DMU, ZGW, ELC and CM5 hardware were not modified or rebooted. No commit or push was made.

---

## Historical revisions

## DMU communication architecture revision (2026-10-18)

**Status: source-integrated, cross-compiled transport/diagnostic harness; full STM32 firmware build and runtime behavior unverified.** This revision changes only `DMU_APPL` plus this report. CBM, ZGW, ELC and DMU_FBL were not modified.

DMU now owns its own `BSW/Com/{Can,CanIf,CanTp,PduR}` sources and headers. ZGW `ParallelFlashSwc` maps DMU to route `0x56`; its classic CAN receive PDU uses standard ID `0x710` and its extended TP configuration uses gateway destination `0xF0` and tester address `0x41`. DMU's bxCAN filter bank 12 now admits only standard data frames on `0x710`. Both existing FIFO0 callbacks enqueue these frames in a 16-entry Can receive ring (15 usable frames); the 5 ms diagnostic task drains up to 16 queued frames per tick through Can -> CanIf -> CanTp -> PduR -> Dcm while `CanH` retains the cluster, camera, recorder, NM and time/date CAN application logic. Responses travel Dcm -> PduR -> CanTp -> CanIf -> Can -> bxCAN1 on `0x710`, padded to eight bytes. The prior direct `0x702/0x703` APP diagnostic path is removed; the standalone DMU_FBL still uses that legacy pair.

CanTp has independent 256-byte RX/TX buffers, extended-address SF/FF/CF/FC handling, sequence checking, block-size and STmin handling, overflow FC, bounded WAIT FC handling, and one-second transport timers driven every 5 ms. PduR forwards complete UDS PDUs and records RX/TX errors. The DCM owns a one-request mailbox, dispatches complete UDS requests in the diagnostic task, and returns responses through PduR. Supported services are TesterPresent (`3E`, including suppress-positive-response), session control (`10`), ReadDataByIdentifier (`22 F186/F180/0005`, including repeated DIDs), ReadDTCInformation (`19 0A`, one segmented response), ClearDiagnosticInformation (`14`), CommunicationControl (`28`), ControlDTCSetting (`85`), DMU periodic status (`2A`), DMU cluster/camera/odometer routines (`31`) and ECU reset (`11`). Invalid lengths, unsupported services, unsupported DIDs and unsupported sessions receive bounded NRC responses. `Dcm_Init`, `Can_Init`, `CanIf_Init`, `CanTp_Init` and `PduR_Init` run in `MX_FREERTOS_Init`; `Can_MainFunction`, `CanTp_MainFunction` and `Dcm_MainFunction` run in order in the existing 5 ms DCM task. This keeps FlowControl transmission out of the CAN interrupt. The DCM request/response work buffers are static so the existing 512-byte DCM task stack is not consumed by two 256-byte arrays.

**Validation evidence:** Current `.cproject` identifies STM32F407VETx and GNU Tools for STM32 13.3.rel1. The checked-in Debug makefiles were extended for the four modules and their project-local include paths were corrected. `make -C "DMU - Digital Main Unit/DMU_APPL/Debug" all` was attempted, but `make` is not installed; `arm-none-eabi-gcc` and the referenced STM32Cube F4 V1.28.1 HAL tree are also absent. Therefore no STM32 compile/link/map check is claimed. The available `aarch64-none-linux-gnu-gcc` compiled and linked all four modules, DMU DCM and `tests/test_com_stack.c` with `-std=c11 -Wall -Wextra -Werror -DDMU_HOST_TEST` (exit 0). The harness covers addressing, SF request/response, multi-frame request/response with FC, block-size and STmin pacing, 256-byte reassembly, invalid sequence, RX/TX timeout, wrong ID/address, oversized FF and mailbox overrun. The resulting AArch64 executable was not run: no AArch64 emulator or native C compiler was found, and WSL returned access denied. `git diff --check` passed for the DMU scope. Source inspection confirms no DMU module includes CBM sources or headers; this does not substitute for a full firmware build or bus test.

**Remaining deficiencies:** bxCAN mailbox acceptance is not physical TX confirmation. The 50 ms delayed reset after `10 02` or `11` is not correlated to bus delivery. The CAN receive ring holds 15 frames and DCM has one pending request slot, with no P2/P2* asynchronous service framework; additional requests while occupied are counted and dropped. DMU APP does not implement UDS programming services `34/36/37`, and DMU_FBL is still on its legacy transport, so FCD flashing is not enabled by this change. The ZGW route still needs bench verification with real DMU frames and correlated target responses; ZGW's synthetic TX acknowledgement must not be mistaken for ECU execution. No firmware was flashed, software deployed, CM5 modified or rebooted, or code committed or pushed.

---
## CBM communication architecture revision (2026-10-17)

CBM_APPL now contains independent `Can`, `CanIf`, `CanTp` and `PduR` modules under `BSW/Com`. The physical driver remains CBM's MCP2515 `CanSpi`; its application traffic and communication-state handling were retained. Its diagnostic RX filters now match standard CAN ID `0x710` and both receive buffers pass these frames to `Can_RxIndication`. `CanIf` maps only ID `0x710` to the diagnostic PDU. `CanTp` implements extended-address ISO-TP with target address `0x54`, gateway FC address `0x41`, response address `0xF0`, 256-byte buffers, SF/FF/CF/FC, sequence checking, block-size and STmin processing, 1-second receive/transmit timers and overflow FC. `PduR` routes reassembled requests to CBM DCM and DCM responses to CanTp, with transport error counts. Module initialization is in `MX_FREERTOS_Init`; the 5-ms diagnostic task runs MCP2515 polling and `CanTp_MainFunction`. The DCM's old direct transmit calls now route through PduR; its DTC-list service builds one 28-byte UDS response instead of six unrelated 0x701 frames.

Source path: MCP2515 RX -> Can -> CanIf -> CanTp -> PduR -> Dcm. Reply path: Dcm -> PduR -> CanTp -> CanIf -> Can -> MCP2515 TX. The legacy DCM service handlers are retained behind a transport adapter for short requests; `22 F186` can accept repeated DIDs and produce a segmented response. Requests above six UDS bytes other than repeated F186 receive NRC `0x13`. This is a service limitation, not transport limitation. MCP2515 accepts a submitted frame into a hardware TX mailbox but has no actual bus TX confirmation callback; PduR success means mailbox acceptance, not proven delivery. The new stack is not yet bench proven against ZGW/FCD. ZGW must correlate a real ECU response for route `0x54` and must not treat its synthetic CAN transmit acknowledgement as ECU execution.

**Build/test evidence:** `make -C "CBM - Central Body Module/CBM_APPL/Debug" all` was attempted and failed because GNU `make` is unavailable. ARM Cortex-M `arm-none-eabi-gcc` and the STM32 HAL/Drivers tree are also absent, so a complete firmware link was not possible. The four portable C modules and `tests/test_com_stack.c` passed `aarch64-none-linux-gnu-gcc -std=c11 -Wall -Wextra -Werror -DCBM_HOST_TEST -fsyntax-only`. This is compile/syntax evidence only: the C harness was not executed because no native C compiler or AArch64 emulator was available. `wsl.exe --list --quiet` returned access denied. The harness contains SF request/response, multi-frame RX/TX, FC, SN error, RX/TX timeout, wrong ID/address and overflow cases, but their runtime results are **unverified**. `git diff --check` passed. Hardware tests still needed: MCP2515 ID/filter reception and arbitration timing, 0x710 ZGW response correlation, FC pacing, TX confirmation and recovery after bus errors. No hardware was flashed.

**Next revision:** obtain a native/ARM build toolchain and STM32 HAL tree, run the C harness and full CBM build, then bench the ZGW route `0x54`. Replace the legacy short-request DCM adapter with bounded full-payload service parsing where required by FCD; add physical TX completion/error reporting from MCP2515. DMU communication architecture is outside this execution.

---


## Architectural continuation

The follow-up request makes ZGW's internal BSW architecture the target, not only wire compatibility. The two existing FreeRTOS applications already have separate BSW, diagnostics, memory and ECU management directories, but their CAN and DCM paths are tightly coupled to ECU logic. The diagnostic service front end now resides separately in each application at `BSW/Diag/Dcm/DcmCore.h`; the repository-level Common dependency was removed. It implements 5-second S3 timing at their existing 5 ms task period, validates and handles `3E` (including suppress-positive-response) and `22 F186`, returns standard negative responses for malformed requests, and maps each ECU's internal session to a UDS session byte. Bounds checks were added to both legacy periodic-DID dispatch tables to prevent out-of-range function-pointer calls. ECU-specific DID handlers and application algorithms remain in each application.

This is **partial architecture migration**. It does not constitute a port of the complete ZGW stack or routed FCD compatibility. The table below records the earlier baseline and is superseded for CBM communication by the architecture revision above.

| ZGW reference | CBM APP | DMU APP | Adapted behavior / missing work | Status and evidence |
|---|---|---|---|---|
| Can | MCP2515 `CanSpi` owns low-level RX/TX | HAL bxCAN1 `CanH` | MCU-specific drivers retained; no ZGW `Can` API yet | Existing source inspected; no bus test |
| CanIf | Direct ID dispatch in `CanSpi` | Direct ID dispatch in `CanH` | Configured PDU table, callbacks and TX confirmation still needed | Missing |
| CanTp | No segmented target transport | No segmented target transport | ZGW extended-address `0x710` and `0x54/0x56` identity need ECU transport adapters | Missing |
| PduR | DCM directly uses `CanSpi_Transmit` | DCM directly uses HAL CAN | TP/IF routing and response correlation still needed | Missing |
| Com | CAN signals parsed in `CanSpi` | CAN signals parsed in `CanH` | Preserve CBM body and DMU cluster semantics while extracting configured signal PDUs and timeouts | Legacy integrated; migration missing |
| CanSM / CanNm / ComM | Local communication state and `0x510` NM handling | Local communication state/sleep and `0x510` NM handling | ZGW lifecycle/state interfaces and current `0x3FF` NM contract not reconciled | Missing |
| Dcm | Legacy `Dcm.c` plus shared DcmCore front end | Legacy `Dcm.c` plus shared DcmCore front end | S3, TesterPresent and F186 now share validation/formatting; service table, full ISO-TP, P2/P2*, async operations and remaining NRCs needed | Partial; source integration and diff check only |
| Dem | ECU-specific `Dem` and DTC arrays | ECU-specific `Dem` and DTC arrays | ZGW event status/config API and NVM block integration still needed | Legacy integrated; migration missing |
| NvM / MemIf / Fee / Fls | Direct sector-backed `Nvm.c`; corrected DTC block copy length | Direct sector-backed `Nvm.c` | Must preserve old layouts and add validation/migration before changing format; AURIX Fee/Fls cannot be copied | CBM copy fix integrated; migration missing |
| OS / application | FreeRTOS 5/10/100 ms tasks, body controls | FreeRTOS 5/10 ms tasks, cluster/camera/recorder | Existing scheduler calls DCM core through each DCM; ZGW initialization/task ownership still to be adapted | Partial |

### FBL and self-updater status

Both standalone FBL projects retain the previous revision's flash bounds and CAN fixes. This continuation adds vector-table validation before application handover: initial MSP must be aligned and in MCU RAM, and the Thumb reset entry must lie in the linked APP window. This rejects an erased or plainly invalid vector but does not prove image integrity. Neither FBL has been converted to the ZGW DCM/CanTp/PduR programming path. ZGW `Fbl_RamUpdater_AURIX.c` executes flash operations from RAM on a TC37x layout; ELC `ram_blob.c` is an STM32F103 implementation with its own page geometry. Neither driver is portable to STM32F411/F407 without re-deriving relocation, vectors, flash sectors, RAM closure and recovery. F93 currently has no separately recoverable bootloader slot or proven ROM recovery transport. A RAM-resident routine that erases sector 0 could strand the ECU after power loss; no FBL self-erase entry was added or claimed safe. A dedicated FBL updater and FCD workflow remain **missing** for both ECUs.

### AUTOSAR DEM continuation — 2026-10-18

The later [compliance matrix](../autosar/COMPLIANCE_MATRIX.md), [DEM trace](../autosar/DEM_SWS_TRACEABILITY.md), [DEM implementation report](../autosar/DEM_IMPLEMENTATION_REPORT.md), and [BSW roadmap](../autosar/BSW_MIGRATION_ROADMAP.md) supersede architectural-completion language in this earlier ZGW-focused report. CBM and DMU now contain separate configuration-driven DEM source, DMU hardware monitoring has moved to an application monitor, and both DCM 0x19 0A paths enumerate DEM DTC/status records. Selected-DTC wrong-code/origin errors now use the R20-11 return values in both DEM copies. DEM remains **partial**: complete client select/clear behavior, records/NvM, DET, exhaustive R20-11 requirement trace, executed tests and target builds are outstanding. The user supplied a local R20-11 DEM PDF as the updated working baseline. AArch64 cross-compiler syntax/link checks passed, but no C test ran. STM32 GCC/make/HAL package were unavailable. No CBM, DMU, ZGW or ELC firmware was flashed; no commit or push was made.

### Validation of this continuation

`git diff --check` passed. Relative shared-header paths resolve from both DCM source directories. The existing FreeRTOS task paths call both DCM MainFunctions every 5 ms, so the new core is in the runtime path. The checked-in managed makefiles specify GNU Tools for STM32 13.3.rel1; rebuilding requires that Cortex-M ARM GCC toolchain, GNU make and the STM32CubeIDE generated project include paths. No STM32 `arm-none-eabi-gcc`, `make`, or native host C compiler was available in the Windows environment; WSL returned access denied. An AArch64 Linux cross compiler was available for syntax/link checks, but its binaries could not execute here. **All four firmware builds and C execution tests remain unverified.** No CAN, FCD, FBL, NVM, or updater behavior was tested on hardware. ZGW and ELC were not changed; nothing was flashed, committed, pushed, deployed or rebooted.

## Scope and evidence

Source-only revision. ZGW and ELC were read as references; neither was modified. No ECU was programmed, reset, or bench tested. The two F93 images remain distinct projects.

| ECU | MCU and application | Physical bus | Application diagnostics | Loader and build |
|---|---|---|---|---|
| CBM | STM32F411RETx; body inputs/actuation | External MCP2515 over SPI, classic CAN | Handwritten DCM, NVM and DEM; request `0x700`, response `0x701` | `CBM_FBL`; STM32CubeIDE managed makefile; FBL `0x08000000`/32 KiB, APP `0x08060000`/128 KiB |
| DMU | STM32F407VETx; cluster, camera and recording | On-chip bxCAN1, classic CAN | Handwritten DCM, camera/cluster DIDs; request `0x702`, response `0x703` | `DMU_FBL`; STM32CubeIDE managed makefile; FBL `0x08000000`/16 KiB, APP `0x08020000`/256 KiB |

Neither MCU/bus controller supports CAN FD. Neither project has a LIN interface. Do not configure these targets as FD or LIN ECUs. These observations are from source and linker scripts; transceiver wiring and oscillator timing require bench confirmation.

## Implemented in this revision

### CBM

- Corrected the FBL MCP2515 first receive filter to the implemented request ID `0x700` (it had been `0x701`, the response ID).
- Unified FBL application start with the APP linker origin `0x08060000`; the application validity read now uses the vector-table start rather than `0x080601A0`.
- Added 32-bit alignment and region bounds checks to flash-word writes. Programming is confined to the APP linker window; existing coding and calibration writes retain their separate sectors.
- Rejects malformed or out-of-region single-frame TransferData with NRC `0x31`, and reports a flash write failure with NRC `0x72` instead of acknowledging the failed word.

### DMU

- Removed a duplicate FIFO-full dispatch that interpreted positive response SID `0x50` as a request; both FIFO callbacks now use the same request path.
- Rejects frames outside CAN request ID `0x702`, non-standard IDs, invalid single-frame lengths, and unsupported segmented PCI shapes before dispatch.
- Confines FBL flash-word writes to the APP linker window `0x08020000..0x0805FFFF`, with alignment/bounds checks and flash lock on errors.
- Narrows APP erase from sectors 5–7 to sectors 5–6, matching that linker window.
- Rejects malformed or out-of-region single-frame TransferData with NRC `0x31`, and reports flash write failure with NRC `0x72` instead of acknowledging it.

The fixes preserve the existing body and cluster application code and their distinct CAN drivers. No application signal layout or transmission period was changed because the gateway DBC and F93 active COM definitions do not yet establish a safe, unambiguous common contract.

## ZGW/FCD interface mapping and blockers

FCD opens DoIP to ZGW (`0x0710` tester, `0x1001` target), prefixes the target route (`0x54` CBM, `0x56` DMU), and ZGW strips that route byte. ZGW selects classic CAN for these route values. Its forwarded CAN TP uses **extended addressing on CAN ID `0x710`**, while the current F93 applications and loaders use plain single-frame CAN IDs `0x700/0x701` and `0x702/0x703`. ZGW lists `0x701/0x703` as COM receive PDUs rather than the forwarded CAN TP receive PDU. There is no verified gateway translation or response correlation from the F93 physical IDs. Thus routed FCD diagnostics and flashing are **not compatible yet**.

ZGW `ParallelFlashSwc` currently permits assumed CAN flow control and can synthesize a positive UDS-looking response after TX confirmation. Such a reply does not prove target execution. An end-to-end test must require a correlated target response and distinguish synthetic acknowledgements. This is a ZGW/FCD owner change; no shared repository was modified.

FCD's generic flash path sends UDS `34 00 44` with four-byte address and size, repeated `36` with BSC, `37`, optional CRC routine, and target-specific erase. Both F93 loaders currently have only a small single-frame protocol, fixed erase paths, no complete ISO-TP, no standard `34` length parser, no rigorous download-size/BSC/CRC lifecycle, and no verified application health or interruption recovery. Do **not** enable the simulated/unselected CBM and DMU FCD targets for vehicle programming. The loader's new bounds checks protect word writes but do not make its erase/download sequence safe for production use.

ZGW/FCD must either provide an explicitly tested gateway mapping for the F93 CAN pairs or agree on a new shared TP channel and request/response addressing, disable synthetic success for these routes, and select ECU-specific flash profiles. F93 then needs matching extended-address ISO-TP in APP and FBL, while keeping CBM MCP2515 and DMU bxCAN implementations separate. ZGW-local DIDs, RIDs and security must not be copied into these ECUs by default.

## Verification

- `git diff --check`: passed.
- Compared CBM and DMU APP/FBL linker origins and lengths with the new FBL bounds.
- Reviewed active ZGW `CanIf`, `CanTp`, `ParallelFlashSwc`, FCD routing/settings, and both F93 CAN/DCM implementations.
- ARM GCC/make toolchain was not discoverable in the available Windows environment; neither managed STM32 project could be rebuilt here. Compilation and linker/map validation remain unverified.
- No host-side transport test was added because these loaders still lack an isolated transport module; a source mirror test would not prove bus behavior.

## Hardware-dependent verification and next revision

1. Agree on physical classic CAN bitrate, transceiver topology, gateway PDU IDs, extended address, and target response correlation. Capture `10 02`, `22 F1 86` and `3E` on each bus; prove replies originate from the ECU.
2. Implement and host-test separate MCP2515 and bxCAN ISO-TP receive/transmit paths with flow control, segmentation, timeout, and malformed-frame handling; then integrate with APP DCM and FBL.
3. Define per-ECU erase RID, download memory range, maximum block length, BSC/retry behavior, CRC and reset/entry contract. Implement complete stateful programming and image validity checks. Protect CBM coding/calibration sectors and DMU non-APP flash.
4. Reconcile active CBM/DMU application frames with ZGW COM/DBC signal ownership and periods. Change only confirmed mappings; test body commands and cluster status separately.
5. Build four images with the STM32 ARM toolchain, inspect ELF/HEX against linker partitions, run host protocol tests, then perform bench-only loss-of-power and recovery tests before any vehicle use.


ELC updater continuation: see [the complete comparison](../bootloader/ELC_FBL_PORTING_MATRIX.md) and [execution/machine-code evidence](../bootloader/RAM_UPDATER_VERIFICATION.md). Both independent default/controlled ARM images and native actual-image replacement simulations pass. Default activation remains interlocked; no physical flashing or reset occurred.
