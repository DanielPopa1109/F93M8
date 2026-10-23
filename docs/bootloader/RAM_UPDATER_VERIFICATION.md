# F93 RAM updater implementation and verification

Date: 2026-10-19. **Software implementation verified locally; hardware unverified; default deployment interlock retained.** This supersedes earlier staging-only F09 descriptions. Scope is F93 CBM and DMU; no reference-repository edit, hardware programming, reboot, deployment, commit or push occurred.

## Execution and memory ownership

Both FBLs own separate copies of `Fbl_Updater.c`, `ram_blob.c`, `Fbl_RamFlash.c`, `Fbl_RamPlatform.c`, `Fbl_Identity.c` and their headers. Resident programming remains in the existing independent FBL/CanTp stack. `FblUpdater_Prepare` validates Flash LMA and SRAM VMA size/location, 512-byte alignment, exact stack top, Thumb reset entry and every SRAM exception/IRQ vector. It copies/read-compares the blob before handover. Thread context, idle Flash, unprotected resident sectors, software-IWDG option and inactive WWDG are required. No option byte is modified. Software-started IWDG cannot be inferred reliably from these checks; the controlled bench precondition is that none is running, consistent with reviewed FBL source (both watchdog HAL modules disabled and no start call).

RID 0155 first prepares the blob and sends NRC78 through the resident transport. Only physical completion enables task-context activation. Activation revalidates, stops SysTick, clears all eight NVIC enable/pending banks and pending SysTick/PendSV, sets VTOR, uses barriers and sets MSP/CONTROL/BASEPRI/FAULTMASK in a final assembly branch. It never continues C stack use after changing MSP. SRAM `Ram_Start` initializes its own state, owns/reinitializes communication and sends final `71 01 01 55 00`. The next client request therefore cannot race the resident-to-RAM CAN transition. A second preflight/copy failure returns final NRC72 from the resident FBL.

| Area | CBM | DMU |
|---|---|---|
| Resident Flash allocation | 08000000..08007FFF, sectors 0/1 | Enlarged from 16 to 32 KiB; sectors 0/1 |
| Fixed resident identity | 08000200..0800021F | Same offset, different target/MCU/layout values |
| Updater vectors/code/data VMA | Starts 20000000, within 60 KiB UPDATER | Same, ordinary SRAM rather than CCM |
| Initialized blob LMA | Included in resident Flash image | Same |
| Updater state/candidate | `.ram_blob_bss`, NOLOAD, explicitly initialized | Same independent allocation |
| Updater stack | 2000F000..2000FFFF, 4096 bytes | Same |
| Resident runtime SRAM | 20010000..2001FBFF, 63 KiB | Same; ordinary CCM allocation remains separate |
| Dedicated retained mailbox | 2001FFF0..2001FFFF | 1000FFF0..1000FFFF |
| APP and footer | 08060000; footer 0807FC00 | 08020000; footer 0805FC00 |
| Journals / legacy persistent | Sectors 5/6; legacy 2/3/4 | Sectors 4/7; legacy 2/3 |

The linker retains `.ram_blob_vectors`, collects `.ram_blob_text*` and `.ram_blob_data*`, defines `__ram_blob_{load,vma}_{start,end}`, then reserves independent NOLOAD state/stack. Assertions enforce VTOR, blob/state/stack boundaries, descriptor presence and Flash capacity. SRAM placement does not depend on an APP image being present.

## Complete programming protocol

Read-only source anchors: ZGW `BSW/Com/CanIf/CanIf.c`, `CanTp/CanTp.c`, `BSW/Diag/Dcm/Dcm_Cfg.h`, `APP/ParallelFlashSwc/ParallelFlashSwc.c`; FCD `Scripts/FCD/FCD.pyw`, `_execute_target_flash_preamble`, `_select_fbl_software_block`, `_flash_payload`. Current gateway constants are ZGW RX EA F0 and tester/TX EA 41. CAN ID 710 and target EA54/EA56 are retained in APP, resident FBL and RAM updater.

