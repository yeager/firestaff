# DM1 V1 Multiplayer Limitations — Source Lock Audit

## Source Evidence

**ReDMCSB:** No multiplayer code — N/A
**Firestaff:** `include/firestaff_multiplayer.h`, `src/engine/firestaff_multiplayer.c`

## Finding: No Original DM1 Multiplayer = No Limitations to Document

DM1 V1 has no multiplayer mode. Therefore there are no "multiplayer restrictions" in the original game.

### Firestaff Framework Limits (Stubs)

`include/firestaff_multiplayer.h` defines:
```c
#define MP_MAX_PLAYERS 4
#define MP_PORT 7777
#define MP_PACKET_SIZE 1024
#define MP_MAGIC 0x46535450  /* "FSTP" */
```

These are design limits for the Firestaff multiplayer framework, NOT limitations of the original DM1 game (which never had multiplayer).

### Firestaff Framework Status
- Max players: 4 (defined, not enforced — `fs_mp_send_input()` is stub)
- Port: 7777 (UDP)
- Packet size: 1024 bytes
- All protocol functions either stub-only or partially implemented (receive only)

### Original DM1 Game Limits (Single Player Context)
- Max champions per party: **4** (in-engine limit, single-player design)
- Max party size enforced by party management in ReDMCSB `PARTY.C`, `PARTYMGMT.C`
- No networking = no connection limits, no bandwidth limits, no sync constraints

## Conclusion

| Item | Status |
|------|--------|
| Original DM1 multi-player max players | **N/A** (no multiplayer) |
| Original DM1 feature restrictions in multi mode | **N/A** (no multiplayer) |
| Firestaff MP_MAX_PLAYERS | **4** (design limit, not enforced) |
| Firestaff MP_PORT | **7777/UDP** |
| Firestaff protocol completeness | **STUB** — input/state not implemented |
| Source lock | ReDMCSB: zero multiplayer code |

