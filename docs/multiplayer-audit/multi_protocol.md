# DM1 V1 Multiplayer Protocols — Source Lock Audit

## Source Evidence

**ReDMCSB:** No multiplayer code exists; DM1 V1 has no networking layer.
**Firestaff:** `src/engine/firestaff_multiplayer.c` (157 lines) — FUTURES-only stub.

## Finding: No Original DM1 Multiplayer Protocol

Since DM1 V1 has no multiplayer support (see multi_support.md), there is no original protocol to document.

### Firestaff Stub Protocol (Not Yet Implemented)

The `FS_MP_Packet` structure in `firestaff_multiplayer.h` defines a UDP-based protocol:

```
FS_MP_Packet layout:
  magic     (4 bytes) — 0x46535450 ("FSTP")
  type      (1 byte)  — MP_PKT_JOIN/ACCEPT/INPUT/STATE/CHAT/LEAVE
  player_id (1 byte)
  seq       (2 bytes) — sequence counter
  tick      (4 bytes) — game tick
  data      (variable) — up to 1012 bytes
```

### Packet Types
| Type | Direction | Description |
|------|-----------|-------------|
| MP_PKT_JOIN | Client→Host | Request to join session |
| MP_PKT_ACCEPT | Host→Client | Accept/reject join |
| MP_PKT_INPUT | Client→Host | Player command input |
| MP_PKT_STATE | Host→Client | Game state broadcast |
| MP_PKT_CHAT | Any | Text chat |
| MP_PKT_LEAVE | Any | Player disconnect |

### Implementation Status (firestaff_multiplayer.c)
- `fs_mp_host_start()` ✅ Creates UDP socket, binds port 7777
- `fs_mp_client_join()` ✅ Sends JOIN packet, connects to host
- `fs_mp_receive()` ✅ Receives packets, dispatches by type (JOIN/STATE/INPUT/LEAVE)
- `fs_mp_send_input()` ❌ **FUTURE** — stub, no-op
- `fs_mp_broadcast_state()` ❌ **FUTURE** — stub, no-op

### Comment from firestaff_multiplayer.c
```c
int fs_mp_send_input(FS_MP_State *mp, int command) {
    (void)mp; (void)command;
    /* FUTURE: pack input into packet and send to host.
     * Multiplayer is a post-v2 feature — requires network protocol
     * design, state synchronization, and host migration. */
    return 0;
}
```

## Conclusion

| Item | Status |
|------|--------|
| Original DM1 protocol | **NONE** (single-player only) |
| Firestaff protocol | UDP, defined but incomplete |
| State sync | **FUTURE** (stub) |
| Input transmission | **FUTURE** (stub) |
| Host migration | **FUTURE** (not designed) |

Source lock confirmed: ReDMCSB `Toolchains/Common/Source/` — zero networking code.
