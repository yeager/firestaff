# Pass 1108 — DM2 V1 Savegame Unified Module (c_savegame.cpp)

## What

Unified aggregation module for the complete DM2 save/load subsystem, covering
all 29 functions from skproject c_savegame.cpp (2288 lines). The implementation
is decomposed across 14 granular sub-modules; this module provides a single
convenience header, layout constants, and integration tests.

## Implemented functions

| Function | skproject source | Sub-module |
|----------|-----------------|------------|
| DM2_GAME_LOAD | c_savegame.cpp:1415 | save_load, save_orchestrator |
| DM2_GAME_SAVE_MENU | c_savegame.cpp:2087 | save_orchestrator |
| DM2_READ_DUNGEON_STRUCTURE | c_savegame.cpp:230 | save_dungeon_data |
| DM2_LOAD_NEW_DUNGEON | c_savegame.cpp:616 | save_load |
| DM2_SKLOAD_READ | c_savegame.cpp:638 | save_load |
| DM2_SKSAVE_WRITE | c_savegame.cpp:1587 | save_load |
| DM2_SUPPRESS_READER | c_savegame.cpp:655 | save_load |
| DM2_SUPPRESS_WRITER | c_savegame.cpp:1596 | save_load |
| DM2_SUPPRESS_INIT | c_savegame.cpp:1345 | save_load |
| DM2_SUPPRESS_FLUSH | c_savegame.cpp:1664 | save_load |
| DM2_READ_1BIT | c_savegame.cpp:735 | save_load |
| DM2_WRITE_1BIT | c_savegame.cpp:1658 | save_load |
| DM2_READ_RECORD_CHECKCODE | c_savegame.cpp:808 | save_read_record_checkcode |
| DM2_WRITE_RECORD_CHECKCODE | c_savegame.cpp:1739 | save_write_record_checkcode |
| DM2_READ_SKSAVE_DUNGEON | c_savegame.cpp:1108 | save_load_extra_dungeon_data |
| DM2_STORE_EXTRA_DUNGEON_DATA | c_savegame.cpp:1958 | save_store_extra_dungeon_data |
| DM2_WRITE_POSSESSION_INDICES | c_savegame.cpp:1684 | save_write_possession_indices |
| DM2_COMPACT_TIMERLIST | c_savegame.cpp:1715 | save_compact_timerlist |
| DM2_PROCEED_GLOBAL_EFFECT_TIMERS | c_savegame.cpp:1041 | save_post_load_global_effects |
| DM2_ADD_INDEX_TO_POSSESSION_INDICES | c_savegame.cpp:790 | save_write_possession_indices |
| DM2_SELECT_LOAD_GAME | c_savegame.cpp:744 | save_load |
| DM2_3a15_020f | c_savegame.cpp:1351 | save_post_load_timer_rebuild |
| DM2_savegame_3a15_0002 | c_savegame.cpp:82 | save_timers |
| DM2_savegame_2066_2498 | c_savegame.cpp:128 | save_dungeon_data |
| DM2_2066_197c | c_savegame.cpp:976 | save_read_record_checkcode |
| DM2_2066_062b | c_savegame.cpp:1003 | save_load_extra_dungeon_data |
| DM2_2066_0b44 | c_savegame.cpp:1942 | save_store_extra_dungeon_data |
| DM2_1c9a_3bab | c_savegame.cpp:2043 | save_orchestrator |
| FSUBSAVE | c_savegame.cpp:2062 | save_orchestrator |

## Sub-modules (14)

| Sub-module | Header | Functions |
|------------|--------|-----------|
| save_load | dm2_v1_save_load.h | SuppressReader/Writer, SKLOAD_READ, SKSAVE_WRITE, slots |
| save_suppress_masks | dm2_v1_save_suppress_masks_pc34_compat.h | Savegame buffer, hero, timer masks |
| save_record_masks | dm2_v1_save_record_masks_pc34_compat.h | Record sizes, diff masks per type |
| save_timers | dm2_v1_save_timers_pc34_compat.h | Timer accessors, sort, materialize |
| save_compact_timerlist | dm2_v1_save_compact_timerlist_pc34_compat.h | Pre-save timer cleanup |
| save_dungeon_data | dm2_v1_save_dungeon_data_pc34_compat.h | Tile suppress, teleporter refs |
| save_read_record_checkcode | dm2_v1_save_read_record_checkcode_pc34_compat.h | Recursive record reader |
| save_write_record_checkcode | dm2_v1_save_write_record_checkcode_pc34_compat.h | Recursive record writer |
| save_write_possession_indices | dm2_v1_save_write_possession_indices_pc34_compat.h | Possession index writer |
| save_load_extra_dungeon_data | dm2_v1_save_load_extra_dungeon_data_pc34_compat.h | Load tile/record data |
| save_store_extra_dungeon_data | dm2_v1_save_store_extra_dungeon_data_pc34_compat.h | Store tile/record data |
| save_post_load_global_effects | dm2_v1_save_post_load_global_effects_pc34_compat.h | Global effect timers |
| save_post_load_timer_rebuild | dm2_v1_save_post_load_timer_rebuild_pc34_compat.h | Hero/record timer rebuild |
| save_orchestrator | dm2_v1_save_orchestrator_pc34_compat.h | GAME_SAVE_MENU flow |

## Constants verified

- `DM2_SAVEGAME_BUFFER_SIZE` = 0x3C (s_savegamebuffer)
- `DM2_SAVEGAME_HEADER_SIZE` = 0x2A (s_hex30)
- `DM2_SAVEGAME_DUNGEON_HEADER_SIZE` = 0x2C (s_sgwords)
- 16 record types (0x00-0x0F) with word-aligned sizes
- 8 tile types (0-7) with correct suppress bit counts
- 8 actuator sub-types with 9-bit suppress field

## Test results

17 integration tests, all PASS:
- Source evidence and sub-module consistency
- Savegame buffer, record type, tile type constants
- Tile suppress sizes matching skproject switch statement
- Record sizes word-alignment invariant
- Timer accessor behavior on zeroed records
- Compact timerlist no-op on full list
- Suppress reader/writer round-trip (3-byte all-bits)
- Suppress reader single-bit read
- Teleporter forward reference logic

## Source lock

- Reference: `skproject/SKULLWIN/c_savegame.cpp` lines 1-2288
- All 29 functions covered across 14 sub-modules
- Callback-based architecture throughout (no global state)
