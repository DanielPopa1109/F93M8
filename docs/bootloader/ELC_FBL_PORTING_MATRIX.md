# ELC architecture implementation in F93 - 2026-10-19

Scope: independent CBM_FBL/CBM_APPL and DMU_FBL/DMU_APPL only. ELC and ZGW/FCD were read-only; G26 was outside the active repository scope. Existing working-tree changes were retained. This report describes source and executed software verification, not a flashed ECU.

## Source reference and actual ELC execution model

The reference inspected was `D:/GitHub_Repos/ELC/ELC_FBL`: `Core/Src/main.c`, `ram_blob.c`, `can.c`, interrupt/MSP files, startup assembly, `.cproject`, `.project` and `STM32F103C8TX_FLASH.ld`. The current checked-in source, rather than the reference's older documentation, defines the baseline. No new ELC build or hardware test was performed.

ELC reset runs `SystemInit`, copies `.data`, clears `.bss`, invokes constructors and `main`. `HAL_Init`, HSE/PLL x9, GPIO, CAN and CRC are initialized before boot selection. POR clears retained NCR. A retained programming/reset-counter condition or invalid APP vectors selects PROGRAMMING; otherwise `FBL_JumpToAppl` transfers to APP `0x08003C00`. The APP CRC word is `0x0800F7FC`, parameter pages `0x08003800..0x08003BFF`, resident FBL `0x08000000..0x080037FF`.

Resident CAN RX callbacks collect normal-address ISO-TP on `0x702`; responses use `0x703`. `FBL_ProcessUds` owns session, erase, download, transfer, CRC and reset state. `31 01 00 07` sets `g_run_ram_updater`; the main loop calls `FBL_RunRamUpdater`. That function validates copy size/location, copies the linker-defined image, checks MSP/Thumb entry, stops SysTick and sets VTOR/MSP before entry. The linker puts vectors/text/data in `.ram_blob` with SRAM VMA and resident-Flash LMA. The reference's RAM stack is `0x20004F80`, and its copy validation leaves 1 KiB below that stack.

ELC `ram_reset` calls `ram_updater_start`; its own polling bxCAN, ISO-TP and UDS use SRAM code. FLASH page erase/halfword programming use STM32F1 registers directly. The updater receives an FBL replacement, checks ranges/BSC/CRC, erases via RID `0006`, verifies writes and resets through AIRCR. Reference CRC RID `0000` accepts LE CRC; TransferExit CRC is BE. Resident erase RIDs include APP `0004` and parameter `0005`; RAM activation is `0007`. These are reference behavior, not the current FCD contract.

ELC RAM state includes `block_start_addr`, `block_length`, `prog_addr`, `bytes_received`, pending word bytes, BSC and ISO-TP state. Explicit `.ram_blob_data` objects coexist with ordinary `.bss` state; its polling controller inherits initialization from the resident FBL. Its fault handler writes an NCR dump and halts. Its CAN mailbox wait has no bound. No ELC machine-code closure, authentication, physical TX-success guarantee, transport-timeout guarantee or interrupted-replacement recovery is inferred merely from these source attributes. The port preserves the execution separation and strengthens these particular boundaries.

## Function and ownership comparison

In this table, **C** is `CBM - Central Body Module/CBM_FBL`, **D** is `DMU - Digital Main Unit/DMU_FBL`. Every listed implementation exists separately below both roots. There are no cross-project firmware source includes or shared firmware objects. Verification details and exact artifacts are in [RAM verification](RAM_UPDATER_VERIFICATION.md).

