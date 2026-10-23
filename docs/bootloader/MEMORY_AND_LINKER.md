# Current F93 memory and linker contract

| Project | Linked FLASH | SRAM / persistent reservations |
|---|---|---|
| CBM FBL | 08000000, 32 KiB, sectors 0/1 | Updater low 60 KiB + dedicated 4 KiB stack; resident RAM 20010000, 63 KiB |
| CBM APP | 08060000, 127 KiB | Final 1 KiB at 0807FC00 is validity metadata; journal sectors 5/6; legacy calibration/coding sectors 2/3 and DEM sector 4 |
| DMU FBL | 08000000, enlarged from 16 to 32 KiB, sectors 0/1 | Updater in ordinary low SRAM, 4 KiB stack; resident SRAM 20010000, 63 KiB; CCM remains data-only |
| DMU APP | 08020000, 255 KiB | Final 1 KiB at 0805FC00 is validity metadata; journal sectors 4/7; legacy mileage/DEM sectors 2/3 |

F411RE/F407VE geometry: sectors 0-3 are 16 KiB, sector 4 is 64 KiB, sectors 5-7 are 128 KiB. APP erase remains CBM sector 7 / DMU sectors 5-6. NvM writes only its journal banks. The new independent SRAM updater can erase sectors 0/1 in a controlled enabled build after full image validation; the default activation interlock remains zero. DMU sector 1 was outside all existing APP/journal/legacy reservations, so enlargement requires no APP or persistent-region relocation.

Both FBL linker scripts retain the raw-image identity at 08000200..0800021F. The identity states format/version, target address, MCU, resident size, APP start/footer and vector count. Resident text follows it. Missing or wrong identity is rejected by the updater. A coordinated initial install is required for older raw FBL images; no such install occurred here.

`.ram_blob` has VMA 20000000 and resident-Flash LMA, with 512-byte vector alignment and vectors/text/initialized data. Its size-matched load/VMA symbols are validated before copying. NOLOAD `.ram_blob_bss` follows and holds the candidate/transport state. Dedicated `.ram_blob_stack` is 2000F000..2000FFFF; all state must end below it. Resident data/BSS/heap/stack live above 20010000, and initial resident MSP must equal 2001FC00. The updater never erases its own active SRAM code or accesses a resident Flash helper while programming.

Retained mailbox: CBM 2001FFF0, DMU 1000FFF0, 16 bytes including the fault counter. APP NOLOAD NCR ends before it. DMU CCM ordinary data remains 10000000..1000FBFF; APP NCR 1000FC00..1000FFEF. CBM APP NCR remains 2001FDA8..2001FFEF. FBL resident stack/RAM ends at 2001FC00, below CBM retained data.

Linker assertions, current ELF load/section/symbol inspection, input relocations and final SRAM machine code pass for default and controlled builds. See [exact verification](../modernization/F93_VERIFICATION_RESULTS.md) and [RAM evidence](RAM_UPDATER_VERIFICATION.md). Full APP ELF/heap/stack and separate CubeIDE/GCC evidence remain unavailable. RAM linker alternatives are not deployment images; the checked FLASH scripts are the validated configurations. Existing APPs without a committed validity footer are not auto-trusted.
