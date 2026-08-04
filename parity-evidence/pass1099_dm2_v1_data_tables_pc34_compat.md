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

Total: 17 tables, 377 bytes of game data.

## Tests

13 tests: dir_dx, dir_dy, dir_opposite, music_map_level0,
music_map_sentinel, sound_freq_table, clock_sound_table,
dir_position_map, item_type_flags, bitmap_color_tables,
creature_skill_index, creature_type_class, door_ordinal.

## Files

- `include/dm2_v1_data_tables_pc34_compat.h`
- `src/dm2/dm2_v1_data_tables_pc34_compat.c`
- `tests/test_dm2_v1_data_tables_pc34_compat.c`
