# DM2 V1 Multiplayer Support — Audit

## Sources

- skproject (github.com/gbsphenx/skproject) — SKUL.LDA disassembly, SKWIN/SKULLWIN source
- Firestaff `docs/multiplayer-audit/multi_support.md`, `multi_firestaff.md`

## Finding: DM2 Has No Original Multiplayer

Dungeon Master II: The Legend of Skullkeep (1993) was designed and shipped as a
**single-player experience**. No multiplayer code, data structures, or
documentation exist in the original DM2 source or binaries.

### Evidence from skproject

Grep across all skproject source (C/H/ASM files):
- `SKWIN/`, `SKULLWIN/`, `SKWINDOS/`, `SKWINSPX/`, `DMDC2/`, `DM2GDED/` — **zero matches** for `multiplayer`, `network`, `LAN`, `modem`, `serial cable`, `co-op`, `two player`
- SKUL.LDA disassembly (522,128 lines): no multiplayer subsystem referenced
- No `fserver.c`, `network.c`, `client.c`, `peer.c`, or equivalent files
- No `MP_`, `NET_`, `SYNC_` prefixes in any source
- No port binding, socket, or UDP/TCP code in any DM2 source file

### DM2 Game Architecture

DM2 is a single-player dungeon crawler:
- Single party of up to 4 champions + companions
- Single input stream (keyboard/mouse)
- No network layer in original binary
- No session/join/accept protocol
- Executable distributed as standalone DOS/Windows binary

### Comparison with DM1

| Feature | DM1 | DM2 |
|---------|-----|-----|
| Multiplayer (original) | **No** | **No** |
| Serial cable support | **No** | **No** |
| Modem support | **No** | **No** |
| LAN support | **No** | **No** |
| Network code in source | None | None |

### Firestaff Multiplayer Framework

Firestaff (`include/firestaff_multiplayer.h`) defines a UDP-based stub for
"DM2-style LAN play" — this is a post-v2 design intent, not derived from any
original DM2 source. DM2 V1 has no multiplayer implementation.

### Files Examined

| Path | Type | Multiplayer? |
|------|------|-------------|
| skproject/SKWIN/ | Windows C/H | No |
| skproject/SKULLWIN/ | Windows C | No |
| skproject/SKWINDOS/ | DOS C | No |
| skproject/SKWINSPX/ | SPX variant | No |
| SKUL.LDA (522K lines) | Disassembly | No |

## Conclusion

| Item | Status |
|------|--------|
| Original DM2 multiplayer (serial) | **NOT SUPPORTED** |
| Original DM2 multiplayer (modem) | **NOT SUPPORTED** |
| Original DM2 multiplayer (LAN) | **NOT SUPPORTED** |
| Original DM2 network layer | **NONE** |
| skproject source evidence | **Zero multiplayer code** |
