# Pass 1099 — DM2 baked-in data tables (dm2data.cpp)

## Source

skproject/SKWINSPX/src/v5/dm2data.cpp

## What was ported

ROM-like lookup tables embedded in the DM2 executable, used across
direction computation, music, sound, creature AI, viewport rendering,
and item/door queries.

### Tables ported

| skproject symbol | Firestaff symbol | Size | Purpose |
|---|---|---|---|
| table1d27fc[4] | dm2_v1_dir_dx[4] | 8B | Direction X deltas |
| table1d2804[4] | dm2_v1_dir_dy[4] | 8B | Direction Y deltas |
| table1d645d[6] | dm2_v1_table_1d645d[6] | 6B | Creature positioning |
| table1d70f0[24] | dm2_v1_table_1d70f0[24] | 24B | Clock-hour sound duration |
| table1d14e2[24] | dm2_v1_table_1d14e2[24] | 48B | Sound frequency |
| tblMusicsMap[64] | dm2_v1_music_map[64] | 64B | Level-to-track (SONGLIST.DAT) |
| table1d7092[8] | dm2_v1_table_1d7092[8] | 32B | Bitmap color table |
| table1d7072[8] | dm2_v1_table_1d7072[8] | 32B | Bitmap color table |
| table1d7052[8] | dm2_v1_table_1d7052[8] | 32B | Bitmap color table |
| table1d7042[4] | dm2_v1_table_1d7042[4] | 16B | Bitmap color table |
| table1d7029[20] | dm2_v1_table_1d7029[20] | 20B | Creature AI sub-skill |
| table1d7012[23] | dm2_v1_table_1d7012[23] | 23B | Creature type class |
| table1d6f4c[16] | dm2_v1_table_1d6f4c[16] | 16B | Item type flags |
| table1d26a8[32] | dm2_v1_table_1d26a8[32] | 32B | Direction-position map |
| table1d6290[9] | dm2_v1_table_1d6290[9] | 9B | Creature damage class |
| table1d6299[5] | dm2_v1_table_1d6299[5] | 10B | Creature action sub-type |
| table1d6f27[5] | dm2_v1_table_1d6f27[5] | 5B | Door ordinal query |

### Additional tables (batch 2)

| skproject symbol | Firestaff symbol | Size | Purpose |
|---|---|---|---|
| table1d62ee[30] | dm2_v1_table_1d62ee[30] | 30B | Creature AI behavior |
| table1d62e8[4] | dm2_v1_table_1d62e8[4] | 4B | Direction reverse |
| table1d62e0[4] | dm2_v1_table_1d62e0[4] | 8B | Creature group offsets |
| table1d62b0[8][2] | dm2_v1_table_1d62b0[8][2] | 32B | 8-dir neighbor offsets |
| table1d62d0[4][2] | dm2_v1_table_1d62d0[4][2] | 16B | Perpendicular offsets |
| table1d3ffc[4] | dm2_v1_table_1d3ffc[4] | 4B | Dir X delta (i8) |
| table1d3ff8[4] | dm2_v1_table_1d3ff8[4] | 4B | Dir Y delta (i8) |
| table1d27c4[8] | dm2_v1_table_1d27c4[8] | 16B | Viewport item ordinals |
| table1d27d4[10] | dm2_v1_table_1d27d4[10] | 20B | Viewport item ordinals B |
| table1d268e[6] | dm2_v1_table_1d268e[6] | 6B | Creature slot indices |
| table1d2660[16] | dm2_v1_table_1d2660[16] | 16B | Tile visibility bitmask |
| table1d2670[13] | dm2_v1_table_1d2670[13] | 26B | Tile property flags |
| table1d26c8[8] | dm2_v1_table_1d26c8[8] | 8B | Creature AI attack type |
| table1d26f8[4] | dm2_v1_table_1d26f8[4] | 4B | Direction bitmask shift |
| table1d2752[4] | dm2_v1_table_1d2752[4] | 8B | Viewport wall ordinals |
| table1d324c[44] | dm2_v1_table_1d324c[44] | 44B | Creature AI movement |
| table1d3278[16] | dm2_v1_table_1d3278[16] | 32B | Door/wall interaction |
| table1d3298[16] | dm2_v1_table_1d3298[16] | 16B | GUI element type map |
| table1d6fee[18] | dm2_v1_table_1d6fee[18] | 36B | Door visual ordinals |
| table1d6fdc[18] | dm2_v1_table_1d6fdc[18] | 18B | Door visual byte map |
| table1d6f9c[32] | dm2_v1_table_1d6f9c[32] | 64B | Wall ornament ordinals A |
| table1d6f7c[32] | dm2_v1_table_1d6f7c[32] | 32B | Wall ornament byte map A |
| table1d6f5c[32] | dm2_v1_table_1d6f5c[32] | 32B | Wall ornament byte map B |
| table1d6f2c[16] | dm2_v1_table_1d6f2c[16] | 32B | Item-to-wall mapping |
| table1d6f0b[14] | dm2_v1_table_1d6f0b[14] | 28B | Item ordinals for things |

Total: 42 tables, ~900 bytes of game data.

## Tests

22 tests: dir_dx, dir_dy, dir_opposite, music_map_level0,
music_map_sentinel, sound_freq_table, clock_sound_table,
dir_position_map, item_type_flags, bitmap_color_tables,
creature_skill_index, creature_type_class, door_ordinal,
dir_reverse, dir_i8_variants, neighbor_offsets,
perpendicular_offsets, door_visual_ordinals,
wall_ornament_ordinals, creature_ai_behavior,
tile_visibility, gui_element_map.

## Files

- `include/dm2_v1_data_tables_pc34_compat.h`
- `src/dm2/dm2_v1_data_tables_pc34_compat.c`
- `tests/test_dm2_v1_data_tables_pc34_compat.c`
