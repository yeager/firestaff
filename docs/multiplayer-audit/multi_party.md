# DM1 V1 Party Sharing in Multiplayer — Source Lock Audit

## Source Evidence

**ReDMCSB:** No multiplayer code — party sharing in multi-player is N/A
**Firestaff:** include/firestaff_multiplayer.h, src/engine/firestaff_multiplayer.c

## Finding: Original DM1 Has No Multiplayer Party Sharing

DM1 V1 is a single-player game. There is no multiplayer party sharing mechanism.

The original game party management is entirely single-process, single-party.

### Original DM1 Party Design (ReDMCSB Source)
From Toolchains/Common/Source/ party files PARTY.C, PARTYMGMT.C, RECRUIT.C:
- Party of up to 4 champions
- Champion control: single input stream, single cursor
- Inventory sharing: drop/pickup, chest storage, hand exchange
- No concurrent controller concept

### Firestaff Multiplayer Party Design (Stub)
include/firestaff_multiplayer.h:
  typedef struct {
    char name[32];
    int connected;
    int champion_index;  // which champion this player controls
    uint32_t last_seen_tick;
  } FS_MP_Player;

Design intent: each player controls one champion from the shared party.
DM2-style LAN play described in header comment.

### Champion Control Model (Firestaff Stub)
- Host: runs game loop, broadcasts state
- Clients: send inputs, receive state updates
- All parties see same dungeon in real-time
- One champion per player, controlled independently

### Item Sharing
No item sharing protocol defined. Stub has no item trade or pickup sync.

## Conclusion

| Item | Status |
|------|--------|
| Original DM1 multi-player champion control | N/A (single-player) |
| Original DM1 multi-player item sharing | N/A (single-player) |
| Firestaff champion-per-player design | STUB - champion_index field defined but unused |
| Firestaff item sharing protocol | NOT DESIGNED - no code |
| Source lock | ReDMCSB: zero multiplayer party code |
