> Current 2026-10-19 result: [closure and remaining integration gates](../modernization/F93_REMAINING_GAPS_CLOSURE.md) and [executed verification](../modernization/F93_VERIFICATION_RESULTS.md) supersede prior legacy-transport/raw-NvM findings below. ZGW/FCD were inspected read-only. F93 now implements their routed addressing and standard RequestDownload/BSC/CRC shape, but contiguous packaging, footer migration, complete APP builds, bench timing and safe resident activation remain gates. No live interoperability claim.

# Compatibility test plan

Before enabling FCD ELC flashing:

1. Confirm the actual ELC hardware, MCU density, bus wiring/bitrate and target build version.
2. Capture FCD → DoIP → ZGW route → CAN → ELC request/response IDs, addressing bytes, segmentation, flow control and timing.
3. Test read-only diagnostic session/DID path and ensure replies route to tester with expected extended address.
4. On isolated bench, validate entry/selection, erase bounds, download ALFI/address/size, BSC 1/wrap, CRC request encoding and reset/reconnect behavior against one agreed profile.
5. Exercise expected negative cases and verify flash errors are propagated.
6. Run controlled power interruption only with a recovery plan and sacrificial hardware; prove FBL and APP recovery separately.
7. Confirm FCD target descriptor is Real, selected, package address matches ELF/HEX, logs show accepted responses, and resulting image boots.

The checked-in settings mark ELC simulated/unselected, so no claim of current FCD compatibility is made. See ZGW compatibility docs and [FBL validation](../bootloader/VALIDATION_TESTS.md).
