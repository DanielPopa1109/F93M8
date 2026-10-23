> Current 2026-10-19 ELC-port implementation: [porting matrix](ELC_FBL_PORTING_MATRIX.md), [RAM execution and verification](RAM_UPDATER_VERIFICATION.md), [memory](MEMORY_AND_LINKER.md) and [executed results](../modernization/F93_VERIFICATION_RESULTS.md). Both FBLs have real independently executing SRAM updaters and verified default/controlled ARM ELFs. Default deployment is interlocked; full CubeIDE/APP builds and all hardware/recovery evidence remain unverified. Older staging-only/legacy statements below are historical.

# Bootloader validation plan

## Build and static artifact checks

- Clean-build bootloader and application separately with recorded toolchain, HAL, defines and linker scripts.
- Inspect ELF/map/HEX: entry point, vector table, MSP/reset handler, flash bounds, section overflow, overlap and reserved metadata.
- Static review range math, integer overflow, sector planner, exact byte accounting, flash unlock/lock on every path, error propagation and watchdog behavior.
- Host-test transport parsing, UDS state transitions, BSC wrap/duplicate/out-of-order behavior, timeouts, erase planning and image verification using fake flash.

## Bench protocol checks

Capture raw bus/DoIP and test each expected SID/session/RID, positive and negative response, lengths, IDs, extended/standard addressing, flow control, block pacing, max block, timeout, tester-present and reset. Verify both direct target and ZGW-routed response correlation. Compare requests produced by actual FCD code with ECU parser, including payload cap.

## Flash and recovery checks

On disposable target hardware: erase boundary, alignment, all supported sizes, readback, CRC/signature mismatch, invalid range, wrong sequence, block retry, bus disconnect, voltage interruption at erase/program/metadata/commit/reset, watchdog expiry, invalid vectors, app startup timeout and repeated update cycles. Prove that loader and metadata remain recoverable. Record actual firmware hash and hardware revision.

## Acceptance reporting

Separate source/build/unit/simulator/bench/vehicle evidence. A build does not prove flash correctness; CRC success does not prove application boot; a successful reset does not prove health confirmation or rollback. Record untested failure cases as unresolved; do not infer them from state names or simulator results.
