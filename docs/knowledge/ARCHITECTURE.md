# F93 architecture — current source view

F93 contains four separate STM32CubeIDE firmware projects: `CBM - Central Body Module/CBM_APPL`, `CBM_FBL`, `DMU - Digital Main Unit/DMU_APPL`, and `DMU_FBL`. CBM uses STM32F411RE with an MCP2515 SPI CAN controller; DMU uses STM32F407VE with bxCAN1. Their sources, configuration, linker maps, peripheral hooks and tests remain independent. ZGW is the gateway and architecture example, not a shared implementation linked into these images.

Each APP combines generated STM32/HAL and FreeRTOS setup with its own directly called BSW: Can/CanIf/CanTp/PduR, Com/CanNm/CanSM/ComM, Dcm/Dem, NvM/MemIf/Crc, EcuM and ECU-specific application monitors. There is no RTE dependency in the reviewed production BSW/FBL sources. Direct calls intentionally replace RTE glue for this laboratory design.

Each resident FBL is a separate boot image and owns its own CAN transport and UDS programming service. The linked APP origin is `0x08060000` for CBM and `0x08020000` for DMU. APP validity metadata occupies the final 1 KiB of each APP region. The independent SRAM updater is built into each FBL but default resident-FBL replacement activation is disabled until external recovery is demonstrated.

See [current four-project review](../modernization/CURRENT_STATE_REVIEW_2026-10-24.md), [memory/linker map](../bootloader/MEMORY_AND_LINKER.md), [communications](COMMUNICATIONS.md), and [diagnostics](DIAGNOSTICS.md). Native host tests and Cortex-M object/FBL link checks have run; complete APP links and hardware behavior remain unverified.
