# Pass 1133 — DM2 V1 level loading/initialization (c_loadlevel.cpp)

## Source
skproject SKULLWIN/c_loadlevel.cpp — 9 functions for level loading and dynamic resource management.

## Ported functions
- DM2_MARK_DYN_LOAD — mark dynamic load entry with hi-res flag support
- DM2_LOAD_MISCITEM — load misc item into sorted table
- DM2_LOAD_LOCALLEVEL_DYN — load local level dynamic resources
- DM2_LOAD_NEWMAP — load new map with graphics table initialization
- DM2_INIT_LEVEL_GRAPHICS — initialize level graphics state from GDAT

## Files
- include/dm2_v1_loadlevel_pc34_compat.h
- src/dm2/dm2_v1_loadlevel_pc34_compat.c
- tests/test_dm2_v1_loadlevel_pc34_compat.c

## Test
All tests pass (dm2_v1_loadlevel_pc34_compat).
