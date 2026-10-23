# Migration requirements

## For ELC to work through current ZGW/FCD

- Mark/configure ELC as a real target only after bench validation; choose correct bus, routing extended address `0x58`, CAN TP channel and target logical address.
- Confirm ZGW route maps `0x58` to the physical ELC CAN segment and that CAN addressing mode and gateway PDU IDs match ECU TP. Current source's ELC FBL listens on `0x702/0x703`; inspect generated ZGW CanTp PDU/addressing definitions and prove the frame translation.
- Agree on one FBL service contract: FBL selection, entry routine, erase RID/option layout, download bounds, CRC algorithm/byte order, TransferExit semantics and reset response.
- Either adapt FCD's ELC-specific profile to the current ELC FBL or implement the current ZGW/FCD contract in ELC; do not silently change shared defaults or infer compatibility from generic UDS support.
- Validate app/FBL HEX segments against linker regions; checked-in FCD ELC examples use base `0x08000000`, unlike APP's `0x08003C00` origin.
- Resolve MCU density and image limit before generating a real package.

## For migration into another STM32 ECU

Separate gateway interface, ECU application semantics, MCU flash/boot constraints, portable UDS state machine, family-specific flash driver, security/integrity, and new recovery work. Preserve this separation in design and docs. Follow [reusable updater guide](../bootloader/REUSABLE_IMPLEMENTATION_GUIDE.md).
