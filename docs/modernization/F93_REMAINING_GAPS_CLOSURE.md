# F93 remaining gaps closure — 2026-10-19

**Overall: PARTIAL; no deployment or hardware-programming approval implied.** This is an implementation continuation across M1–M8. Native host execution, Cortex-M4 object compilation and complete freestanding FBL links are distinguished below. No complete AUTOSAR compliance, complete APP build, bench success, vehicle success or recoverable resident-FBL replacement is claimed.

Only F93 was modified. ZGW and its FCD source were read-only. The existing dirty tree was retained. No ECU/CM5 was connected, flashed, modified or rebooted; no software was deployed; no commit/push occurred. `codex_credentials.env` is ignored. No credentials are included in artifacts.

## Finding disposition

The repository's `STATIC_AUDIT_REVIEW.md` has an original **F01–F12** table, headed 2026-10-18, with later implementation notes. Neither its text nor the other supplied repository documents contains the original F13/F14 findings. They are explicitly retained as unresolved audit-identification rows below. Numbering was not invented from the different numbering in the new request's section headings.

| Finding from the available audit | Disposition | Current implementation and evidence / remaining gate |
|---|---|---|
| F01 DMU DEM bounds | Fixed and verified in host configuration | Nine events use bounded indexes; actual DMU descriptors, restore, status/filter and durable clear execute in the native suites; board monitors have Cortex-M object evidence. |
| F02 CBM FlowControl | Fixed and verified in executed host cases | EA/PCI/BS/STmin offsets, segmented request/response, BS gating, physical-confirmation waits, wrong sequence and timeouts execute. Exhaustive ISO 15765/SWS edge cases and bus timing remain unverified. |
| F03 APP/FBL transport mismatch | Implemented but unverified on a bus | Both FBLs now own Can/CanIf/CanTp/PduR on standard 0x710, target EA 0x54/0x56, response EA 0xF0, gateway FC EA 0x41. Real independent FBL stack host tests and complete ARM FBL ELF links pass. No legacy 0x700/0x701 or 0x702/0x703 dispatch remains in FBL runtime. |
| F04 premature erase success | Implemented but unverified on STM32 | An NRC 0x78 must receive physical TX confirmation before APP erase runs. Unlock/erase/lock and erased-word verification precede 0x71; failures return 0x72. Host sequencing/failure assertions pass. HAL erase remains synchronous; worst-case P2* under voltage/watchdog/interrupt load requires bench measurement. |
| F05 NM3 | Partial | Exact one-byte 0x3FF active-0xFF validation, remote expiry, logical full/silent/no modes, application-TX gating, bus-off backoff and return are connected. Diagnostics stay on the controller independently of application COM groups; sleep waits for persistence and diagnostic activity. Physical sleep/wake/transceiver recovery is not demonstrated. |
| F06 DCM/DEM | Partial | Both DCMs process complete UDS payloads with configured service lengths/session masks, S3/P2/P2*, retained buffers, asynchronous clear/reset/program transitions and physical final-response confirmation. Both DEMs persist events/primary entries/snapshots/counters and invoke configured callbacks. Full SWS semantics, diagnostic snapshot DID assignment and all services/security variants remain open. |
| F07 memory architecture | Implemented and host-verified; APP link/target unverified | Independent queued NvM, descriptors, ReadAll/WriteAll/block/status/default/invalidate APIs, MemIf journal, Crc and STM32 flash backend exist. Native tests exercise corruption, interrupted append/rollover, migration and durable clear. No complete NvM SWS claim. |
| F08 flash locking/recovery | Implemented but unverified on STM32 | Writable ranges restrict the backend to its configured sectors; lock paths and error returns exist. Replacement records commit last after readback, keeping the previous bank valid through rollover. Three bounded attempts/backoff and unchanged-write suppression execute in simulation; electrical power-loss/controller behavior is unverified. |
| F09 FBL self-updater | Implemented, ARM-linked and host-tested; deployment interlocked | Both independent ELC-style SRAM updaters own CAN/ISO-TP/UDS, full candidate identity/vector/CRC validation, F4 resident erase/program/readback/CRC and confirmed reset. Default activation stays interlocked; the enabled path links and actual linked images pass simulation. DMU FBL enlarged to 32 KiB without moving data/APP. No board recovery or power-loss recovery is proved. See [RAM verification](../bootloader/RAM_UPDATER_VERIFICATION.md). |
| F10 linker overlap | Fixed and statically verified | Four MEMORY maps and new APP-footer/retained-mailbox reservations pass checks. Both actual FBL ELFs fit their configured FLASH/RAM. Complete APP ELF/map evidence remains unavailable. |
| F11 CAN TX semantics | Implemented but unverified on hardware | MCP2515 TXREQ/TXnIF/ABTF/TXERR and bxCAN completion/abort callbacks feed CanIf/CanTp; final TP success waits for confirmation. Actual callback wiring compiles for Cortex-M. Executed host tests prove queue acceptance alone cannot finish TP or trigger reset/erase. Hardware ACK/error/bus-off traces remain required. |
| F12 project/build integration | Partial | Debug/Release project metadata, local include paths, generated Debug source/rule/object chains and source boundaries pass checks. Complete FBL Clang/LLD builds pass. CubeIDE/GCC APP links and middleware integration remain unverified. |
| F13 original finding text unavailable | Blocked for original-audit traceability | Original F13 text was not present in the supplied audit or repository. This is not silently considered fixed. Obtain the original finding and map it to the implementation/tests. |
| F14 original finding text unavailable | Blocked for original-audit traceability | Same limitation as F13; no fabricated original finding title or closure claim. |

