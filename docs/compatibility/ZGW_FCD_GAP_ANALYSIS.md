> Current 2026-10-19: F93 now has independent ELC-style SRAM updaters with the current FCD 0200/0155/0001/download/BSC/CRC sequence, raw-image export and software verification. FCD must enable TransferExit CRC; default deployment is interlocked. See [complete protocol/evidence](../bootloader/RAM_UPDATER_VERIFICATION.md). APP/CubeIDE builds, board recovery and live interoperability remain gates. Older gaps below are historical. ZGW and FCD were read-only.

# F93 vs ZGW/FCD compatibility gaps

## Baseline

ZGW/FCD reference evidence: ZGW `docs/compatibility/DIAGNOSTIC_CONTRACTS.md`, `FLASHING_CONTRACTS.md`, `NETWORK_CONTRACTS.md`, `ECU_REQUIREMENTS.md`; active ZGW paths `ZGW/ZGW_APP/BSW/Com/{CanIf,CanTp}`, `APP/ParallelFlashSwc`, `BSW/Diag/Dcm`; FCD `Scripts/FCD/FCD.pyw`, `fcd_settings.json`. These contracts distinguish local ZGW services from downstream route-prefixed UDS and target-specific flash profiles. FCD candidate route addresses: CBM `0x54`, DMU `0x56`; both marked simulated/unselected in checked-in settings. ZGW compatibility docs state forwarding strips the route byte before sending UDS downstream.

F93 evidence: FBL main files use classic CAN IDs `0x700/0x701` and single-frame payload parsing; CBM CAN is external MCP2515; DMU CAN1. Application BSW DCM exists in each, but this scan does not establish complete current service/address configuration. `F93M8_CAN_DEC.dbc` and TSMaster PDBC files are not sufficient proof of active ECU behavior.

## Gap register

| Area | Evidence-based status | Required resolution / owner |
|---|---|---|
| Route/address to physical bus IDs | FCD candidates configured; ZGW route must map to F93 request `0x700` / response `0x701` | Verify current `ParallelFlashSwc` target descriptor and `CanIf` RX/TX PDU mapping; ZGW/FCD/system |
| Diagnostic transport | F93 FBL consumes 8-byte single CAN frames and lacks observed ISO-TP segmentation; ZGW forwarding uses configured CanTp | Add/verify target ISO-TP behavior and flow control or constrain client. End-to-end bench capture required; target/ZGW |
| UDS session | F93 has hand-coded `10 02`; CBM coding/calibration states also; no clear complete NRC/session timing policy | Reconcile exact requests/responses, P2/P2*/S3, extended session and `3E`; target/FCD |
| Security | No active F93 FBL `27` handler evidenced; ZGW app has no configured `27`, ZGW FBL security is separate | Define downstream security requirement and FCD profile; never transplant ZGW FBL key scheme; target/system |
| Download/transfer | F93 `34/36/37` are hard-coded, 4-byte word writes, no robust size/range/BSC/CRC; FCD expects a negotiated UDS download profile and target-specific chunks | Specify address-length format, max block, sequence/retry, CRC/signature, NRC and exit response; target/FCD |
| Erase | F93 uses fixed sector RIDs/options; no full requested range planning. CBM has app/coding/calibration; DMU app only | Agree exact RID/options and region protection; map ZGW route sequence to these semantics; target |
| Coding/config | CBM FBL contains coding/calibration session/erase states; app has NVM and DCM/DEM. No verified common coding schema/read-write flow | Inspect CBM Dcm/Nvm plus FCD node descriptor; preserve ECU schema and readback; target/FCD |
| App handover/recovery | Weak erased-first-word check, direct VTOR/MSP branch, no validity/health confirmation. Single-slot erase may destroy app | Define validity, pending/confirmed state, reset, boot confirmation and outage recovery; target |
| FCD target selection | CBM/DMU candidate nodes are simulated/unselected in ZGW `Scripts/FCD/fcd_settings.json` | Verify actual FCD descriptor/image ranges and live gateway before enabling; FCD |

## Overall conclusion

**Not currently demonstrated compatible for routed flashing.** F93 code has partial UDS SID branches but its FBL transport and transfer implementation cannot support normal multi-frame image streaming, and profile/range/integrity/recovery requirements are unresolved. There may be useful application diagnostics or simulation definitions, but no evidence here makes F93's FBL behavior equivalent to ZGW's own FBL or to FCD's general downstream flow. Do not alter ZGW/FCD or F93 firmware until exact on-wire behavior and ECU-specific acceptance criteria are reconciled.
