# F93 executed verification snapshot - 2026-10-19

Current ELC SRAM-updater implementation: [comparison](../bootloader/ELC_FBL_PORTING_MATRIX.md), [full RAM execution/machine-code evidence](../bootloader/RAM_UPDATER_VERIFICATION.md), [remaining modernization gates](F93_REMAINING_GAPS_CLOSURE.md). Software results below do not establish hardware, deployment or full AUTOSAR compliance. This replaces the earlier staging-only/16-KiB-DMU snapshot.

## Native compiled and executed suites

Command: `python tools/run_host_tests.py`. Compiler: local pinned Zig 0.13.0 Clang, x86_64-windows-gnu, baseline CPU, C11 with Wall/Wextra/Werror. Physical I/O and Flash are mocks. All twelve suite executables built and returned exit 0.

| ECU | Persistence | Resident programming | Diagnostics | Migration | Routed FBL transport | Independent SRAM updater |
|---|---|---|---|---|---|---|
| CBM | PASS | PASS | PASS | PASS | PASS | PASS |
| DMU | PASS | PASS | PASS | PASS | PASS | PASS |

SRAM tests stream a full 32 KiB candidate and each actual exported controlled FBL binary through SF/FF/CF reception into the actual updater C. Assertions cover FC, BSC wrap/identical and changed duplicates, full CRC/identity/all-vector checks, low-SRAM/invalid MSP rejection, authorized ranges, neighboring Flash preservation, vector-last ordering, interruption/reset loss, CRC mismatch, Flash erase/program/readback failure, readback corruption, periodic NRC78, failed pending/reset TX and commit-dependent reset. Diagnostics suites also execute FCD's exact `28 01 03` and `85 02` preamble. Existing persistence, APP footer validity and CBM parameter-journal cases continue to pass. No instruction-level Cortex-M emulation or hardware I/O execution is claimed.

## Cortex-M source compilation and complete FBL links

`python tools/check_cortex_sources.py` compiled production sources against pinned official HAL/CMSIS: CBM APP 21/21, CBM FBL 23/23, DMU APP 22/22, DMU FBL 18/18, zero failures. Complete freestanding FBL builds compile the final resident and SRAM code together, actual MCU startup, local board hooks and enabled official HAL drivers.

Commands: `python tools/build_fbl_validation.py` and `python tools/build_fbl_validation.py --activation`. Both default and enabled controlled ARM images link and pass SRAM verification. Explicit ELF entry matches Reset_Handler. Default activation stays interlocked; the controlled definition compiles the real VTOR/MSP handover and independent replacement path without a stub or error directive. Neither command accesses a target.

| ECU / configuration | Flash load bytes / 32768 | SRAM blob bytes | SRAM candidate/state bytes | Reserved stack / conservative bound | Relocations / SRAM instructions |
|---|---|---|---|---|---|
| CBM default | 27088 / 32768 | 5056 | 33320 | 4096 / 480 | 181 / 1660 |
| CBM controlled | 27600 / 32768 | 5056 | 33320 | 4096 / 480 | 181 / 1660 |
| DMU default | 19304 / 32768 | 4440 | 33320 | 4096 / 472 | 169 / 1430 |
| DMU controlled | 20328 / 32768 | 4440 | 33320 | 4096 / 472 | 169 / 1430 |

DMU exceeded its former 16 KiB, so FBL FLASH now covers sectors 0/1 (32 KiB), without moving its APP, journals, legacy data or retained mailbox. Both linker scripts reserve low SRAM for updater code/data/state and a separate 4 KiB stack; resident runtime uses high SRAM. F407 updater code never executes from CCM.

Exact ELF SHA-256 (unstripped files include source/debug information; raw image hashes/CRC are in adjacent engineering manifests):

| ECU / configuration | ELF SHA-256 |
|---|---|
| CBM default | `c03d0df114642fcaec0dd14ffa27ccfe8e5929cce51f634dc9f1ec1f116d4619` |
| CBM controlled | `17a00331b081abce6a7e7289ffbb97850e96b16d6c71fb0c21af07bfdb48e59f` |
| DMU default | `45018fdcbbf5fd2a5a01b317592eff821a72563487d4d1b2fc13f5b1202e7a72` |
| DMU controlled | `99e11f5af259446e51e6fb910c765b7c6cffaab19b373280ff44c9c1cf3675e3` |

`verify_ram_updater.py` runs on every validation build: exact pre-link object relocations, final function disassembly, branches/veneers/literals, all 112 SRAM vectors, LMA/VMA/stack partitions and conservative fixed-frame call-chain bounds. No required helper/code/constant relocation escapes into resident Flash. Dynamic programming/readback pointers are separately range checked and simulated; this is not a general theorem about all dynamic memory accesses.

`python tools/test_ram_verifier.py` passes for both targets by actually linking deliberate SRAM-to-resident-Flash reset calls and confirming rejection of relocations and final branches/indirect veneers. Negative fixtures are never packaged.

The pinned Zig frontend rejects emit-relocs and GNU Map options. Relocation evidence therefore uses exact input objects; final machine code uses the actual ELF. `.map` artifacts are explicitly ELF-derived section/symbol maps, not GNU linker maps. `package_fbl_image.py` exports all FLASH load segments, including the updater LMA, and checks the target descriptor before producing raw `.bin` plus CRC/SHA256 engineering metadata. Both actual controlled raw binaries passed native updater replacement. No FCD .pfpkg format was fabricated.

## Memory and source/build integration

`python tools/inspect_memory.py` passes both actual FBL Flash bounds, all four FLASH-script MEMORY-region overlap checks, APP validity-footer reservations and retained mailboxes. `python tools/audit_modernization.py` passes independent project boundaries, Eclipse XML and source/rule/object chains: CBM APP 22 entries, CBM FBL 15, DMU APP 21, DMU FBL 12. These counts cover the selected modernization source set, not all project files. New FBL sources are discovered by both managed Debug/Release Core entries and generated Debug lists. `git diff --check` passes.

Artifacts remain ignored under `tools/.build/{cbm,dmu}-fbl-arm{,-controlled}/`; dependencies are local ignored `tools/.runtime/` files. Two pinned ELF/disassembly libraries were installed there; no system SDK or compiler installation changed. Existing dirty files were preserved.

## Remaining build and physical gates

Current Get-Command checks found no arm-none-eabi-gcc, make or STM32CubeIDE; the configured STM32Cube_FW_F4_V1.28.1 tree is absent. Earlier APP make attempts therefore stopped before compilation. The APP FreeRTOS/graphics middleware and complete APP ELF/linker maps remain unavailable. These Clang/LLD Cortex-M FBL links do not substitute for separate CubeIDE/GCC builds of all four complete projects.

No voltage, watchdog/option-byte, oscillator, MCP2515/bxCAN ACK/timing, SRAM execution on the physical MCU, real handover or interruption/recovery test occurred. Default activation remains interlocked until independently usable board recovery is proved. Full replacement has no automatic power-loss recovery: interrupted resident erase/program can leave a nonbootable ECU requiring external SWD or a physically accessible supported system-ROM path.

No software was deployed. No ECU or CM5 was modified, flashed, reset or rebooted. ELC, ZGW/FCD and G26 were not edited. No commit or push occurred. See [source manifest](F93_SOURCE_MANIFEST.md) for changed-file groups and [RAM verification](../bootloader/RAM_UPDATER_VERIFICATION.md) for protocol, primary hardware references and recovery details.