## Application module completion matrices

`Implemented/host verified` describes the configured behavior and executed cases, not full SWS conformance or deployment readiness. These are separate project-local implementations/configurations, with no Common firmware directory or cross-project source include.

| Module | CBM APP | DMU APP |
|---|---|---|
| Can | MCP2515 mapping and polled physical completion/error ownership; ARM object compiled; hardware unverified | bxCAN mailbox ownership, completion/abort and bus-off callbacks; ARM object compiled; hardware unverified |
| CanIf | Configured diagnostic mapping plus bounded application IDs and physical callbacks; host verified | Same independently owned implementation; host verified |
| CanTp | 256-byte extended-address SF/FF/CF/FC, timing/sequence/block handling, confirmation-dependent TX; host verified subset | Same with EA 0x56; host verified subset |
| PduR | Initialized diagnostic route, request/error/final-TX callbacks; host verified; narrower than complete AUTOSAR buffer API | Same independent route; host verified |
| Com | Configured RX fields and two periodic TX PDUs, bit packing/unpacking, validation, groups, deadline/default actions and notifications; host core verified | Configured cluster/body RX fields and alive/mileage TX PDUs with original periods; host core verified |
| CanNm | One-byte ZGW NM3 follower and timeout/state transitions; host verified subset | Same independent follower; host verified subset |
| CanSM | Logical communication state/TX gate and bounded bus-off quiet period; physical controller/transceiver coordination partial | Same; bxCAN automatic bus-off recovery configured; physical recovery unverified |
| ComM | One local channel/user plus remote-NM request arbitration; multi-user/inhibition/complete SWS partial | Same independent single-channel configuration |
| Dcm | Full payload dispatcher replaces legacy six-byte/frame adapter; existing coding/calibration/periodic reads retained through ECU hooks; host core verified | Cluster/camera/mileage routines and reads retained in own hooks; host core verified |
| Dem | Configured events, UDS state, debounce/cycles/confirmation/aging, allocation/displacement, persistent records, filter/clear/callbacks; host verified subset | Same independent runtime; nine original monitors retained |
| NvM | DEM 512-byte block + calibration 40 bytes + coding 24 bytes; all participate in ReadAll/WriteAll; real migration host verified | DEM 512-byte block + original two mileage words; all participate; real migration host verified |
| MemIf | Two-bank, append/commit-last CRC journal and tombstone invalidation; host failure injection verified | Same independent journal on different physical sectors |
| Crc | Reflected CRC32 with continuation; known-vector and persistence/programming tests pass | Same independently owned implementation |
| STM32 flash | HAL erase/program/readback, write bounds and controller relocking; ARM object evidence; hardware unverified | Same with F407-specific sector mapping |
| Init/scheduling/types/error handling | Existing FreeRTOS architecture retained; NvM/DEM progression has one diagnostic-task owner, including startup/postrun; source/static integration verified | Startup deadlock prevented by progressing NvM outside RUN-only gating; full scheduler build still requires the Cube middleware |

