# Current application programming

Both FBLs accept the FCD standard 34 00 44 address/size format and advertise 256-byte UDS blocks. APP images must be contiguous, start at the linked APP vector and end before the reserved footer. Sparse/multiple independent segments are not yet accepted as one recoverable transaction. No other ECU's memory/controller code is reused.

Erase emits NRC 78, waits for actual CAN completion, then checks HAL unlock/erase/lock and erased-word readback before a final positive response. TransferData checks BSC, exact duplicate data, total length and protected address bounds; HAL programming and readback errors return NRC 72. TransferExit performs bounded asynchronous CRC readback, checks vector bounds, writes size/CRC/complement and commits the validity word last. Incomplete/invalid images do not boot. Reset follows physical positive-response completion.

CBM coding/calibration collect the exact original 24/40-byte blocks and update the same journal schema used by its APP. Their preparation erase does not destroy the old raw sectors. Completion is positive only after the journal commits. Parameter CRC reads committed MemIf data. DMU's resident APP programming remains specific to its own APP layout; its new independent SRAM updater owns resident replacement separately.

No physical flash was executed. Full hardware erase timing, watchdog/interrupt behavior, retention, power-loss and ZGW/FCD bus validation remain required. See [closure](../modernization/F93_REMAINING_GAPS_CLOSURE.md) and [tests](../modernization/F93_VERIFICATION_RESULTS.md).


## Resident replacement continuation

Both independent SRAM updaters now execute real sector-0/1 erase, byte program, cache invalidation, direct readback and CRC, after complete SRAM candidate validation and physical NRC78. This is distinct from the APP erase/write/footer path above. See [memory](MEMORY_AND_LINKER.md), [ELC comparison](ELC_FBL_PORTING_MATRIX.md) and [executed verification/recovery limits](RAM_UPDATER_VERIFICATION.md). Default activation remains interlocked; no electrical programming was performed.
