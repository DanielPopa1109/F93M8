# Reusable STM32 updater implementation guide

## Architecture to reproduce

1. Reserve a protected boot region and define application origin/vector contract in separate linker scripts.
2. Provide explicit boot request/validity state with CRC/signature/version and recovery semantics; avoid volatile-only state where resets/power cycles matter.
3. Separate transport, UDS state machine, memory-range policy and MCU flash driver.
4. Validate every requested address, size, overflow, alignment, BSC, transfer count and session/security prerequisite.
5. Erase bounded regions, program aligned units, check each return/status, read back, verify whole image, then atomically mark it bootable.
6. For self-update, prove code/data/vectors/stack/compiler helpers execute from SRAM while target flash is busy; provide independent recovery if the sole FBL is interrupted.
7. Handover only after validating vector MSP, reset handler, image integrity and target range.

## ELC reusable pieces

Potential reference patterns: `FBL_ProcessUds` service dispatch, ISO-TP RX framing, CRC routine, page loop, `.ram_blob` linker LMA/VMA scheme, RAM vector/entry mechanism. These are examples, not safe drop-in components: source currently omits critical guards/checks and has distinct resident/RAM parser contracts.

## Interfaces to define

Document fixed target address, CAN ID/addressing, ISO-TP limits/STmin, supported sessions/security, routine identifiers/options, RequestDownload ALFI/endian, TransferData block size/BSC wrap, CRC algorithm/byte order, reset timing and post-reset readiness. Align FCD, gateway and ECU through traces and integration tests.
