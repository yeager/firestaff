# Theron Track 02 spawn-record ISO projection

## Scope

This source lock establishes that the authenticated US and JP MODE1/2048
Track 02 ISO projections contain the same eight creature-pointer records and
five regular-spawn zone records as their regional MODE1/2352 BINs. It admits
these bytes to native source metadata; it does not establish runtime RNG,
creature placement, AI, combat, or category semantics.

## Authenticated inputs

| Region | Raw Track 02 MD5 | Full ISO MD5 | Raw INDEX 01 pregap | ISO pointer-table offset |
|---|---|---|---:|---:|
| US | `f23601102138f87c33025877767ebf76` | `ceb02343868f80cec899e9b239aff2da` | 225 sectors | `0x203818` |
| JP Rev. 1 | `b7afb338ad31be1025b53f9aff12d73a` | `62a39bbf43415c9739c41c2481080a49` | 224 sectors | `0x203818` |

The corresponding raw user-data pointer-table offsets are US `0x274018` and
JP `0x273818`. Subtracting each authentic CUE pregap in 2048-byte sectors
produces the common ISO offset `0x203818`. The five zone-record offsets are
`0x203858`, `0x2038d7`, `0x203902`, `0x203929`, and `0x203950` in both ISO
projections. The regional roster marker at `0x2039ef` is also checked against
the exact retail bytes.

## Pointer-to-zone source relationship

The eight decoded pointer records contain the same `spawn_data_offset` values
in both regions: `0x0058`, `0x00d7`, `0x0102`, `0x0129`, `0x0150`, `0`, `0`,
`0`. The five nonzero offsets resolve to the five zone records relative to
the regional table base (`0x274000` for US raw user data, `0x273800` for JP
raw user data, and `0x203800` in the pregap-adjusted ISO projection):

| Pointer index | `spawn_data_offset` | Zone index | US raw user-data offset | JP raw user-data offset | ISO user-data offset |
|---:|---:|---:|---:|---:|---:|
| 0 | `0x0058` | 0 | `0x274058` | `0x273858` | `0x203858` |
| 1 | `0x00d7` | 1 | `0x2740d7` | `0x2738d7` | `0x2038d7` |
| 2 | `0x0102` | 2 | `0x274102` | `0x273902` | `0x203902` |
| 3 | `0x0129` | 3 | `0x274129` | `0x273929` | `0x203929` |
| 4 | `0x0150` | 4 | `0x274150` | `0x273950` | `0x203950` |

Pointer indices 5–7 have zero offsets and do not resolve to any of the five
regular-spawn zone records. This is an authenticated static table join only.
It does not establish that the original runtime selects these pointers,
interpret them as categories, or spawns a creature from them. It also does not
identify RNG, placement, AI, combat, or JP category semantics. The native
lookup API returns a zone index only for this source-record relationship; it
must not be treated as spawn authorization.

`theron_v1_track02_decode_spawn_source_iso()` first verifies the complete
regional image MD5, then reads these bounded fields from the ISO user-data
stream. Its real-media regression compares every decoded pointer and zone
field to the independently hash-verified raw BIN. It also flips a byte in a
copy of the ISO and requires admission to fail. The startup dungeon entry now
binds these records for authentic ISO media as it already did for raw BINs.

The JP regular-spawn category remains withheld by
`theron_v1_world_track02_spawn_category()`. The ISO binding does not add a
runtime consumer or create creature records. See the [capture readiness
summary](../theron/THERON_CAPTURE_READINESS.md) for the remaining original-game
execution and renderer joins.
