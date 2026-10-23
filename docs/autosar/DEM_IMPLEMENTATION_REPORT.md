# Current continuation — 2026-10-19

See [remaining-gaps closure](../modernization/F93_REMAINING_GAPS_CLOSURE.md), [current compliance matrix](COMPLIANCE_MATRIX.md), and [executed verification](../modernization/F93_VERIFICATION_RESULTS.md). Queued transactional persistence, durable DEM clear, full UDS DCM, configured COM and routed FBL programming are now implemented to the documented scope. Full SWS classification, APP builds, physical validation and resident-updater activation remain incomplete. The material below is historical where it conflicts.

---

# DEM implementation report â€” 2026-10-18

**Result: partial implementation in both independent applications; AUTOSAR R20-11 DEM compliance is unverified and unmet.** The user supplied the R20-11 DEM PDF as the working baseline after its version was identified. This execution added source paths and project build entries, but the full SWS trace, complete select-and-request behavior, persistence semantics, target builds, and executed tests remain open. See [traceability](DEM_SWS_TRACEABILITY.md) and [module matrix](COMPLIANCE_MATRIX.md).

## Source changes

| ECU | Changed behavior and files |
|---|---|
| CBM | Local `BSW/Diag/Dem/Dem_Core.c`, `Dem_Cfg.c/.h`, `Dem_Types.h`, `SchM_Dem.h`, adapted `Dem.c/.h` and `BSW/Std_Types.h`. `Core/Src/freertos.c` calls PreInit and MainFunction; `BSW/Mem/Nvm.c` restores before Init; `BSW/Diag/Dcm/Dcm.c` assembles 0x19 0A records through DEM and selects/clears DTCs through DEM, then calls the legacy synchronous flash writer and reports write failure. `BSW/Sys/EcuM/EcuM.c` no longer writes raw DTC words on startup. Existing CBM monitor calls remain through `Dem_SaveDtc` adapter; one-byte fault CAN notification moved to `CanSpi.c`. Managed `.cproject`/Debug build inputs include new DEM sources. |
| DMU | Independent local DEM files and `Std_Types.h` as above, with nine DMU events. Hardware fault evaluation moved from DEM to `APPL/DiagMonitor/DiagMonitor.c/.h`, called by `Core/Src/freertos.c`. `BSW/Mem/Nvm/Nvm.c` restores before Init; `BSW/Diag/Dcm/Dcm.c` uses DEM for 0x19 0A, 0x14 and 0x85. Managed `.cproject`/Debug build inputs include DEM and DiagMonitor. Existing transport and application CAN code remains separate from CBM. |

Configured source behavior: CBM retains 13 event IDs and its 26-word flash pair layout; DMU retains nine IDs and its 18-word layout. The low status byte remains legacy `1`/`2`; high bytes now carry UDS status, occurrence and aging. Erased or mismatched event-ID pairs are rejected on restore; the format has no CRC and cannot detect all corruption. Four RAM primary-memory slots, configurable thresholds, one-cycle confirmation, 40-cycle aging, single-client filtering, snapshot metadata, counter extended data, and PRIMASK critical sections have been added. **The numeric UDS DTC codes are provisional, and flash compatibility is source-level only.** No real old-image/new-image power-cycle migration was executed.

Both local DEM copies now return the R20-11 `DEM_WRONG_DTC` (8) and `DEM_WRONG_DTCORIGIN` (9) codes for invalid selected DTCs/origins, `DEM_E_NO_DTC_AVAILABLE` (10) for an unsupported DTC format in `Dem_GetDTCOfEvent`, and `DEM_NO_SUCH_ELEMENT` (48) when a DTC filter is exhausted. The test harnesses assert these paths, but the assertions have only compiled and linked.

## Verification actually performed

| Check | Result |
|---|---|
| Official DEM source | Supplied local 535-page PDF cover says R20-11; user directed this as the working DEM baseline. Key type/API/behavior sections extracted and inspected. Exhaustive R20-11 applicability review **not executed**. |
| Build metadata | `.cproject`, `Debug/sources.mk`, `objects.list`, `makefile`, and DEM/DiagMonitor `subdir.mk` include new sources. Source-level inspection only. |
| CBM and DMU DEM core + `tests/test_dem_core.c` | AArch64 GCC 15.3.1 `-Wall -Wextra -Werror -DDEM_HOST_TEST` syntax checks and links **passed**. Binaries could not execute on this Windows host. Assertions are **unexecuted**. |
| DMU application monitor + `tests/test_dem.c` | Same cross compiler syntax check and link **passed**; test **unexecuted**. |
| DMU Can/CanIf/CanTp/PduR/DCM + `tests/test_com_stack.c` | Same cross compiler syntax check and link **passed**; transport and 0x19 assertions **unexecuted**. |
| STM32 application builds | **Could not start:** `arm-none-eabi-gcc`, GNU `make`, and project-referenced `STM32Cube_FW_F4_V1.28.1` package were absent from inspected installations/PATH. No ELF/map generated. |
| Static checks | `git diff --check` passed after build-file whitespace correction; no DMU DEM/DiagMonitor source/build dependency on CBM or ZGW found. |
| Bench/hardware | **Not executed.** No firmware flashed, ECU reset, commit or push. CM5 was not accessed, modified or rebooted. |

## Material deficiencies blocking completion

1. R20-11 DEM has client-local selection and request APIs. `Dem_SelectDTC`, client-based `Dem_ClearDTC`, `Dem_GetStatusOfDTC` and DTC setting calls now exist for client 0. They do not yet implement complete busy/async/error behavior, multiple clients, or selected record retrieval. Other record APIs retain project-specific direct DTC arguments.
2. Primary event memory and snapshots live in RAM. The legacy flash pairs restore status/occurrence/aging but do not preserve snapshot content, primary allocation order, confirmation counter or complete operation-cycle state. No NvM block/job notification interface, CRC/redundancy, flash-failure recovery proof or full migration test exists.
3. UDS status bit transitions and aging need full R20-11 state-machine verification. The current snapshot is internal metadata, not configured application freeze-frame data; extended data is limited to three internal counters.
   R20-11 also defines asynchronous `Dem_RestartOperationCycle` and configured monitor-reinitialization callbacks. The current cycle API is a synchronous project adapter and these standard paths are missing.
4. DET, optional version information, complete `Platform_Types`, Compiler/MemMap, ECUC-generated configuration, and complete SchM/reentrancy integration are missing. Selection now distinguishes wrong DTC/origin, but remaining `E_NOT_OK` guards do not satisfy all specified return codes.
5. DCM services remain handwritten and narrow. Both support only 0x19 subfunction 0x0A from DEM; 0x14/0x85 do not yet follow the full standard DEM selection/clear/setting lifecycle. ECU-specific DTC assignment is not confirmed.
6. No native Windows C compiler or permitted WSL runtime was available. All requirement assertions and persistence/concurrency failure-injection tests remain unexecuted. Target builds and bench behavior are unverified.

## Acceptance status

This work advances both applications beyond raw DTC helpers and preserves ECU-local implementation files. It **does not meet the requested complete DEM acceptance gate**. Finish the supplied R20-11 SWS/ECUC applicability audit, close the traceability rows, implement the missing standard APIs and persistence behavior, execute requirement tests, and build each STM32 image before reconsidering compliance.
