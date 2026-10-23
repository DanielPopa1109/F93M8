# Current DEM verification supplement — 2026-10-19

The requirement table below retains previously source-reviewed R20-11 IDs. Its old AArch64-only and raw-pair-persistence descriptions are historical and superseded by this supplement and [the closure report](../modernization/F93_REMAINING_GAPS_CLOSURE.md). Full requirement applicability is still not complete; no new requirement ID or compliance result is inferred here.

| Previously reviewed ID/area | Current independent CBM/DMU implementation | Executed evidence | Remaining qualification |
|---|---|---|---|
| SWS_Dem_01183 / nonvolatile status | `Dem_Core.c` explicit serialized runtime schema in configured NvM block 1, CRC/commit-last journal | `test_persistence.c`, `test_migration.c`: reset and legacy import | Full R20-11 cycle/status semantics and physical power-loss unverified |
| SWS_Dem_00391 / confirmation and aging | Confirmation/occurrence/aging counters now serialized; startup clears only per-cycle test flags without falsely aging | Actual-config restore/status assertions | Exhaustive cycle/aging applicability and source-specific capture unverified |
| SWS_Dem_00665 / ClearDTC | DEM_PENDING then NvM-owned durable completion/failure; failed clear restores RAM backup; DCM sends 0x78 and final 0x54/0x72 | `test_persistence.c` plus real DCM/transport/NvM `test_diagnostics.c` | Single client and project status-poll helper; complete standardized error/re-entry/cancellation review open |
| SWS_Dem_00652 / event memory | Primary entries, event index, allocation order and six-byte snapshots now persisted/restored | Snapshot survives simulated reset | Full displacement/entry lifecycle variants and electrical interruption unverified |
| Configured status callback area | `Dem_Cfg` callback dispatch in MainFunction outside IRQ locks; CBM original failure announcement retained | Observable callback in executed host suite; actual hooks M4 compiled | All R20-11 monitor-init/re-enable callback requirements not closed |

Ten current native suites compile and execute; the verification report lists exactly which scenarios are covered. Earlier `test_dem_core.c`/AArch64 statements below are historical and are not presented as executed acceptance. Internal snapshot persistence is supported; UDS snapshot DID encoding is intentionally unconfigured and rejected on wire. DET/MemMap/RTE are outside the explicit project scope and are not dependencies.

---

# DEM R20-11 SWS traceability â€” open, incomplete

