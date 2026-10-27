# DM2 V1 Server Architecture — Audit

## Sources

- skproject (github.com/gbsphenx/skproject)
- `include/firestaff_multiplayer.h`
- `src/engine/firestaff_multiplayer.c`

## Finding: No Original DM2 Server Architecture

DM2 has no multiplayer. There is no server, client, host, or any architecture
for multiplayer in the original game.

### Evidence from skproject

- Zero server code in SKWIN/SKULLWIN/SKWINDOS
- Zero `fserver`, `gameserver`, `dedicated_server` in any source
- No listen socket, no port binding, no client management
- SKUL.LDA disassembly: no server loop, no session management

### DM2 Original Architecture (Single-Player)

DM2 is a single-executable, single-process game:
- No server process
- No client process
- No IPC between processes
- No port 7777 or any network port
- No `MP_MAX_PLAYERS` or player limit enforcement
- `DM2_V1_GameState` is owned entirely by one process

### Firestaff Host/Client Model (Design Intent, Not Implemented)

Firestaff multiplayer defines a host/client architecture as a stub:

```
firestaff_multiplayer.h:
  /* Host model:
   * - Host runs game loop, broadcasts state
   * - Clients send inputs, receive state updates
   * - All parties see same dungeon in real-time */
```

Functions:
- `fs_mp_host_start()` — creates UDP socket on port 7777 (stub)
- `fs_mp_client_join()` — connects to host (stub)
- `fs_mp_receive()` — dispatches JOIN/LEAVE/STATE (implemented)
- `fs_mp_send_input()` — **stub** (no-op)
- `fs_mp_broadcast_state()` — **stub** (no-op)

**Server-like behaviors not implemented:**
- No game loop on host side — `fs_mp_broadcast_state()` is empty
- No client state management — player list has `connected` flag but no sync
- No session persistence — no save/resume of multiplayer sessions

### Peer-to-Peer vs Dedicated Server

DM2 original: **neither** — single-player
Firestaff design intent: **authoritative host** (one player runs game, others are thin clients)
Firestaff implementation: **neither fully** — host_start/client_join are stubs

### What a DM2 Server Would Need (Future Design)

A real DM2 multiplayer server would require:
1. **Game loop host** — run `dm2_v1_game_loop()` as authoritative tick
2. **Client registry** — track up to 4 players by `player_id`
3. **State serialization** — serialize `DM2_V1_GameState` each tick
4. **Input collection** — queue inputs from all clients with `seq`/`tick`
5. **State broadcast** — send STATE packets at fixed tick rate
6. **Disconnect handling** — remove player, free champion slot

None of this exists in DM2 V1. Firestaff has placeholder structures but no implementation.

## Conclusion

| Item | Status |
|------|--------|
| Original DM2 server | **NONE** (single-player) |
| Original DM2 client | **NONE** (single-player) |
| Dedicated server | **N/A** |
| Authoritative host | **DESIGN** (Firestaff, not implemented) |
| Peer-to-peer | **N/A** |
| Host start | **STUB** — socket opened, no game loop |
| Client join | **STUB** — connected but no state received |
| Source lock | skproject: zero server/client code |
