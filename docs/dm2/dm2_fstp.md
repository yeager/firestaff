# DM2 V1 FSTP — Audit

## Sources

- `include/firestaff_multiplayer.h` (Firestaff)
- `src/engine/firestaff_multiplayer.c`
- `docs/multiplayer-audit/multi_protocol.md`

## Finding: FSTP Is a Firestaff Framework Concept, Not DM2 Original

FSTP ("Firestaff Transfer Protocol") is defined in Firestaff's `firestaff_multiplayer.h`
as a future UDP-based protocol for "DM2-style LAN play." It does not exist in any
original DM2 source.

### FSTP in Firestaff (Post-V2 Stub)

FSTP is a packet protocol design that does not ship in any current DM2 V1 code:

```c
#define MP_MAGIC 0x46535450  /* "FSTP" — ASCII 'F' 'S' 'T' 'P' */
#define MP_PORT 7777
#define MP_PACKET_SIZE 1024

FS_MP_Packet:
  magic     (4 bytes) — 0x46535450, validates FSTP frame
  type      (1 byte)  — packet class
  player_id (1 byte)  — source player (0-3)
  seq       (2 bytes) — sequence counter for ordering
  tick      (4 bytes) — game tick of sender
  data      (0-1012 bytes) — type-specific payload
```

### FSTP Packet Flow (Stub Only)

| Direction | Type | Status |
|-----------|------|--------|
| Client→Host | MP_PKT_JOIN | Stub: `fs_mp_client_join()` implemented |
| Host→Client | MP_PKT_ACCEPT | Stub: `fs_mp_receive()` dispatches |
| Client→Host | MP_PKT_INPUT | **Stub**: `fs_mp_send_input()` no-op |
| Host→Client | MP_PKT_STATE | **Stub**: `fs_mp_broadcast_state()` no-op |
| Any | MP_PKT_CHAT | Not implemented |
| Any | MP_PKT_LEAVE | Stub: `fs_mp_receive()` dispatches |

### Why FSTP Is Not DM2

DM2 V1 has:
- No socket code
- No packet framing
- No UDP/TCP in original source
- No port 7777
- No sequence numbers
- No tick field

FSTP was designed in Firestaff as an original protocol to support future DM2-style
LAN play. It is not reverse-engineered from any DM2 binary or source. DM2 has no
network layer to have an FSTP-like protocol.

### Relation to skproject

- skproject SKWIN/SKULLWIN/SKWINDOS: zero FSTP references
- SKUL.LDA disassembly: zero FSTP references
- DM2 original: no protocol, no packets, no networking

## Conclusion

| Item | Status |
|------|--------|
| FSTP in original DM2 | **NONE** — DM2 has no networking |
| FSTP magic ("FSTP") | Firestaff original design, not DM2-derived |
| FSTP in Firestaff | **STUB** — defined but input/state not implemented |
| FSTP protocol completeness | **PARTIAL** — only JOIN/LEAVE/ACCEPT working |
| Source lock | FSTP is new Firestaff code, not skproject-derived |