Baseline updated by the user: the supplied local `D:\GitHub_Repos\AUTOSAR_SWS_DiagnosticEventManager.pdf`, whose cover identifies [AUTOSAR CP R20-11 DEM SWS, document 019](https://www.autosar.org/fileadmin/standards/R20-11/CP/AUTOSAR_SWS_DiagnosticEventManager.pdf). The original R4.3.1 baseline is superseded **for DEM in this execution**. This table is still **not the exhaustive mandatory requirement map**: the 535-page document and ECUC variability have not been fully classified. Unlisted requirements remain unverified. Do not treat DEM as compliant.

Status meanings: **partial** means some behavior exists without complete SWS semantics; **missing** means required behavior is absent; **unverified** means the applicable R20-11 text or executed test evidence is unavailable. `test_dem_core.c` refers to separate CBM and DMU copies; they compiled and linked for AArch64 but did not execute.

| Official R20-11 ID | Required behavior from the supplied PDF | CBM implementation | DMU implementation | Verification test and actual result | Status |
|---|---|---|---|---|---|
| SWS_Dem_00169 | Distinct pre-initialization and full initialization modes | `BSW/Diag/Dem/Dem_Core.c`: `Dem_PreInit`, `Dem_Init` | Same local module | `tests/test_dem_core.c`: state assertions; linked, **not executed** | Partial |
| SWS_Dem_00180 | `Dem_PreInit` initializes processing state and resets debouncing | `Dem_Core.c`: state and counter reset | Same | PreInit/PREFAILED transition test; linked, **not executed** | Partial: pre-init operation-cycle rules need audit |
| SWS_Dem_00182 | Shut down DEM | `Dem_Shutdown` changes state | Same | Shutdown assertion compiled/linked, **not executed** | Partial: NvM shutdown sequencing unverified |
| SWS_Dem_00183 / 00198 / 00203 | Report event status; retrieve event DTC and fault detection counter | `Dem_SetEventStatus`, `Dem_GetDTCOfEvent`, `Dem_GetFaultDetectionCounter`; wrong DTC format returns code 10 | Same | Status/invalid-event and DTC-format assertions compiled/linked, **not executed**; FDC API cases absent | Partial: all standardized errors and all debounce configuration variants unverified |
| SWS_Dem_00194 / 00679 | Asynchronous operation-cycle restart and configured monitor-restart callbacks | Project-specific synchronous `Dem_SetOperationCycleState`; standard restart entry/callback absent | Same | No restart/callback test | Missing standard restart path |
| SWS_Dem_00389 | UDS bit 1 transition, failed this operation cycle | `Dem_Core.c`: failure/start transitions | Same | Status/cycle assertions; linked, **not executed** | Partial: nonvolatile cycle status not complete |
| SWS_Dem_00390 | UDS bit 2 PendingDTC transition | `Dem_Core.c`: fail and clean cycle end | Same | Failure and cycle assertions; linked, **not executed** | Partial: displacement and all operation-cycle cases not proven |
| SWS_Dem_01183 | PendingDTC status is nonvolatile | Legacy packed DTC word through `BSW/Mem/Nvm.c` | Legacy packed DTC word through `BSW/Mem/Nvm/Nvm.c` | Legacy restore assertion; linked, **not executed**; power-loss test absent | Partial |
| SWS_Dem_00391 | UDS bit 3 ConfirmedDTC transition | `Dem_Core.c`: configurable threshold and aging | Same | Confirmation/aging assertions; linked, **not executed** | Partial: failure-cycle counter is not persistent |
| SWS_Dem_00057 | Client-local DTC filter persists until reset/new filter | `Dem_Core.c`: single configured client ID 0 and cursor | Same | Enumeration assertions; linked, **not executed** | Partial: no multiple-client configuration |
| SWS_Dem_00208 / 00214 | Configure client filter and count matching DTCs | `Dem_SetDTCFilter`, `Dem_GetNumberOfFilteredDTC` | Same | Filter/count assertions compiled/linked, **not executed** | Partial: severity/FDC filter options rejected; no pending behavior |
| SWS_Dem_01252 | Accept only configured client IDs; report DET on invalid client | Client 0 checked; no DET | Same | Invalid-client test absent | Partial/missing DET |
| SWS_Dem_01253 | Selected DTC/group is target for subsequent clear/status/record APIs | `Dem_SelectDTC` targets clear/status, but record APIs still use direct DTC | Same | Select/clear test compiled/linked, **not executed** | Partial |
| SWS_Dem_00060 | Return configured UDS status availability mask for 0x19 | `Dem_GetDTCStatusAvailabilityMask`, configured 0x7F | Same | DCM 0x19 source path inspected; linked test, **not executed** | Partial: not ECUC-derived |
| SWS_Dem_00657 | Mask DTC status bytes supplied to DCM | `Dem_GetNextFilteredDTC` masks by configured availability | Same | No status mask execution | Partial |
| SWS_Dem_00933 / 00934 | Standard UDS DTC format and primary-memory origin types/values | `Dem_Types.h`: UDS 1, origin `uint16`/1 | Same | Compile/link only | Partial |
| SWS_Dem_01198 | All-DTC group symbol represents `0xFFFFFF` | `Dem_Types.h`: `DEM_DTC_GROUP_ALL_DTCS` | Same | Group clear assertion compiled/linked, **not executed** | Partial: group-selection variants beyond all-DTC unconfigured |
| SWS_Dem_91016 | Client selects DTC or group with format and origin | `Dem_SelectDTC` stores client-0 selection | Same | Select/clear assertion compiled/linked, **not executed** | Partial: no multi-client or busy processing |
| SWS_Dem_91023 / 91020 | Report selected DTC validity, including wrong DTC and origin for general and clear selection | `Dem_GetDTCSelectionResult` / `Dem_GetDTCSelectionResultForClearDTC`; R20-11 numeric codes 8 and 9 | Same | Invalid DTC/origin assertions compiled/linked, **not executed** | Partial: synchronous checking only; no pending/busy handling |
| SWS_Dem_00212 / 00213 | Return status of selected DTC and configured status availability mask | `Dem_GetStatusOfDTC` / `Dem_GetDTCStatusAvailabilityMask` | Same | Selected-status/mask assertions compiled/linked, **not executed** | Partial: one client/origin; no asynchronous retrieval |
| SWS_Dem_00215 | Enumerate filtered DTCs and return `DEM_NO_SUCH_ELEMENT` when exhausted | `Dem_GetNextFilteredDTC`; return code 48 | Same | Enumeration/exhaustion assertions compiled/linked, **not executed** | Partial: synchronous single-client cursor only |
| SWS_Dem_00665 | Clear the client-selected DTC/group with specified completion and errors | `Dem_ClearDTC(0)` clears RAM/legacy image | Same | Clear assertion compiled/linked, **not executed** | Partial: NvM completion/error semantics missing |
| SWS_Dem_00242 / 00243 | Client-based disable/enable DTC setting | `Dem_DisableDTCSetting` / `Dem_EnableDTCSetting` | Same | DCM call source inspected; no execution | Partial: storage/event semantics unverified |
| SWS_Dem_00680 / 00682 | Invoke configured event-monitor callbacks after clear and DTC-setting re-enable | No callback configuration or dispatch | Same | No callback test | Missing, pending applicability/configuration decision |
| SWS_Dem_00079 / 00080 / 00626 | Disable/enable storage and UDS updates; ignore status reports while disabled | `Dem_SetEventStatus` returns success without changing state when disabled | Same | Disabled-setting assertion compiled/linked, **not executed** | Partial: debounce freeze/reset details unverified |
| SWS_Dem_00683 / 00927 | Reset or freeze event debounce counter using standardized action values | `Dem_ResetEventDebounceStatus` | Same | Freeze/reset assertions compiled/linked, **not executed** | Partial: timer-based debouncing not configured |
| SWS_Dem_00652 | Return number of entries in client-selected event memory | `Dem_GetNumberOfEventMemoryEntries` | Same | Four-slot count assertion compiled/linked, **not executed** | Partial: RAM allocation state is not fully persisted |

## Behaviors pending complete R20-11 requirement classification

These implementation points do **not** count as SWS closure. Exact IDs, mandatory applicability, error returns, ECUC switches, and tests require a full review of the supplied R20-11 text.

| Area | CBM and DMU source | Test/evidence | Result |
|---|---|---|---|
| `Dem_SetEventStatus`, PREFAILED/PREPASSED and FAILED/PASSED | `Dem_Core.c`, `Dem_Cfg.c` | Threshold test compiled/linked only | Partial; reset/jump behavior and all event parameter variants unverified |
| Event occurrence, confirmation, aging | `Dem_Core.c` | Counter/cycle test compiled/linked only | Partial; packed legacy record lacks confirmation persistence |
| Primary memory/displacement | Four slots, priority and oldest-order selection in `Dem_Core.c` | Displacement test compiled/linked only | Partial; event memory records and displacement semantics need SWS review |
| Freeze frame and extended data | Internal six-byte snapshot, three-byte counters in `Dem_Core.c` | Read test compiled/linked only | Partial; snapshot is internal metadata, lacks configured application data and NvM persistence |
| DTC clear | `Dem_SelectDTC`/`Dem_ClearDTC` and DCM 0x14 | Select/clear test compiled/linked only | Partial; synchronous return precedes full NvM lifecycle, persistence result not owned by DEM |
| DTC setting control | `Dem_SetDtcSetting` and DCM 0x85 | No dedicated test | Partial; API and event-processing semantics need SWS reconciliation |
| NvM migration | Existing CBM 26-word and DMU 18-word flash pair layouts retained; packed UDS status/occurrence/aging in high bytes | Legacy record test compiled/linked only | Partial; no failure injection or power-cycle execution |
| Invalid inputs and concurrency | `E_NOT_OK` guards, `SchM_Dem.h` PRIMASK section | Invalid event test compiled/linked only | Partial; getters and filter operations lack complete critical-section and reentrancy coverage |
| DET, version, Compiler/MemMap | No complete integration | None | Missing or applicability unverified |

The R20-11 DEM SWS describes client-ID-based DCM filter use for diagnostic service 0x19. This implementation supports only one DCM client (ID 0). The current filter, select and clear call shapes include that ID; snapshot/extended record APIs and several return values remain project-specific. The separate DCM module remains on the original R4.3.1 baseline until its own audit.

The supplied SWS explicitly defines `DEM_DTC_FORMAT_UDS = 1` (`SWS_Dem_00933`) and `DEM_DTC_ORIGIN_PRIMARY_MEMORY = 0x0001` with a `uint16` origin (`SWS_Dem_00934`). Both local `Dem_Types.h` files now use these values. The R20-11 service interface defines `DEM_WRONG_DTC = 8` and `DEM_WRONG_DTCORIGIN = 9`; both DEM copies return these for invalid client selections. `Dem_SelectDTC` (`SWS_Dem_91016`), client-based `Dem_ClearDTC` (`SWS_Dem_00665`), and client-based DTC setting APIs (`SWS_Dem_00242`, `_00243`) are present, while complete behavior and remaining error codes are partial.
