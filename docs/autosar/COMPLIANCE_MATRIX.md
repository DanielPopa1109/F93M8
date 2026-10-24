# F93 BSW implementation/compliance matrix — 2026-10-19

> 2026-10-24 update: See the four-project review at ../modernization/CURRENT_STATE_REVIEW_2026-10-24.md and selected DCM trace at DCM_SWS_TRACEABILITY.md for CommunicationControl, S3 and DMU bus-off findings. The matrix below remains a functional inventory, not a conformance certificate.

**No complete AUTOSAR Platform or module compliance claim.** DEM retains the supplied R20-11 SWS baseline; other modules retain R4.3.1. ZGW is the architecture/interoperability reference, not a substitute for the official SWS. Existing verified DEM requirement text is retained in [DEM_SWS_TRACEABILITY.md](DEM_SWS_TRACEABILITY.md); unreviewed IDs/applicability remain unverified.

Current functional evidence is in [the closure report](../modernization/F93_REMAINING_GAPS_CLOSURE.md) and [executed verification](../modernization/F93_VERIFICATION_RESULTS.md). Every ECU has independent firmware implementation/configuration. The table describes configured behavior, not compliance certification.

| Module | CBM | DMU | Verification and remaining scope |
|---|---|---|---|
| Can | MCP2515 SPI/mailbox completion/error ownership | bxCAN mailbox completion/abort/bus-off callbacks | Actual board sources compile for M4; transport completion executes with hardware mocks. Electrical/controller evidence missing. |
| CanIf | Diagnostic and application mappings, physical callbacks | Independent equivalent mappings/callbacks | Host source integration verified; full SWS controller/PDU/wakeup API set unverified. |
| CanTp | EA 0x54, 256-byte SF/FF/CF/FC, confirmations/timers | EA 0x56 with own state/configuration | Native request/response, FC and error tests execute; full SWS/ISO edge-case classification and timing unverified. |
| PduR | Initialized fixed diagnostic route and error/completion callbacks | Own initialized route | Native pipeline executes. Standard dynamic buffer/routing APIs and full SWS scope partial. |
| Com | Configured RX signals, two periodic TX PDUs, groups/deadlines/defaults/notifications | Configured RX signals, alive/mileage TX PDUs, own layout | Native generic core tests and M4 ECU-hook compilation. Full SWS update-bit/invalidation/filter/mode variants unverified. |
| CanNm | Configured one-byte ZGW NM3 follower | Own independent follower | Active/expiry states execute. ZGW alone originates NM3. Complete SWS applicability, wake and bench behavior open. |
| CanSM | Logical modes, application TX gate, bounded quiet period after bus-off | Same plus configured bxCAN automatic bus-off recovery | Mode/gate assertions execute; physical controller/transceiver transition recovery remains partial. |
| ComM | One channel/user plus remote NM arbitration | Own one-channel configuration | Host subset executes; multi-user/channel/inhibition/diagnostic standard APIs not fully implemented. |
| Dcm | Configured service/session/length checks; complete UDS payload buffers; asynchronous persistence/transitions; ECU services retained | Own config/hooks preserving camera/cluster/mileage services | Native DCM/DEM/NvM/transport execution. Full R4.3.1 DSL/DSD/DSP, OpStatus/cancellation/security and all service variants partial. Configured 19 04 record 1 returns metadata DID F1A0; unsupported records return NRC 0x31. |
| Dem | Configured events/DTCs, UDS state, counters/cycles, primary memory/displacement, records, callbacks, async durable clear | Own nine-event configuration and original monitors | Actual record serialization/restoration and success/failure clear execute; R20-11 trace still partial. Snapshot content is event metadata exposed by DID F1A0; application environmental capture remains unconfigured. |
| NvM | 3 configured blocks, queue, block/status/default/invalidate/ReadAll/WriteAll APIs | 2 configured blocks, own queue/configuration | Native real-descriptor migration and failure injection; complete SWS requirements and full APP link unverified. |
| MemIf | CRC/sequence/commit-last two-bank journal | Own geometry/backend | Executed torn append/rollover/corruption/unchanged/tombstone tests. Flash electrical behavior unverified; no whole-WriteAll atomicity claim. |
| Crc | Project-local reflected CRC32 continuation | Independent same implementation | Known-vector, journal and image-integrity tests execute. Full SWS CRC family/API applicability unverified. |
| STM32 flash | F411 journal sector bounds, HAL status/readback/lock | F407-specific journal geometry, HAL status/readback/lock | M4 compiled, emulated flash tests. Hardware power-loss/controller validation remains open. |
| EcuM/scheduler | Existing lifecycle with one diagnostic-task NvM/DEM owner and startup drain | NvM/diagnostics progress in startup/postrun; sleep gated on durable result/activity | Source/static scheduling evidence, EcuM M4 objects; complete RTOS APP build remains unavailable. |
| Standard types/synchronization | Independent Std_Types/Platform_Types, DEM/NvM critical sections | Independent equivalents | Cortex-M fixed types and host compilation. Complete concurrency, API/reentrancy and SWS type review open. |
| DET, MemMap, RTE | Explicitly excluded; no dependency introduced | Explicitly excluded | Their absence is an explicit project scope decision; it must not be represented as complete platform compliance. |

Detailed requirement IDs for newly implemented non-DEM behaviors have not been reviewed against the complete R4.3.1 SWS and are **unverified**. Existing DEM IDs are not automatically closed because a functional host test now exists. Finish the exact mandatory applicability/source/test trace before any compliance statement. [Official AUTOSAR Classic specifications](https://www.autosar.org/standards/classic-platform).
