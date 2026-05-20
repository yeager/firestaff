# DM1 V1 C006 Group Generator Cell Placement Source Lock (2026-05-20)

Primary source: ReDMCSB WIP20210206 under `Toolchains/Common/Source/`.

## Source Evidence

- `TIMELINE.C:964-974`: C05 corridor events identify `C006_SENSOR_FLOOR_GROUP_GENERATOR`, decode count/randomize, health multiplier, and call `F0185_GROUP_GetGenerated(..., M004_RANDOM(4), mapX, mapY)`.
- `GROUP.C:525-541`: generated groups use `0xFF` cells for a single centered creature. Multi-creature groups consume one random starting cell, then pack each creature cell with `F0178_GROUP_GetGroupValueUpdatedWithCreatureValue`, advancing by one cell per creature and by an extra cell for half-square creatures.
- `GROUP.C:174-185` and `DEFS.H:1367-1369`: packed group values store each creature in 2-bit slots; `0xFF` is the single-centered sentinel.
- `DUNGEON.C:668-733`, `DEFS.H:1597`, `DEFS.H:1613`: the third `CREATURE_INFO` initializer field is `Attributes`; low two bits encode creature size, with `1` meaning half-square.
- `GROUP.C:543-547` and `MOVESENS.C:316-907`: after group data is populated, `F0267_MOVE_GetMoveResult_CPSCE` performs world placement and may defer/kill the generated group. Full world insertion remains a separate runtime-dispatch slice.

## Firestaff Slice

`GeneratorResult_Compat.spawnedGroupCells` now records the source-style GROUP.Cells byte produced by `F0860_RUNTIME_HandleGroupGenerator_Compat`. The field reuses the prior reserved int32 slot, so `GeneratorResult_Compat` remains 112 bytes and existing serialization shape is unchanged.