Remaining common limits: one diagnostic client/connection; provisional DTC numeric assignments from prior configuration; internal six-byte snapshots are persisted, but **UDS 19 04 is rejected with NRC 0x31 until snapshot DID/record encoding is configured**. UDS 19 06 formats the DTC status plus configured record number/counters. Full ECUC and SWS requirements are not exhaustively classified. RX date/RTC effects and original monitor fallback behavior remain ECU-specific. Locking covers DEM mutations and queued NvM updates; complete task/ISR/SPI reentrancy and worst-case response timing need review/bench evidence.

## Persistence and migration contract

- CBM journal banks: sector 5 at 0x08020000 (128 KiB), sector 6 at 0x08040000 (128 KiB). APP remains sector 7; legacy DEM sector 4 and calibration/coding sectors 2/3 are preserved by APP journal operations.
- DMU journal banks: sector 4 at 0x08010000 (64 KiB), sector 7 at 0x08060000 (128 KiB). APP remains sectors 5/6; legacy mileage sector 2 and DEM sector 3 remain available for migration.
- Each 1024-byte record contains a magic, sequence, configured block ID/version/length entries, payloads, image CRC32 and final commit word. Scanning accepts only valid committed records and selects the newest sequence. Dirty/torn slots are skipped. Bank rollover erases the alternate bank while retaining the previous committed image.
- A write snapshots its RAM payload into the bounded queue. No NvM flash operation occurs in the enqueue API. MemIf compares unchanged data before writing. Controller operations happen in MainFunction; retries are bounded to three with backoff. CBM postrun has a latch, preventing continuous WriteAll erase attempts after a failure.
- The record is a journal snapshot of the currently committed block dataset. A single block commit is verified; **WriteAll is not claimed to be an all-block atomic transaction**. CRC detects accidental corruption; it is not authentication or a guarantee against every physical failure.
- When a configured block has no usable journal entry, ROM defaults are installed and the original ECU-specific raw address is imported where present. DTC pairs are validated. Imported legacy values had no original CRC; their prior integrity cannot be proven retroactively. Valid tombstones restore defaults without resurrecting legacy data. Migration does not erase the legacy sectors.
- `Dem_Serialize` stores an explicit versioned byte schema, independent of C structure padding: runtime status/debounce/confirmation/occurrence/aging, primary allocation order and snapshot bytes. Startup starts a new cycle without falsely aging untested events. ClearDTC returns DEM_PENDING, commits the cleared block, then publishes completed/failed status; DCM cannot send durable success beforehand. Failed clear restores the RAM backup and returns NRC 0x72.

## FBL programming and updater status

Both FBLs now have independent CAN adapters, a 5 ms transport/service loop and the routed addressing above. FCD source was inspected at `Scripts/FCD/FCD.pyw`: RIDs 0x0001/0x0002, RequestDownload `34 00 44 + address_be32 + size_be32`, advertised maxBlockLength, SID/BSC overhead, modulo-256 BSC and optional CRC in TransferExit. No AURIX addresses/controller code were reused.