| ELC function/module | CBM implementation | DMU implementation | Placement / interface adaptation | Evidence |
|---|---|---|---|---|
| `Reset_Handler`, constructor initialization | C startup + `Fbl_Runtime.c` | D startup + `Fbl_Runtime.c` | Correct Cortex-M4 part startup, ordinary data/BSS initialization; updater NOLOAD state is explicitly initialized after handover | Complete ARM FBL links; APP full link unavailable |
| `main`, `SystemClock_Config`, `MX_NVIC_Init` | C `Core/Src/main.c`, SPI1/MCP2515 | D `Core/Src/main.c`, CAN1 | Existing source clocks/pins retained; configured 100/168 MHz core; task loop every 5 ms | Official-header ARM objects/links; physical clock unverified |
| retained session/reset selection | C `main`, `Fbl_SetInitialSession` | D equivalents | Dedicated mailboxes `2001FFF0` / `1000FFF0`; POR/BOR does not trust old request | Source/ARM link; no physical reset performed |
| `FBL_AppVectorsValid`, Flash CRC | C `Fbl_ApplicationValid` | D equivalent | Existing APP committed footer, size complement, vector bounds and Flash CRC retained | Executed resident programming/boot host tests |
| resident `FBL_ProcessUds` | C `Fbl.c::process` | D own `Fbl.c::process` | Current FCD RID 0200/0155/0001/0002; `34 00 44`; BE32 CRC | Resident host suites + source contract inspection |
| resident CAN RX callback | C MCP2515 poll / local Can stack | D `HAL_CAN_RxFifo0MsgPendingCallback` / local Can stack | Standard ID 710, target EA 54 / 56, response EA F0, gateway FC EA 41 | Executed transport suites; target bus unverified |
| `IsoTp_OnCanRx`, `IsoTp_SendFlowControl`, resident response | C BSW CanTp/CanIf/PduR | D own BSW CanTp/CanIf/PduR | Bounded 256-byte full UDS, segmented TX, final physical confirmation | Executed resident transport tests |
| `FBL_NvM_EraseFlash_APPL`, `Nvm_GetPage` | C `Fbl_BoardErase`, sector 7 | D `Fbl_BoardErase`, sectors 5/6 | F4 sectors; APP erase excludes journals, calibration/coding and FBL | Host range/error tests; ARM build; electrical behavior unverified |
| `FBL_NvM_FlashWriteData`, `FBL_NvM_FlashReadData` | C board program/read hooks | D own hooks | Existing byte programming/readback; committed APP validity footer | Native resident suites; Cortex-M link |
| `FBL_NvM_EraseFlash_PARAM` | C transactional parameter MemIf/NvM path | Not applicable | Preserve CBM coding/calibration schema and journal; do not copy CBM parameter services to DMU | CBM native persistence/programming tests |
| `FBL_JumpToAppl` | C `Fbl_BoardJump` | D own `Fbl_BoardJump` | Validity check; NVIC/SysTick cleanup; VTOR/MSP/CONTROL tail branch; APP start 08060000 / 08020000 | Existing native validity tests and ARM source compilation; handover bench open |
| `FBL_DiagService_ER_HardReset` | C `Fbl_BoardReset` | D equivalent | Reset after physical final-response confirmation; retained request cleared | Executed native confirmation tests |
| `g_run_ram_updater`, `FBL_RunRamUpdater` | C `Fbl.c` deferred transition + `FblUpdater_Prepare/Activate` | D independently owned equivalents | RID 0155 sends confirmed NRC78; task-context copy/vector/precondition validation; final 0155 response comes from SRAM | Enabled and default ARM links; layout host cases; no execution on a board |
| `.ram_blob_vectors`, text/data, copy symbols | C FLASH linker / `Fbl_Ram.h` | D own FLASH linker / header | Low SRAM VMA, Flash LMA, 512-byte VTOR alignment; separate NOLOAD state and 4 KiB stack | ELF/relocation/disassembly/stack checker passes |
| `ram_reset`, `ram_updater_start` | C `ram_blob.c::Ram_Start` | D own `ram_blob.c::Ram_Start` | Own initialization, CAN, DWT polling time; interrupts masked in NVIC; no resident callbacks | Final ARM machine code inspected |
| `default_irq` | C `Fbl_RamPlatform.c::fault` | D equivalent | All 112 SRAM vectors populated; unexpected fault halts in SRAM, no automatic reset into damaged Flash | Vector and branch checks; fault injection on hardware open |
| `can_rx_poll` | C direct SPI1 + MCP2515 register polling | D direct CAN1 FIFO polling | PA5/6/7 AF5, PC4 CS and source CNF 00/90/82 / inherited PA11/12 AF9 bxCAN configuration | Cortex-M link/closure; hardware I/O not simulated by protocol tests |
| `can_tx_blocking` | C MCP2515 TXREQ/TX0IF/errors | D CAN1 RQCP/TXOK/ALST/TERR/bus-off | Bounded polling, explicit physical success before erase/reset; no HAL callbacks | ARM closure; native TX-failure protocol tests; physical ACK traces open |
| `IsoTp_OnCanRx_blob`, RAM FC/send | C `Ram_OnCan`, `reply` | D independently owned equivalents | EA SF/FF/CF reception; FC CTS BS=0/STmin=5; every response <=6 UDS bytes fits SF; EA41 FC requires no TX transition | Native segmented/sequence/timeout/wrong-address tests |
| RAM `FBL_ProcessUds` | C `ram_blob.c::process` | D equivalent | Independent session/TesterPresent/DIDs/routines/download/BSC/retry/exit/reset | Native complete and failure transactions |
| `flash_wait_ready`, unlock/lock | C `Fbl_RamFlash.c` | D own F4 implementation | F4 BSY bit16/errors/keys/CR lock, bounded wait, periodic confirmed 37 NRC78 | Official headers + ARM code closure; physical timing open |
| `flash_page_erase`, `flash_mass_erase_range` | C `Ram_Erase` sectors 0/1 | D `Ram_Erase` sectors 0/1 | F4 SER/SNB/STRT x8 PSIZE; erase scope exactly 32 KiB; no mass erase or option-byte write | Host authorized-scope simulation + source geometry; target erase unexecuted |
| `flash_program_hword`, `flash_program_word_strict` | C `Ram_Program` | D own implementation | F4 byte programming allows arbitrary final/block lengths; ART cache invalidate and readback; vectors programmed last | Host Flash model/errors + ARM closure |
| RAM `FBL_CalcCrc32Flash`, verification | C `crc32`, `vectors`, `commit` | D own functions | Full staged CRC before erase; ECU/MCU/layout descriptor and all vectors checked; full Flash compare/CRC before success | Both actual linked FBL binary replacement simulations pass |
| RAM ECUReset/AIRCR | C `Ram_Reset` | D equivalent | Explicit post-commit reset after physical 51 response; no reset after incomplete/error state | Host success/failure/reset tests; physical reset unexecuted |
| ELC DID F17E/F17F/F180 status/version branches | Resident F186; SRAM F100/F186 | Same independent session/block status | Current programming profile retained; legacy ELC DID numbers are not reused. Generic FCD F101/FCD1 status schemas remain unsupported unless configured; no invented software-version/MCU packet encoding | Source reviewed; programming sequence does not depend on these optional reads |
| `Error_Handler`, assertions | Resident handlers retained; separate SRAM halt | Same independently | No Flash error-handler call from RAM | Relocation and machine-code checks |

