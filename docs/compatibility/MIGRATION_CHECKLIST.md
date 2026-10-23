> Current 2026-10-19 result: [closure and remaining integration gates](../modernization/F93_REMAINING_GAPS_CLOSURE.md) and [executed verification](../modernization/F93_VERIFICATION_RESULTS.md) supersede prior legacy-transport/raw-NvM findings below. ZGW/FCD were inspected read-only. F93 now implements their routed addressing and standard RequestDownload/BSC/CRC shape, but contiguous packaging, footer migration, complete APP builds, bench timing and safe resident activation remain gates. No live interoperability claim.

# F93 migration checklist

1. Read this index and the relevant ZGW contract pages; recheck current ZGW/FCD code.
2. Inspect CBM and DMU independently. Confirm MCU order code, board pins/transceiver, application services and FBL source before each change.
3. Reconcile FCD route byte (`0x54` CBM, `0x56` DMU) with ZGW current route/CanIf mapping and physical `0x700/0x701` diagnostic CAN IDs.
4. Capture request/response frames through direct tester, ZGW forwarding and FCD; confirm ISO-TP single/multi-frame, flow control, addressing, timeout and response correlation.
5. Produce per-ECU UDS matrix: session, security, DIDs, RIDs/options, NRC, TesterPresent, P2/P2*/S3, reset and preconditions.
6. Specify flash image address/length, sector erase, address-size encoding, maximum block, BSC/retry, transfer exit, CRC/signature, app validity, confirmation and interrupted-update recovery.
7. Reconcile all linker scripts, sector geometry, application vector, FBL reserved areas, build outputs and image packaging; resolve CBM `0x0800C000` vs `0x08060000` conflict.
8. Preserve ECU-specific CBM actuation and DMU cluster/camera functions. Treat DBC/PDBC as configuration evidence, not proof of active emissions.
9. Assign every gap to target, ZGW, FCD or system integration; do not hide gateway/tool defects with ECU changes.
10. Build each project, run available software/protocol tests, then bench-test transport, flash boundaries, interruption, reset and app health before claiming compatibility. Update affected knowledge pages after implementation.