APP programming accepts a **contiguous image starting at the linked APP origin**, bounded below the reserved footer. Maximum UDS request length is 256 bytes, so TransferData carries at most 254 data bytes; FCD also applies its routed limit. Exact duplicate BSC/data returns the previous acknowledgement without writing again; changed duplicate data or a wrong BSC is rejected. Missing bytes, transfer expiry, invalid ranges, failed flash/readback and CRC mismatches fail explicitly. Long CRC verification runs in bounded chunks after a physically confirmed NRC 0x78. Completion writes a size/CRC/complement record and commits its validity word last. Boot revalidates the complete recorded image and vector bounds.

**Packaging limits:** arbitrary sparse/multiple Intel HEX segments are not yet a supported programming transaction. Repeated per-segment whole-APP erase would destroy earlier segments; this implementation rejects non-origin downloads rather than pretending support. APP linker FLASH lengths are reduced by 1 KiB for the footer. Existing unversioned applications do not contain this footer and are not automatically trusted by the new FBL; a coordinated rebuild/package migration is required before deployment. FCD and ZGW were not changed to work around these target limitations.

CBM additionally retains sessions 0x04/0x05 for its original coding/calibration address contracts. These collect exactly 24/40 bytes and commit block 3/2 through an independent FBL-owned copy of the parameter journal, preserving both old raw sectors and the other journal blocks. A positive TransferExit follows the journal commit. APP reads the same block schema after reset. The original legacy single-frame downloader has been replaced, not left as an alternate routed programming path. DMU has no copied CBM parameter contract.

| Image | Current status |
|---|---|
| CBM FBL | Real programming/transport tests and complete Clang/LLD Cortex-M ELF; board timing and routed bench sequence unverified |
| DMU FBL | Same; particularly small remaining flash margin requires the configured size optimization and a GNU ARM map check |
| CBM FBL updater | Complete independent SRAM path implemented, default/controlled Cortex-M links and full replacement/error host tests pass; default deployment interlocked; no persistent resume or power-loss recovery |
| DMU FBL updater | Independent F407/bxCAN SRAM implementation, 32 KiB resident bound, same software verification and physical-recovery gate |

The current board evidence does not establish accessible BOOT0/system-memory UART/USB/SWD recovery, a separately protected recovery loader, or a recoverable alternate resident image. Enabling activation requires board access proof, a validated recovery/provisioning procedure, flash-sector/image bounds, RAM vector/stack/helper/ISR relocation, watchdog management, and interruption testing at every destructive step. Changing a macro alone cannot enable activation: the header deliberately rejects such a build, and `FblUpdater_Activate` returns failure.

## Executed verification and build results

Commands executed from F93:

```text
python tools/run_host_tests.py
python tools/check_cortex_sources.py
python tools/build_fbl_validation.py
python tools/inspect_memory.py
python tools/audit_modernization.py
git diff --check
make -C "CBM - Central Body Module/CBM_APPL/Debug" all
```

Ten native Windows C executables were compiled with Clang (`ziglang 0.13.0`), `-std=c11 -Wall -Wextra -Werror`, and executed: separate CBM/DMU persistence, programming, diagnostic/communication, actual-config migration, and FBL transport suites. They use the actual module C sources with explicit physical-hardware/ECU-I/O mocks. Earlier historical AArch64 harness results are not counted as executed tests.

Executed cases include known CRC32, default/read/write/status/invalidate, unchanged-write suppression, corruption fallback, append and bank-rollover cut points, DEM reset/snapshot/counter restore, successful and failed durable clear, configured callbacks, full multi-frame UDS, FC BS/STmin/sequence/timeouts, TX acceptance versus completion, reset/erase confirmation, COM minimum DLC/group/timeout, NM3 activation/expiry and bus-off gate, FCD download/BSC/changed duplicates/TransferExit integrity, invalid addresses, transfer timeout, image-validity rejection, CBM parameter commit and updater interlock/reset loss.

