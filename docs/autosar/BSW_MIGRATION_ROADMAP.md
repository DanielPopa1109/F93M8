# Current continuation — 2026-10-19

See [remaining-gaps closure](../modernization/F93_REMAINING_GAPS_CLOSURE.md), [current compliance matrix](COMPLIANCE_MATRIX.md), and [executed verification](../modernization/F93_VERIFICATION_RESULTS.md). Queued transactional persistence, durable DEM clear, full UDS DCM, configured COM and routed FBL programming are now implemented to the documented scope. Full SWS classification, APP builds, physical validation and resident-updater activation remain incomplete. The material below is historical where it conflicts.

---

# Requirements-first BSW migration order

Baseline for DEM: the user-supplied official AUTOSAR CP **R20-11** SWS. Baseline for the other modules remains the originally requested **R4.3.1** SWS until revisited. Include corresponding ECUC definitions, BSW general requirements, standard interfaces and module descriptions. Record every applicable mandatory SWS ID, configuration decision, source owner, executed test and deviation. ZGW provides an implementation example only. This roadmap does not certify any current module.

1. **Close DEM first.** Complete the supplied R20-11 DEM PDF and ECUC applicability review and [traceability](DEM_SWS_TRACEABILITY.md). Define CBM and DMU event/DTC tables, cycle/autostart, primary memory/displacement, records, status mask, DCM client IDs and optional features. Finish client select/request, clear, setting-control, DET and NvM lifecycle. Preserve and test each existing flash layout. Execute transition, debounce, aging, filter, displacement, snapshot, failure-injection and concurrency tests on a native host compiler, then target-build both apps.
2. **Foundation interfaces.** Provide `Platform_Types`, `Std_Types`, Compiler/MemMap, Det and SchM interfaces/configuration with explicit STM32 deviations. Clarify FreeRTOS and HAL integration boundaries; do not call them AUTOSAR OS/MCAL without evidence. Establish configuration variant and version checks.
3. **Memory stack.** Specify NvM block descriptors and job/status callbacks for each ECU; map to MemIf/Fee/Ea or a documented STM32 flash adaptation. Add integrity, retries, update policy, power-fail-safe migration and restore/clear result handling before DEM storage closure.
4. **Communication lower layers.** Requirement-map and validate Can then CanIf for CBM MCP2515 and DMU bxCAN separately. Follow with CanTp and PduR standard PDU/buffer contracts. Retain proven extended-address diagnostics and existing application CAN behavior while testing timing, FC, timeouts, bus-off and callbacks.
5. **DCM.** Configure DcmDemClientRef, protocol connections, sessions/security, 0x19 subfunctions, 0x14, 0x85, NRCs and asynchronous DEM/NvM result handling. Verify both directions through CAN, ISO-TP, PduR and DEM on host and bench before FCD claims.
6. **Com and network management.** Derive each ECU signal/PDU contract from current ZGW DBC and actual F93 code; implement Com after PduR. Configure CanNm, CanSM and ComM only after bus sleep/wakeup and controller behavior are agreed. Test body and cluster application signals independently.
7. **System integration and evidence.** Reconcile EcuM startup/shutdown and operation cycles, cross-module version/configuration checks, RTE use (if selected), watchdog/error paths, target ELF/map, host tests, bus capture and bench fault injection. Keep CBM, DMU and their loaders as independent builds.

The [compliance matrix](COMPLIANCE_MATRIX.md) records current gaps and owners. No flashing, deployment or vehicle integration is implied by this roadmap.