1. FCD APP preamble: `10 03`, `28 01 03`, `85 02`, `10 02`. Existing durable APP transition writes its dedicated retained mailbox and resets only after the response is physically confirmed.
2. In resident FBL: `31 01 02 00 02`, then `31 01 01 55`. Default interlock responds NRC22. In a controlled enabled build, confirmed NRC78 precedes handover; SRAM emits final 71/0155.
3. SRAM updater: `31 01 00 01 [address BE32] [size BE32]` authorizes the exact resident range, base 08000000, <=32768 bytes. This deliberately **defers physical erase** until the complete candidate is received and validated.
4. `34 00 44 [base BE32] [size BE32]` returns `74 20 01 00`, max request length 256 including SID/BSC. `36 [BSC] [up to 254 data bytes]` handles counter wrap and exact duplicate retransmissions; changed duplicates return NRC73. All UDS requests use bounded EA SF/FF/CF reception. FC is EA F0 / CTS / BS0 / STmin5. Every updater response fits an EA single frame; inbound gateway FC EA41 has no outgoing segmented transfer to advance and is ignored. The resident FBL retains full segmented response support separately.
5. **FCD TransferExit CRC must be enabled**: `37 [CRC32 BE]`. The updater requires complete size, CRC, the exact resident-stack MSP (2001FC00), every configured vector, Thumb code entries inside the candidate, and the fixed descriptor (magic/version/EA/MCU/resident size/APP start/footer/vector count). Missing CRC or wrong ECU images fail before erase. CRC is reflected IEEE polynomial EDB88320, initial/final XOR FFFFFFFF. This is an integrity check, not cryptographic image authentication.
6. After a physically confirmed 37 NRC78, SRAM erases only sectors 0/1 and programs the candidate using F4 byte programming, body first and reset vectors last. Flash is locked on completion/error when the controller is no longer busy. Cache invalidation, direct readback comparison and complete Flash CRC precede `77`. Busy waits and chunk programming can issue further NRC78 at 500 ms intervals. A controller that remains busy after the timeout is not reset or read as a valid image.
7. Identical repeated CRC-bearing TransferExit is acknowledged without another erase. Optional RID0002 verifies the committed transaction's range/CRC. `11 01` or `11 03` requires successful commit and physical 51 response completion before SRAM AIRCR reset. Before that reset, the dedicated retained mailbox requests one programming-session entry into the replacement resident FBL, allowing FCD to continue directly with APP flashing. The resident startup consumes/clears the request. Errors never deliberately reset into an incomplete replacement.

ISO-TP incomplete reception expires at 1000 ms; 30 s of inactive UDS aborts authorization/download and returns to default session. Wrong IDs/EA, malformed lengths, CF sequence, address/size changes, incomplete transfer and programming failures are rejected. A failed physical operation keeps the CPU in SRAM; a new authorized transaction can retry while power remains available. There is no persistent progress/resume journal for resident replacement.

## Build and machine-code evidence

Commands (from repository root):

```powershell
python tools/fetch_validation_dependencies.py
python tools/check_cortex_sources.py
python tools/build_fbl_validation.py
python tools/build_fbl_validation.py --activation
python tools/run_host_tests.py
python tools/test_ram_verifier.py
python tools/inspect_memory.py
python tools/audit_modernization.py
git diff --check
```

Builds use the local pinned Zig 0.13.0 Clang/LLD driver, thumb-freestanding-eabi Cortex-M4 hard-float, actual part startup/board source and official HAL/CMSIS. The enabled build defines `FBL_UPDATER_ACTIVATION_ALLOWED=1`; default remains zero. Neither command accesses hardware. Compiler flags disable jump tables, builtin substitution and unwind code. Debug/Release managed project settings retain the same safety flags and local source integration.

`verify_ram_updater.py` runs during each validation build. It checks exact original input-object SRAM relocations, rejects normal Flash/libc/HAL data/code references, disassembles final SRAM functions, rejects direct escaping branches and unresolved indirect calls/jumps, bounds PC-relative literal pools, validates all vectors and SRAM/Flash partitions, and derives a conservative fixed-frame call-chain stack bound including 104 bytes for an exception frame. Linker-generated veneers are also inspected as SRAM functions. Dynamic Flash data accesses are intentionally permitted through the bounded programming/readback functions; this checker is not a general proof of all computed pointer values. Host bounds tests complement it.

