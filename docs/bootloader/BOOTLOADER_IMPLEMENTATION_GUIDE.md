# New STM32 bootloader implementation guide

Use this as a requirements sequence, not a code template. First inspect the target repository and verify every decision against its MCU datasheet/reference manual, board, linker, diagnostic client and current ZGW/FCD path.

## Required interfaces/components

1. **Platform layer:** reset-cause, clock, watchdog, GPIO/transceiver and deterministic timebase.
2. **Transport:** CAN/ISO-TP, CAN FD/ISO-TP, Ethernet/DoIP or other agreed link; RX/TX buffers, flow control, queueing, P2/P2*/S3 timeouts, padding and addressing.
3. **UDS server:** session/security policy, NRC mapper, `10`, `11`, `27` if required, `31`, `34`, `36`, `37`, `3E`, and required DIDs/RIDs. Match FCD/ZGW target profile exactly; do not enable services by habit.
4. **Flash adapter:** address-to-region validation, erase planner, unlock/lock, program alignment/width, busy/error handling, readback, watchdog servicing and protection of loader/metadata.
5. **Image verifier:** declared bounds/length/version, CRC or cryptographic signature, target compatibility, vector validation and anti-rollback policy where required.
6. **Persistent update journal:** redundant/atomic records for active, pending, confirmed, attempt count and failure reason; power-cut-safe commit order.
7. **Boot manager:** choose loader/app/recovery, validate image and execute a controlled handover; application health-confirmation API.

## Application and linker changes

Create a single reviewed memory map. Link bootloader and application to distinct ranges; locate the app vector at the exact negotiated base; reserve metadata/boot flags in suitable erase units; assert boundaries and alignment; export shared symbols/config to package tooling. Update app reset/startup assumptions, VTOR, interrupt table, vector validation and an explicit health-confirmation hook. Ensure app code cannot erase loader or update metadata.

## Programming state machine

Enforce order: enter allowed programming session → conditions/security → validate target/range/length → erase only planned units → acknowledge accepted RequestDownload → transfer chunks with exact BSC and bounded payload → verify size/hash/signature/readback → atomically mark pending → positive TransferExit → reset. Define retries, duplicate blocks, timeout, abort/reset, negative responses and response timing. Never report success before flash/status checks complete.

## Failure/recovery policy

For each failure (bad range, sector erase, program, sequence, transport loss, CRC/signature, reset during update, app crash), define NRC, metadata, boot outcome, retry policy and operator recovery. Single-bank systems need a documented alternate recovery path because update interruption can destroy the only app. No automatic activation until verify passes.

## Integration and evidence

Write a ZGW/FCD ECU profile for wire IDs/route, addressing mode, sessions, security, RIDs, address format, max block, BSC, CRC/signature, reset and readiness. Update tool metadata only from agreed behavior. Keep FCD target sequences separate from ZGW's own FBL protocol. Label each design point as hardware manual, source, bench test or unresolved.