`check_cortex_sources.py` compiles production BSW, ECU hooks and FBL board code for **thumb-freestanding-eabi / Cortex-M4 / hard-float** against pinned official ST HAL v1.8.3, CMSIS-device-F4 v2.6.9 and ARM CMSIS 5.9.0. These are real ARM objects, not AArch64 Linux objects. Final counts: CBM APP 21/21; DMU APP 22/22; CBM FBL 19/19; DMU FBL 14/14. The APP count excludes the unbuildable FreeRTOS scheduler translation unit; scheduler wiring has source/static evidence only.

Complete independent FBLs link with their real startup, board code, enabled official HAL sources, freestanding constructor initialization, compiler runtime and actual linker scripts. No fake HAL is linked into these ELFs. Unused hosted syscall/heap stubs are excluded from this freestanding validation build; CubeIDE retains its normal runtime build. Debug and Release FBL C configurations explicitly use size optimization; Debug retains symbols. GNU GCC/CubeIDE output is still a separate required check.

The attempted APP make command fails before compilation because GNU make is missing. `arm-none-eabi-gcc` and STM32CubeIDE are unavailable; the configured `C:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1` HAL/FreeRTOS/CMSIS middleware tree is missing. Install the matching CubeIDE ARM GCC/make and firmware pack, restore DMU graphics/middleware dependencies as required by its project, regenerate Debug/Release rules, then build both complete applications. The downloaded validation headers do not substitute for those APP dependencies.

Memory evidence is in `tools/.build/memory-results.json` and ELF-derived `.map` text (the installed Zig linker does not support GNU `-Map`). These are **ELF section maps**, not claimed GNU linker maps. Final figures are refreshed in the verification snapshot. Four FLASH linker MEMORY ranges have no internal overlap; APP-footer and retained-mailbox assertions pass. Complete APP stack/heap/section fit cannot be verified without its ELF.

## Remaining work, prioritized

1. Full CubeIDE/GNU ARM Debug and Release APP/FBL builds; inspect APP map/stack/heap and GNU FBL size margins, source discovery, middleware paths and all task initialization. Review complete task/ISR ownership and SPI error propagation.
2. Establish a contiguous APP packaging/migration contract and footer provisioning workflow for FCD; support sparse/multiple segments only with an explicit target transaction model. Verify the fixed ZGW/FCD route with controlled CAN traces.
3. Bench-test actual MCP2515/bxCAN completion, errors/bus-off recovery, NM3 timeout/wake, controller/transceiver modes, worst-case erase/P2* and watchdog behavior. Inject real power loss during append/rollover, programming and metadata commit.
4. Complete formal applicability/traceability review against DEM R20-11 and the retained R4.3.1 module SWS. Configure snapshot DIDs/application capture data and any required security/access, DCM OpStatus/cancellation, standard buffer APIs, multi-user/channel and additional DEM features. No placeholders or positive success are used for unavailable activation/record encoding.
5. Recover original F13/F14 wording and map those findings; do not infer their identities from this continuation.
6. Prove an independent resident-FBL recovery arrangement before implementing destructive RAM execution/activation. Leave the current interlock enabled until the required recovery evidence exists.

See [source manifest](F93_SOURCE_MANIFEST.md) and [verification snapshot](F93_VERIFICATION_RESULTS.md). The older reports below this continuation remain historical evidence, not current readiness statements.


## ELC SRAM-updater implementation continuation

The [ELC porting matrix](../bootloader/ELC_FBL_PORTING_MATRIX.md), [RAM verification](../bootloader/RAM_UPDATER_VERIFICATION.md) and refreshed [executed results](F93_VERIFICATION_RESULTS.md) supersede earlier staging-only activation claims. Both default and controlled builds plus twelve native suites pass. Source, build, host execution and hardware remain distinct; no ECU/CM5 was modified or rebooted and no software was deployed. G26 was outside this repository task.
