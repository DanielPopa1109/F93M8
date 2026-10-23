> Current 2026-10-19 implementation: [closure](../modernization/F93_REMAINING_GAPS_CLOSURE.md), [protocol](DIAGNOSTIC_PROTOCOL.md), [memory](MEMORY_AND_LINKER.md), [programming](FLASH_PROGRAMMING.md), [updater interlock](FBL_UPDATER.md), [executed validation](../modernization/F93_VERIFICATION_RESULTS.md). Both FBLs now have routed ISO-TP/programming and linked ARM ELFs. Older legacy-transport descriptions below are historical; hardware and updater activation remain unverified/blocked.

# Validation plan

No validation run is claimed by this knowledge bank. Suggested staged tests on controlled bench hardware:

1. **Artifact/map**: build both configurations; inspect ELF/map/HEX address ranges, RAM blob load/VMA/size, stack, vectors, application CRC reservation and exact MCU density.
2. **Protocol**: replay valid/malformed ISO-TP SF/FF/CF/FC; wrong CF SN; oversized lengths; BSC duplicate/gap/wrap; wrong ALFI; boundary/overflow addresses; mismatched and truncated lengths; NRC expectations.
3. **Flash**: verify every erase/program return and readback; test alignment, erased-state checks, CRC known vectors and image boundary, while recording CAN response latency.
4. **Handover**: APP good/bad vectors, CRC mismatch, retained state, POR/software reset, stale peripherals and RAM blob fault path.
5. **Recovery**: controlled interruption at each erase/write/verify boundary using sacrificial boards; verify APP failure leaves FBL serviceable and define FBL self-update recovery.
6. **Integration**: ZGW routed diagnostic path and FCD real-target descriptor, compare CAN trace to exact service/RID/address/CRC contract; confirm timeouts and response routing.

Do not use real vehicle/production hardware for interruption testing. Tests require explicit separate authorization for flashing or destructive fault injection.
