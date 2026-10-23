> Current 2026-10-19: F93 now has independent ELC-style SRAM updaters with the current FCD 0200/0155/0001/download/BSC/CRC sequence, raw-image export and software verification. FCD must enable TransferExit CRC; default deployment is interlocked. See [complete protocol/evidence](../bootloader/RAM_UPDATER_VERIFICATION.md). APP/CubeIDE builds, board recovery and live interoperability remain gates. Older gaps below are historical. ZGW and FCD were read-only.

# Compatibility reference

Source-based baseline for future F93 integration. This is a gap analysis/navigation aid, not a test result. Read [ZGW/FCD gap analysis](ZGW_FCD_GAP_ANALYSIS.md) for service, transport and flash differences, then [migration checklist](MIGRATION_CHECKLIST.md) for future work. Verify current source in all repositories before implementation.

The [implementation report](../modernization/IMPLEMENTATION_REPORT.md) and [second static audit](../modernization/STATIC_AUDIT_REVIEW.md) record current application/loader changes and routed-diagnostic blockers. They supersede the baseline's claim that CBM and DMU have no implementation changes.

Reference documents live in `D:\GitHub_Repos\ZGW\docs\compatibility\`: `DIAGNOSTIC_CONTRACTS.md`, `FLASHING_CONTRACTS.md`, `NETWORK_CONTRACTS.md`, `ECU_REQUIREMENTS.md`, `CODING_CONTRACTS.md`, and `MIGRATION_CHECKLIST.md`. Relevant implementations include `ZGW/ZGW_APP/BSW/Com/{CanIf,CanTp}`, `APP/ParallelFlashSwc`, `BSW/Diag/Dcm`, and `Scripts/FCD/FCD.pyw` plus `fcd_settings.json`. Those remote documents describe the inspected checkout; revalidate them before any migration.
