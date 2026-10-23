> Current 2026-10-19 implementation is documented in [remaining-gaps closure](../modernization/F93_REMAINING_GAPS_CLOSURE.md), [verification](../modernization/F93_VERIFICATION_RESULTS.md), [compliance scope](../autosar/COMPLIANCE_MATRIX.md), and [memory](../bootloader/MEMORY_AND_LINKER.md). Both APPs now own queued persistence/full-payload diagnostics/configured COM; both FBLs have routed programming and linked Cortex-M ELFs. Historical raw-NvM/legacy-FBL/no-executed-tests statements below are superseded. Full APP build and hardware proof remain unavailable; resident updater activation is disabled.

# Architecture

## Projects

- `ELC_APP/` is the STM32CubeIDE application. Its own linker script starts flash at `0x08003C00`; it is not linked as a stand-alone image at address zero.
- `ELC_FBL/` is a separate STM32CubeIDE FBL. `Core/Src/main.c` contains the flash-resident UDS receiver and application writer. `Core/Src/ram_blob.c` is a distinct RAM-linked self-programmer for the FBL area.
- `Tool/A2L/A2L_ELC.a2l`, `Tool/Documentation/` and `Tool/Documentation/HW/Rev A/` contain calibration, test spreadsheets, and KiCad design outputs.
- `ZGW_CAN_3/` is a separate TSMaster/test artifact tree with DBC/PDBC, diagnostic scripts and captures. Treat these as test/configuration evidence, not proof of the firmware's live bus contract.

## Application layering

The application combines STM32 HAL/generated peripheral setup with custom BSW modules: `EcuM`, `Nvm`, `Dem`, `Dcm`, and `CanH`; `APPL/SMon` provides load and battery monitoring/control. FreeRTOS CMSIS-RTOS v2 schedules BSW, application and diagnostic functions. Details are in [APPLICATION](APPLICATION.md).

## Update boundary

The APP does not contain the FBL protocol. The FBL boots, chooses programming mode or jumps to APP, then accepts UDS on CAN. Application programming runs through HAL flash operations from the FBL image. FBL self-update instead disables normal IRQ use, relocates a linked updater image to SRAM and executes it there; see [FBL architecture](../bootloader/FBL_ARCHITECTURE.md).

## Related projects

ZGW is an AURIX gateway with routed diagnostics and target-specific transport. F93 is a separate STM32 reference with its own simpler bootloader; neither is interchangeable with ELC. Compatibility guidance: [ZGW/FCD gaps](../compatibility/ZGW_FCD_GAP_ANALYSIS.md).
