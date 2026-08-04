# Pass 1100 — DM2 data tables batch 3 + binary verification

## Source

skproject/SKWINSPX/src/v5/dm2data.cpp

## What was ported

65 additional const lookup tables from dm2data.cpp, bringing the total
to 107 tables (~4KB of ROM-like game data). Struct types defined:
dm2_s_bb, dm2_s_bbw, dm2_s_wwwb, dm2_s_www, dm2_s_4bytearray.

## Binary verification

Disassembled SKULL.EXE (DOS/4GW LE format, 522641 bytes). Extracted
the data segment (60817 bytes at base 0x70000). Discovered constant
address mapping: skproject_addr - 0x161000 = skull_exe_addr.

Verified 41/42 tables byte-exact against the original SKULL.EXE binary.
The music_map (tblMusicsMap) is not in the exe — it's loaded at runtime
from SONGLIST.DAT. The skproject hardcodes it; our port matches skproject.

## Tables added (batch 3)

| skproject symbol | Size | Purpose |
|---|---|---|
| table1d6702[16] | 16B | Creature distance curve |
| table1d6712[21] | 21B | Creature damage scaling |
| table1d672b[9] | 18B | Creature frame offsets |
| table1d673d[7] | 14B | Creature animation offsets |
| table1d281c[16] | 16B | Hero stat modifiers |
| table1d282c[16] | 16B | Hero stat init |
| table1d631a[60] | 60B | Game state bitmasks |
| table1d6356[263] | 263B | Game state extended |
| vsgame[120] | 120B | Default game state |
| table1d26d0[8] | 32B | Direction position order |
| table1d26f0[2] | 8B | Identity position order |
| table1d275a[32][2] | 64B | Viewport scatter offsets |
| table1d3ed5[10] | 40B | Enchantment action table |
| table1d3d23[62] | 496B | Item action dispatch |
| table1d3cd0[83] | 83B | Action type decode |
| table1d3ba0[76] | 304B | GUI action tree |
| table1d6afe[23] | 23B | Viewport cell range |
| table1d6a74[23] | 92B | Viewport cell adjacency |
| table1d6ad0[23][2] | 46B | Viewport cell coords |
| table1d6b43[23] | 23B | Cell mirror front |
| table1d6b5a[23] | 23B | Cell mirror back |
| table1d6b2c[23] | 23B | Cell mirror swap |
| table1d6b15[23] | 23B | Cell depth band |
| table1d6a54[4][4] | 16B | Direction sort A |
| table1d6a64[4][4] | 16B | Direction sort B |
| table1d6b71[5] | 5B | Depth scaling |
| table1d6efd[14] | 14B | Door ornament type |
| table1d6ee1[14][2] | 28B | Door ornament pair |
| table1d6ed3[14] | 14B | Door depth map |
| table1d6eb3[16] | 32B | Ornament position x/y |
| table1d6ea8[3] | 3B | Ornament repeat count |
| table1d6eab[4] | 4B | Ornament dir reverse |
| table1d6eaf[4] | 4B | Ornament dir offset |
| table1d6e68[4][8] | 64B | Creature move vectors |
| table1d6e51[23] | 23B | Creature cell visible |
| table1d6797[37] | 37B | Memory map struct |
| table1d6e41[16] | 16B | Creature attack direction |
| table1d6e35[12] | 12B | Creature visual slot |
| table1d6c70[16] | 32B | Stair/pit ordinals |
| table1d6c90[16] | 16B | Stair/pit flip |
| table1d6ca0[16] | 16B | Stair/pit byte A |
| table1d6cb0[16] | 16B | Stair/pit byte B |
| table1d6cc0[16] | 32B | Floor ordinals |
| table1d6c10[5] | 5B | Depth start offset |
| table1d6c19[5] | 5B | Depth entry count |
| table1d6c1e[23] | 23B | Cell to depth group |
| table1d6c35[23] | 23B | Cell column count |
| table1d6c4c[9] | 18B | Wall ordinals |
| table1d6c5e[9] | 9B | Wall byte map |
| table1d6c67[9] | 9B | Wall flip flag |
| table1d6c06[5] | 5B | Depth color ramp |
| table1d6c0b[5] | 5B | Depth fog start |
| table1d6a31[5] | 30B | Creature group config |
| table1d6e03[26][2] | 52B | 5x5 grid coords |
| table1d6de3[16][2] | 32B | Creature screen coords |
| table1d6dd3[16] | 16B | Creature visible flag |
| table1d69aa[6] | 6B | Projectile step count |
| table1d69a2[6] | 6B | Projectile step dir |
| table1d6998[5] | 10B | Projectile delta |
| table1d6984[20] | 20B | Projectile position |
| table1d6980[4] | 4B | Projectile face dir |
| table1d69b0[32] | 32B | Sound priority order |
| table1d69d0[4] | 4B | Sound dir mapping |
| table1d6b76[132] | 132B | Creature depth params |
| table1d6d3c[6] | 12B | Champion portrait ordinals |
| table1d6d48[6] | 6B | Champion portrait byte |
| table1d6d4e[6] | 6B | Champion portrait type |
| table1d6d54[6] | 6B | Champion portrait slot |
| table1d6ce0[92] | 92B | Creature render params |
| table1d6d5a[4][5] | 20B | Direction shuffle |
| table1d6d6e[4] | 8B | Champion icon pair |
| table1d6d76[2][4] | 8B | Movement delta A |
| table1d6d7e[2][4] | 8B | Movement delta B |
| table1d6d86[2] | 2B | Movement threshold |
| table1d6d88[25] | 25B | Identity permutation |
| table1d6da1[25] | 25B | Row reverse permutation |
| table1d6dba[25] | 25B | Column swap permutation |
| table1d67d9[7] | 7B | Creature sound flag |
| table1d67e0[5][6] | 30B | Creature sound matrix |
| table1d67fe[4] | 4B | Creature sound count |
| table1d292c[32] | 64B | Character code table |
| table1d70b4[17] | 17B | Creature timing init |

## Tests

33 tests covering all batches.

## Files

- `include/dm2_v1_data_tables_pc34_compat.h`
- `src/dm2/dm2_v1_data_tables_pc34_compat.c`
- `tests/test_dm2_v1_data_tables_pc34_compat.c`
