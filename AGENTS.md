# F93 repository guidance

- Start at `docs/knowledge/INDEX.md` and load only task-relevant pages; verify important details against current source.
- Keep CBM and DMU separate. Preserve ECU-specific behavior and distinguish configured/source behavior from build, bench and vehicle evidence.
- For ZGW/FCD integration, read `docs/compatibility/INDEX.md`, then inspect current ZGW, FCD and target source/configuration; assign each gap to its owner.
- For bootloader work, start at `docs/bootloader/INDEX.md`. Re-derive the target MCU, memory geometry, linker layout and board transport before reusing any concept or code.
- Update affected knowledge pages and their indexes when implementation or integration contracts change. Avoid redundant repository-wide scans.
- Do not edit firmware, linker/build configuration or tools when a task is limited to documentation. Never claim source inspection proves hardware behavior.
- For BSW modernization, use the applicable official AUTOSAR Classic Platform SWS and ECUC definitions as the requirement baseline. DEM currently uses the user-supplied R20-11 SWS; other modules retain R4.3.1 until explicitly changed. ZGW is an implementation reference and cannot override the SWS.
- Record each verified requirement ID, applicability decision, source implementation, and executed test in `docs/autosar/DEM_SWS_TRACEABILITY.md` or the relevant module trace. Mark inaccessible or unreviewed requirements unverified; never infer compliance from module names or APIs.
- Keep CBM and DMU implementation files, configurations, and build inputs independent. Preserve ECU-specific fault monitors and persistent records while moving diagnostic state management into DEM.
- Distinguish source review, cross compilation, target builds, executed host tests, and bench evidence. Do not label a BSW module compliant until all applicable mandatory requirements have implementation and verification evidence.

- Current modernization continuation: start at `docs/modernization/F93_REMAINING_GAPS_CLOSURE.md` and `F93_VERIFICATION_RESULTS.md`. Preserve journal-bank reservations, APP validity footers and dedicated retained boot mailboxes. Do not restore legacy FBL CAN IDs or bypass physical TX confirmation for erase/reset/durable service completion.
- Native suites run with `python tools/run_host_tests.py`; official-header Cortex-M object validation and complete freestanding FBL links have separate scripts under `tools/`. These do not replace CubeIDE/GCC application builds or bench tests. Keep `tools/.runtime` and `tools/.build` ignored.
- Resident-FBL activation must stay disabled until independently usable board recovery is proven; changing a macro is insufficient. Original F13/F14 audit text is not available in the repository and must not be invented.
