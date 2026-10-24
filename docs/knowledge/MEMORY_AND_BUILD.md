# F93 memory and build

Four independent projects: CBM APP/FBL (STM32F411RET6, MCP2515) and DMU APP/FBL (STM32F407VET6, bxCAN). Current source, ELF and executed software evidence are in [memory/layout](../bootloader/MEMORY_AND_LINKER.md), [ELC porting matrix](../bootloader/ELC_FBL_PORTING_MATRIX.md) and [verification results](../modernization/F93_VERIFICATION_RESULTS.md).

| Image | Flash | SRAM / persistent reservation |
|---|---|---|
| CBM FBL | 08000000, 32 KiB | Updater low 60 KiB + 4 KiB stack; resident high 63 KiB |
| CBM APP | 08060000, 127 KiB | Footer at 0807FC00; journal sectors 5/6; legacy data 2/3/4 |
| DMU FBL | 08000000, enlarged to 32 KiB | Updater executable ordinary SRAM; resident high 63 KiB; CCM data-only |
| DMU APP | 08020000, 255 KiB | Footer at 0805FC00; journal sectors 4/7; legacy data 2/3 |

DMU FBL enlargement consumes sector 1, which was outside existing APP and persistent reservations; no APP or data region moved. Both FBLs use a fixed raw-image identity at 08000200, SRAM VMA/Flash LMA updater code, NOLOAD candidate/state and a separate 4 KiB updater stack. Linker assertions plus actual default/controlled ARM ELF checks verify these regions. APP metadata and the dedicated retained mailboxes (CBM 2001FFF0, DMU 1000FFF0) remain protected from overlap.

APP persistence is queued NvM/MemIf/Crc with independently configured two-bank journals and legacy migration. MemIf owns the journal directly over each STM32 Fls backend; there is no Fee module. `NvM_IsIdle` now requires completed initialization, so a missing or invalid NvM configuration cannot appear ready to the sleep gate. Native suites exercise integrity/interruption behavior and this pre-init guard. This does not prove physical power-loss behavior. Full APP ELF/heap/stack/middleware builds remain unavailable; Clang/LLD FBL builds and official-header object checks are distinct from CubeIDE/GCC full-project builds. The configured Cube SDK and ARM GNU toolchain are absent on this host.

See [SRAM verification/recovery](../bootloader/RAM_UPDATER_VERIFICATION.md) for programming, raw-image export, diagnostic contracts and recovery gates. Default activation is interlocked; enabled software builds contain the real handover. No ECU/CM5 was modified, flashed, reset or rebooted.
