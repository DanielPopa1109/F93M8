# F93 tools and tests

Current executed results: [verification snapshot](../modernization/F93_VERIFICATION_RESULTS.md). Architecture comparison: [ELC porting matrix](../bootloader/ELC_FBL_PORTING_MATRIX.md). Commands are run from repository root.

| Tool | Purpose / current evidence |
|---|---|
| `tools/run_host_tests.py` | Twelve independent native C suites pass; physical I/O/Flash mocked; includes real linked replacement binaries |
| `tools/check_cortex_sources.py` | Official-header Cortex-M4 production object checks; distinct from full APP link |
| `tools/build_fbl_validation.py` | Complete default FBL ARM links, SRAM verification and raw-image export |
| Same with `--activation` | Compiles the real controlled handover path; never connects to hardware |
| `tools/verify_ram_updater.py` | Input relocations, final SRAM instructions/vectors/literals/branches, memory and conservative stack bound |
| `tools/test_ram_verifier.py` | Actually links forbidden SRAM-to-Flash dependency fixtures and confirms rejection for both targets |
| `tools/package_fbl_image.py` | Raw image includes all Flash load segments, updater LMA and ECU identity; CRC/SHA256 metadata |
| `tools/inspect_memory.py` | Actual FBL ELF load bounds and all four FLASH linker reservations |
| `tools/audit_modernization.py` | Eclipse XML, local source/rule/object chains and independent firmware boundaries |
| `tools/fetch_validation_dependencies.py` | Pinned official HAL/CMSIS and local ELF/disassembly analysis libraries |

Dependencies and artifacts remain ignored under `tools/.runtime` and `tools/.build`. Exported manifests are engineering metadata, not FCD .pfpkg files. No native test result demonstrates physical CAN timing, SRAM execution, electrical Flash programming, recovery access or vehicle behavior. Full CubeIDE/GCC application builds remain unavailable. No hardware operation or deployment was performed.
