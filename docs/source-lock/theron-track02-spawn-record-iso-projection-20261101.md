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