## APP and project integration

Each APP retains its own DCM/EcuM durable transition and mailbox protocol, linker vector origin and footer. DCM now accepts FCD `28 01 03` as well as existing communication type 01; both NM implementations are followers without configured periodic NM TX. The exact preamble is host-tested. CBM calibration/coding and DMU mileage/graphics behavior were not rewritten.

FBL managed Debug/Release include paths and Core source entries already discover local implementation files. `.cproject` now disables generated jump tables and builtin calls for predictable SRAM closure. Generated Debug source/dependency/object lists include all new translation units. The existing part-specific startup, `.project` and `.ioc` pins remain appropriate; they were inspected, with no unsupported MCU/pin changes. Each FBL compiles resident and updater together. No RTE/DET/MemMap or ELC/ZGW firmware dependency was added.

DMU's 16 KiB was insufficient for the combined linked FBL. It now occupies sectors 0/1, `08000000..08007FFF`; sector 1 was outside all existing APP and persistent reservations. APP start/metadata, journal sectors 4/7, legacy sectors 2/3 and CCM mailbox are unchanged. CBM retains its existing 32 KiB partition. Both receive a fixed image identity at `08000200`; text follows the 32-byte descriptor. Old raw FBLs lacking this descriptor are deliberately rejected by this updater and require a coordinated initial installation using an independently verified recovery/programming interface.

## Remaining acceptance limits

Implemented, Cortex-M compiled/linked, ELF-checked and host-tested are separate from hardware readiness. Default activation remains interlocked. `--activation` builds the real transfer path without a stub/error directive. Neither that build nor this report enables deployment. Full CubeIDE/GCC builds, actual CAN timing/ACK, voltage/watchdog/option-byte policy, SRAM execution on a board, recovery access and controlled interruption tests remain unverified. G26 requires a separate task in its own repository. See [RAM verification](RAM_UPDATER_VERIFICATION.md) for reproducible commands and recovery consequences.
