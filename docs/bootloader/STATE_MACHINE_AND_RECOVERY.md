> Current 2026-10-19 ELC-port implementation: [porting matrix](ELC_FBL_PORTING_MATRIX.md), [RAM execution and verification](RAM_UPDATER_VERIFICATION.md), [memory](MEMORY_AND_LINKER.md) and [executed results](../modernization/F93_VERIFICATION_RESULTS.md). Both FBLs have real independently executing SRAM updaters and verified default/controlled ARM ELFs. Default deployment is interlocked; full CubeIDE/APP builds and all hardware/recovery evidence remain unverified. Older staging-only/legacy statements below are historical.

# State machine and recovery

Current FBL lifecycle is routed diagnostic wait -> session/range validation -> physically confirmed pending -> checked APP erase -> bounded download/BSC transfer -> confirmed pending -> chunked CRC readback -> commit-last image metadata -> final physical confirmation -> reset/validated boot. Error paths return negative responses and retain the routed loader. A timed-out/incomplete transfer never creates valid metadata.

NvM has separate append/commit-last bank transactions and keeps the previous committed bank through rollover; simulated cut points pass. An APP image replacement does not have A/B rollback: an interrupted erase/download leaves the resident loader available, not a guaranteed previous bootable application. Resident-FBL replacement remains disabled because independent recovery is unproven.

See [closure](../modernization/F93_REMAINING_GAPS_CLOSURE.md), [programming](FLASH_PROGRAMMING.md), [updater](FBL_UPDATER.md), and [validation evidence](../modernization/F93_VERIFICATION_RESULTS.md). No hardware recovery is claimed.
