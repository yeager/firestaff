# Pass 1147 — DM2 1031 UI logic gaps filled

## Source

skproject/SKULLWIN/c_1031.cpp

## Module

include/dm2_v1_1031_pc34_compat.h
src/dm2/dm2_v1_1031_pc34_compat.c

## Functions added

| Function | skproject source | Description |
|----------|-----------------|-------------|
| dm2_v1_1031_get_child_list | DM2_1031_023b (line 49) | Return child list from table_cd0 |
| dm2_v1_1031_get_click_rects | DM2_1031_024c (line 54) | Return click rect array from table_338c |
| dm2_v1_1031_clear_dirty | DM2_1031_04F5 (line 264) | Clear v1e03a8 and refresh |
| dm2_v1_1031_refresh_current | DM2_107B0 (line 284) | Update state with current mode |
| dm2_v1_1031_find_click_rect | DM2_1031_06b3 (line 414) | Recursive UI tree search by ID |
| dm2_v1_1031_click_by_id | DM2_1031_0781 (line 472) | Queue event for element by ID |
| dm2_v1_1031_lookup_event | DM2_1031_0c58 (line 747) | Look up click rect by event code |

## Functions not ported

| Function | Reason |
|----------|--------|
| DM2_1031_07d6 | Complex table reindexing with 6 nested loops and local arrays |
| DM2_1031_10c8 | Depends on c_buttongroup struct not yet modeled |
| DM2_CLICK_MAGICAL_MAP_AT | 170-line magical map handler with deep global state dependencies |

## Architecture

All functions use existing DM2_V1_1031_Callbacks and DM2_V1_1031_State.
find_click_rect uses gate condition offset +5 matching the skproject source.

## Verification

Compiles without errors via ninja -C build.
