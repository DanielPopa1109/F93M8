> Current 2026-10-19 implementation is documented in [remaining-gaps closure](../modernization/F93_REMAINING_GAPS_CLOSURE.md), [verification](../modernization/F93_VERIFICATION_RESULTS.md), [compliance scope](../autosar/COMPLIANCE_MATRIX.md), and [memory](../bootloader/MEMORY_AND_LINKER.md). Both APPs now own queued persistence/full-payload diagnostics/configured COM; both FBLs have routed programming and linked Cortex-M ELFs. Historical raw-NvM/legacy-FBL/no-executed-tests statements below are superseded. Full APP build and hardware proof remain unavailable; resident updater activation is disabled.

# Application

## Startup and scheduling

`ELC_APP/Src/main.c` calls HAL init, clock setup, GPIO, DMA, CRC, ADC, CAN, timer and FreeRTOS initialization; starts PWM and ADC DMA; checks application/FBL software-version state in the reserved SRAM area; then starts the scheduler. ADC completion computes calibrated voltage, current and temperature values and publishes them to SMon globals.

`ELC_APP/Src/freertos.c` creates CMSIS-RTOS v2 threads for BSW (`EcuM_main`, `Dem_main`), application (`SMon_main`), diagnostics (`Dcm_main`) and CPU-load reporting. A 5-ms timer callback advances these periodic functions. Timer 3 also supplies run-time stats. The tick hook restarts ADC DMA while software state allows it. Stack overflow and allocation failure hooks request a reset.

## Responsibilities

- `BSW/Sys/EcuM`: run/post-run state, wake pin, reset bookkeeping and standby.
- `BSW/Nvm`: block-based persistence, parameter flash decode and sector switching.
- `BSW/Diag/Dem`: DTC table/status and freeze-frame capture.
- `BSW/Diag/Dcm`: single/segmented ISO-TP and UDS service handling.
- `BSW/Com/CanH`: periodic CAN messages, XCP command handling and routing received CAN frames to DCM/XCP.
- `APPL/SMon`: power/load sensing, protection logic, battery-state calculations and control outputs.

These are custom modules, not evidence of full AUTOSAR conformance. Follow code paths rather than module names when migrating.
