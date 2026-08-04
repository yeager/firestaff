# Pass 1139 — DM2 V1 recalc light level (c_light.cpp)

## Source
skproject SKULLWIN/c_light.cpp — DM2_RECALC_LIGHT_LEVEL added to existing light_ops module.

## Added function
- DM2_RECALC_LIGHT_LEVEL (c_light.cpp:39) — recalculates dungeon light level. Checks map tile byte 0xD for light flag (bit 0x40). Enumerates party champion items, collecting those with DBSPEC flag 0x10 (light-emitting). Sorts charges descending, accumulates via table1d6702 lookup with right-shift decay. Adds ambient light, savegame light override, and weather light offset. Clamps result to range 0-5. Writes final level via output setter callback.

## Files modified
- include/dm2_v1_light_ops_pc34_compat.h (added DM2_V1_RecalcLightLevelCallbacks and declaration)
- src/dm2/dm2_v1_light_ops_pc34_compat.c (added implementation)

## Test
All tests pass (dm2_v1_light_ops).
