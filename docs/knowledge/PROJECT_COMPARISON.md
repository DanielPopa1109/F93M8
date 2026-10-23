# CBM vs DMU

| Topic | CBM | DMU |
|---|---|---|
| MCU | STM32F411RETx, Cortex-M4 | STM32F407VETx, Cortex-M4 |
| Application domain | Central body inputs/actuation | Digital cluster, reverse-camera display, data recording |
| Application CAN | MCP2515 via SPI (`BSW/Com/CanSpi`) | On-chip CAN1 (`BSW/Com/CanH`) |
| FBL CAN | MCP2515 via SPI, inline driver in `CBM_FBL/Core/Src/main.c` | On-chip CAN1, `DMU_FBL/Core/Src/can.c` |
| FBL CAN request/response IDs | Hard-coded `0x700` / `0x701` | CAN1 filter `0x702`, reply `request StdId + 1` (`0x703`) |
| Application image | Linker origin `0x08060000`, 128 KiB | `0x08020000`, 256 KiB |
| FBL linker capacity | `0x08000000`, 32 KiB | `0x08000000`, 16 KiB |
| Extra flash handling | Application sector 7; coding sector 3; calibration sector 2 | Application sectors 5–7 |
| FBL session states | Default, extended, programming, coding, calibration, reset, jump | Default, extended, programming, reset, jump |

CBM FBL additionally has SPI/UART and a custom CAN controller implementation; DMU FBL uses bxCAN, with generated filters, RX FIFO callbacks and interrupt handlers. The flashloader protocols share the broad `10/22/31/34/36/37/11` vocabulary but differ in response construction, states, erase scope and address constants. Do not merge them by textual similarity.

Both are incomplete as robust updater references: no SecurityAccess (`0x27`) or download CRC/integrity check is implemented in the active FBL handler paths; address bounds and transfer lifecycle are not rigorously enforced; app validity check only tests a first word against erased flash; handover does not fully reset peripheral/NVIC/SysTick state. CBM and DMU support only small CAN frames in their handlers and do not implement a complete ISO-TP stack.

Source anchors: both FBL `Core/Src/main.c`; FBL and APP `STM32..._FLASH.ld`; DMU FBL `Core/Src/can.c`; CBM application `BSW/Com/CanSpi`; DMU application `BSW/Com/CanH`.
