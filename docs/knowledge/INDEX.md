> Current 2026-10-24 four-project assessment and fixes: [CBM/DMU APP/FBL review](../modernization/CURRENT_STATE_REVIEW_2026-10-24.md). Independent resident/SRAM updaters are ARM-linked and host-tested; default activation is interlocked. Complete APP/CubeIDE builds, bench operation and recovery remain unverified. See [RAM evidence](../bootloader/RAM_UPDATER_VERIFICATION.md), [current memory](../bootloader/MEMORY_AND_LINKER.md) and [verification results](../modernization/F93_VERIFICATION_RESULTS.md). Older contradictory material below is historical.

# F93 knowledge bank

Start here. This repository contains independent CBM (STM32F411RETx) and DMU (STM32F407VETx) application/FBL projects, plus bus test artifacts. See the [modernization implementation report](../modernization/IMPLEMENTATION_REPORT.md) and [second static audit](../modernization/STATIC_AUDIT_REVIEW.md) for current source changes, findings F01–F12 and remaining ZGW architecture gaps.

The current [AUTOSAR compliance matrix](../autosar/COMPLIANCE_MATRIX.md), [DEM trace](../autosar/DEM_SWS_TRACEABILITY.md), [selected DCM trace](../autosar/DCM_SWS_TRACEABILITY.md), [DEM implementation report](../autosar/DEM_IMPLEMENTATION_REPORT.md), and [BSW roadmap](../autosar/BSW_MIGRATION_ROADMAP.md) supersede any earlier suggestion that a ZGW-shaped module is AUTOSAR compliant. DEM uses the user-supplied R20-11 SWS; the other modules retain the R4.3.1 baseline pending their own migrations.

## Topics

- [Architecture](ARCHITECTURE.md) — project boundaries, runtime and dependencies.
- [STM32 platform](STM32_PLATFORM.md) — MCU, clocks, pins, interrupt and watchdog evidence.
- [Application](APPLICATION.md) — tasks and ECU modules.
- [Communications](COMMUNICATIONS.md) — CAN, signals and transport.
- [Diagnostics](DIAGNOSTICS.md) — DCM/UDS behavior.
- [Memory and build](MEMORY_AND_BUILD.md) — linker map, persistent state, builds.
- [Tools and tests](TOOLS_AND_TESTS.md) — engineering tools and evidence limitations.
- [FBL knowledge bank](../bootloader/INDEX.md) — updater source-level trace.
- [ZGW/FCD compatibility](../compatibility/INDEX.md) — current integration gap analysis.

Claims are derived from tracked source/configuration. Binary outputs, historical tests and board files do not independently prove the behavior of a currently connected ECU. See each page for unresolved items.
