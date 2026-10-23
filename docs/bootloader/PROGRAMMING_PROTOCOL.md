> Current 2026-10-19 implementation: [closure](../modernization/F93_REMAINING_GAPS_CLOSURE.md), [protocol](DIAGNOSTIC_PROTOCOL.md), [memory](MEMORY_AND_LINKER.md), [programming](FLASH_PROGRAMMING.md), [updater interlock](FBL_UPDATER.md), [executed validation](../modernization/F93_VERIFICATION_RESULTS.md). Both FBLs now have routed ISO-TP/programming and linked ARM ELFs. Older legacy-transport descriptions below are historical; hardware and updater activation remain unverified/blocked.

# Programming protocol

## Transport

FBL generated CAN config receives standard ID `0x702`, transmits `0x703`; classic CAN, ISO-TP SF/FF/CF RX and FC TX. Resident parser advertises FC BS=0, STmin=5 ms; RX buffer is 4095 bytes. RAM parser buffer is 576 bytes and FC uses BS=0/STmin=5. Neither parser checks CF sequence numbers despite maintaining an SN variable. TX responses are emitted as single frames only (at most 7 UDS bytes), so larger responses are truncated/unsupported.

## Services

| Service | Resident FBL | RAM updater |
|---|---|---|
| `10 02` | enter programming state | retain state=2 |
| `31 01 0004` | erase APP | not implemented |
| `31 01 0005` | erase parameter page | not implemented |
| `31 01 0006` | not implemented | erase FBL range |
| `31 01 0007` | start RAM updater | not implemented in RAM parser |
| `34 00 44 addr size` | stores fields, no range guard | requires 4-byte address, positive size within FBL |
| `36 BSC data` | word buffers/write | word buffers/write |
| `37` | positive response only | positive response only |
| `31 01 0000 CRC32` | flush + CRC compare; persist CRC word | flush + CRC compare |
| `11 01` | reset | reset |

CRC parameter bytes are little-endian in `31 01 0000`, despite FCD helper packing addresses/sizes in big-endian for `34`. Main parser expects CRC in last four bytes (and can accept longer payload); RAM parser requires exactly 8 bytes.

Current FCD sends `31 01 0155` for FBL entry and `31 01 0200 01` block selection. This differs from the source routines above. See [compatibility gap](../compatibility/ZGW_FCD_GAP_ANALYSIS.md).