This driver does not expose `--emit-relocs` or GNU `-Map`. Relocations are inspected in exact pre-link object inputs and branches/literals in the final ELF. `.map` is explicitly an **ELF-derived section and symbol map**, not a GNU linker map. For a separate CubeIDE/GCC build, rerun equivalent dependency checking on its own input objects/final ELF and review its own linker map; no GCC build is claimed here.

`test_ram_verifier.py` actually relinks both targets with an intentionally forbidden SRAM-to-`Fbl_BoardReset` Flash dependency. The checker rejects both object relocation and final branch/veneer evidence. These fixtures remain in ignored `tools/.build/*-ram-negative` and are never exported as replacement images.

Artifacts: `tools/.build/{cbm,dmu}-fbl-arm/` (default) and `*-fbl-arm-controlled/` (enabled). Each contains ELF, `.ram.disasm`, `.ram.json`, `.map`, raw `.bin` and `.manifest.json`. `package_fbl_image.py` includes **all Flash load segments, including updater LMA**, fills gaps with FF, checks the ECU identity and emits image CRC/SHA256. The manifest is engineering metadata, not an invented FCD .pfpkg format. Exact final sizes/hashes are recorded in [verification results](../modernization/F93_VERIFICATION_RESULTS.md).

## Executed host evidence and practical limits

Twelve native C suites pass: independent CBM/DMU persistence, resident programming, diagnostics, migration, transport and SRAM updater. SRAM tests send CAN SF/FF/CF, inspect generated FC and stream a full 32 KiB candidate as well as each target's **actual exported controlled FBL binary** into the actual updater protocol C. They check exact image/CRC/readback, vector-last ordering, neighboring Flash preservation, counter wrap/duplicates, invalid MSP/Thumb/vector/EA/MCU/layout, incomplete transfer, timeout/reset loss, erase/program/readback/mismatch errors and physical-pending/reset TX failure. Physical I/O and Flash registers are mocks; SRAM placement is validated by the ARM ELF, not the native executable.

No instruction-level MCU execution, electrical erase/program test, transceiver timing, board clock measurement, watchdog/voltage characterization, CubeIDE/GCC full project build or APP ELF link was performed. The default images are interlocked. Controlled images are software artifacts prepared for a future recovery-equipped bench session; they are not approved deployment images or hardware-verified bootloaders.

## Recovery and interrupted power

Before physical erase, interruption preserves the old resident image. Once any resident sector is erased, lost power/reset can leave no bootable resident FBL even though APP/data partitions remain intact. Programming vectors last reduces premature use during an uninterrupted session; it provides **no automatic power-loss recovery**. SRAM is lost and cannot resume after a reset. There is no alternate resident image or protected recovery loader.

Independent SWD recovery needs accessible PA13/SWDIO, PA14/SWCLK, GND/VTref and preferably NRST, a suitable probe, readable/unlocked device security policy and a known good complete image. None of those board-level access conditions has been proved here. System ROM recovery additionally needs physical boot-mode selection/reset access and a supported, actually routed serial interface. STM32F411 uses an external MCP2515 that ROM cannot operate; STM32F407 ROM CAN support refers to CAN2, whereas DMU uses CAN1. Do not assume vehicle CAN recovers either board. Exact silicon ROM revision, pins, clocks and option bytes must be checked against [ST AN2606](https://www.st.com/resource/en/application_note/an2606-introduction-to-system-memory-boot-mode-on-stm32-mcus-stmicroelectronics.pdf) before a bench recovery claim.

F4 register/sector and SRAM adaptation follows [ST RM0383 for F411](https://www.st.com/resource/en/reference_manual/dm00119316.pdf) and [ST RM0090 for F407](https://www.st.com/resource/en/reference_manual/dm00031020-stm32f405-and-stm32f407-advanced-armbased-32bit-microcontrollers-stmicroelectronics.pdf). F407 CCM is not used for executable updater instructions. No recovery wiring or option bytes were changed.
