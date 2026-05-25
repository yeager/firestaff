# DM2 V1 State Synchronization — Audit

## Sources

- skproject (github.com/gbsphenx/skproject)
- Firestaff `docs/multiplayer-audit/multi_firestaff.md`, `docs/multiplayer-audit/multi_protocol.md`

## Finding: No Original DM2 State Sync

DM2 is a single-player game with no multiplayer. There is no original state
synchronization mechanism in DM2 source.

### Evidence

- skproject: no sync code, no tick broadcast, no state marshalling
- `DM2_V1_GameState` (include/dm2_v1_game.h) is single-player only
- No `mp_state_*`, `sync_*`, `broadcast_*` functions in any DM2 C file
- SKUL.LDA disassembly: no tick-synchronization, no lock-step, no state delta

### Firestaff State Sync (Post-V2 Stub)

Firestaff multiplayer defines a state-sync architecture in `firestaff_multiplayer.h`:

**Host model** (design intent, not implemented):
- Host runs the authoritative game loop
- Host broadcasts `DM2_V1_GameState` to clients each tick
- Clients render received state, send inputs
- `fs_mp_broadcast_state()` — stub, returns 0 (no-op)

**State update cycle** (not implemented):
```
Host tick:
  1. Process own input
  2. Run game logic → new state
  3. Serialize DM2_V1_GameState
  4. Broadcast STATE packet (UDP)
  5. Clients receive, deserialize, render
  6. Clients send INPUT packet
  7. Host receives, queues for next tick
```

**State struct** (single-player, no sync):
```c
typedef struct {
    int party_x, party_y, party_dir;
    int gold, reputation;
    int dungeon_level;
    DM2_V1_Champion champions[4];
    DM2_V1_CompanionState companions[8];
    // ... no multiplayer fields
} DM2_V1_GameState;
```

### What Sync Would Need (Future Design)

For DM2 multiplayer, state sync would require:
1. **Tick determinism** — same inputs → same state (DM2 tick is deterministic)
2. **State compression** — `DM2_V1_GameState` serialization (DM2 save format)
3. **Delta compression** — only send changed dungeon tiles/champions
4. **Input queuing** — buffer client inputs with sequence numbers
5. **Rollback** — discard late/duplicate inputs on host

None of this exists in DM2 V1 source. Firestaff `fs_mp_send_input()` is a stub.

## Conclusion

| Item | Status |
|------|--------|
| Original DM2 state sync | **NONE** (single-player) |
| Multiplayer tick model | **N/A** |
| State broadcast | **N/A** |
| Input queuing | **N/A** |
| Delta compression | **N/A** |
| Firestaff state sync | **STUB** — no implementation |
| Source lock | skproject: zero sync code |
