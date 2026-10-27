# DM2 V1 Network Protocol — Audit

## Sources

- skproject (github.com/gbsphenx/skproject) — all subdirectories
- Firestaff `docs/multiplayer-audit/multi_protocol.md`

## Finding: No Original DM2 Network Protocol

Since DM2 has no multiplayer support (see dm2_multiplayer.md), there is no original
network protocol to document.

### Evidence

- skproject: zero socket code, zero packet structures, zero protocol headers
- No `MP_PKT_*`, `NET_*`, `PKT_*` constants in any C/ASM file
- SKUL.LDA disassembly: no UDP/TCP/IPX/XMODEM/ZMODEM references
- No port 7777 or any fixed port in any DM2 source

### Firestaff FSTP Protocol (Post-V2 Stub)

The Firestaff `firestaff_multiplayer.h` defines a **FSTP** (Firestaff Transfer Protocol)
as a future extension for DM2-style LAN play:

```c
#define MP_MAGIC 0x46535450  /* "FSTP" */
#define MP_PORT 7777
#define MP_PACKET_SIZE 1024
#define MP_MAX_PLAYERS 4

FS_MP_Packet layout:
  magic     (4 bytes) — 0x46535450 ("FSTP")
  type      (1 byte)  — MP_PKT_JOIN/ACCEPT/INPUT/STATE/CHAT/LEAVE
  player_id (1 byte)
  seq       (2 bytes) — sequence counter
  tick      (4 bytes) — game tick
  data      (variable) — up to 1012 bytes
```

This protocol is:
- **NOT derived from DM2 source** — DM2 has no network layer
- **NOT implemented** — `fs_mp_send_input()` and `fs_mp_broadcast_state()` are stubs
- **Design intent only** — for future post-v2 DM2-style LAN

### Packet Types (Firestaff Stub)

| Type | Direction | Description |
|------|-----------|-------------|
| MP_PKT_JOIN | Client→Host | Request to join session |
| MP_PKT_ACCEPT | Host→Client | Accept/reject join |
| MP_PKT_INPUT | Client→Host | Player command input (stub) |
| MP_PKT_STATE | Host→Client | Game state broadcast (stub) |
| MP_PKT_CHAT | Any | Text chat |
| MP_PKT_LEAVE | Any | Player disconnect |

### DM2 V1 vs FSTP

DM2 V1 (`src/dm2/dm2_v1_*.c`) is the current single-player implementation stub.
FSTP multiplayer is a future design layer that does not yet exist in DM2 V1 code.

## Conclusion

| Item | Status |
|------|--------|
| Original DM2 protocol | **NONE** (single-player only) |
| Original DM2 packet format | **N/A** |
| Firestaff FSTP | UDP, defined but not implemented |
| State sync | **FUTURE** (stub) |
| Input transmission | **FUTURE** (stub) |
| Source lock | skproject: zero networking code |
