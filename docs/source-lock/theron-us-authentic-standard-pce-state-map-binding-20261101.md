# Theron US authentic standard-PCE state map binding

**Captured-state identity:** Mednafen 1.32.1 USA state MD5
`f17f377df210b4a3ae904a13fb85a7f0` (SHA-256
`7687b795eccfdd4974d526714c9ccde4282e147997e3e44946956f64435add4a`).
**Media identity:** USA raw MODE1/2352 Track 02 MD5
`f23601102138f87c33025877767ebf76`, 8,104,992 bytes. Both files remain
outside the repository.

This report records a read-only comparison between the named Mednafen state
variables and source bytes from the authenticated USA Track 02. It does not
alter the state or media and does not claim that save-state contents prove a
cold CD-to-RAM transfer.

## Source bytes and saved RAM

The US Akutuba descriptor offsets are defined in
`src/theron/theron_v1_track02_dungeon_map.c`: dimensions at user-data offset
`0x2a0f1`, map bytes at `0x2a2d7`, and four maps. The loader adds the 225-sector
Track 02 pregap (`225 * 2048 = 0x70800`) to those offsets in its full
MODE1/2048 user-data projection. The resulting source locations are:

| Map | Dimensions | User-data projection offset | Size | `HuC.SysCardRAM` offset | SHA-256 |
|---:|---:|---:|---:|---:|---|
| 0 | 5 × 7 | `0x9aad7` | 35 | `0x18ad7` | `c389b581d87cd5236e4b0a895894d0004bda548d1f475f2316379cb0c6356b9d` |
| 1 | 18 × 12 | `0x9aafa` | 216 | `0x18afa` | `7f50e4c089b86ae4485e6f42ff6dd5b1d3903ad7353d5a1d2f532c4147213b31` |
| 2 | 18 × 16 | `0x9abd2` | 288 | `0x18bd2` | `f30cb225dfc8bb30b2be3b415caa51d639f166fc6821c641b6094b363226b15c` |
| 3 | 12 × 11 | `0x9acf2` | 132 | `0x18cf2` | `6ffd78fb9204cc25732dcec0da20c0934e3c37fe01a4e876bebd628ec58dc411` |

Each complete map byte array occurs exactly once in the state's named
`HuC.SysCardRAM` variable, at the offset shown. The state has
`CPU.MPR = ff f8 68 74 79 70 6d 00`, so MPR slot 3 selects physical page
`$74` for CPU addresses `$6000–$7fff`. `HuC.SysCardRAM` is banked from physical
page `$68`; offset `0x18ad7` is page `$74`, page offset `$0ad7`, corresponding
to CPU address `$6ad7` under this saved MPR mapping. The four array starts are
therefore `$6ad7`, `$6afa`, `$6bd2` and `$6cf2` in this state.

The existing US viewport collision trace records reads at physical
`$0e8ae1` and `$0e8af7`, with values `$20` and `$10`. These addresses are
respectively offsets 10 and 32 from the saved map-0 base `$0e8ad7`; the same
bytes occur at offsets 10 and 32 in the authenticated map-0 source array.
This is consistent with those reads addressing map-0 bytes if page `$74` was
still selected at the time. That capture did not retain a contemporaneous MPR
snapshot at each read, so this address-to-array relation is not a complete
runtime source/consumer join.

## Evidence boundary

All four arrays are resident together. The saved MPR maps their physical page
into the CPU window, but it does not identify which of the four maps the game
selected. This state does not bind a live map selector, party pose to a map
cell, movement/collision semantics, or a VDC/VCE/VRAM update to a consumed
tile. It also cannot establish the earlier CD transfer that populated the
saved RAM. Keep map identity and gameplay semantics unresolved until an
isolated original-game trace binds the selector and pose-bound tile consumer.

The earlier viewport trace and its movement boundary are documented in the
[original viewport-command capture](theron-original-viewport-command-capture-2026-08-21.md).
The broader current evidence overview is
[Theron capture readiness](../theron/THERON_CAPTURE_READINESS.md).
