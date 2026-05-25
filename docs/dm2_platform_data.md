# DM2 V1 — Platform/Build Audit: Data File Loading

## Data File Paths and Formats

### File Layout in skproject

```
skproject/SKULLWIN/Data/          — Windows build data (Allegro)
  DUNGEON.DAT                      — DM2 dungeon (39 KB)
  GRAPHICS.DAT                     — DM2 graphics (~8.6 MB)
  MODLIST.DAT, SONGLIST.DAT        — music track lists
  *.hmp.mid                        — MIDI tracks (00.hmp.mid .. 1c.hmp.mid)
  music.bin                        — embedded music data

skproject/SKWIN/data/              — SKWIN variant data
  DUNGEON.DAT, GRAPHICS.dat        — base PC versions
  DUNGEON_MOD.DAT, DUNGEON_SHOP.DAT, DUNGEON_TEST.DAT — special levels
  SKSAVE*.DAT / SKSAVE*.BAK        — save game slots

skproject/SKWIN/data_dm2_dm/       — DM2 dungeon master data
  graphics.dat                     — DM2 PC English graphics

skproject/SKWIN/data_dm2_tq/      — Theron's Quest variant
  DUNGEON.DAT, graphics.dat
  pcdossk_theronsquest.DAT
  dm2tq_aktuba.dat

skproject/SKWIN/data_dm2_sk/       — Skullkeep variant
  DUNGEON.DAT, graphics.dat
  GRAPHICS_PC9821_v4.0.1_9821J13.dat — Japanese PC-9821 variant
  pcdossk_skullkeep.dat

skproject/SKWIN/data_dm2_beta/    — Beta variant
  DUNGEON.DAT, graphics.dat
  pcskb_dungeon_sk_beta.dat

skproject/SKWIN/data_dm2_demo/    — Demo variant
  DUNGEON.DAT, GRAPHICS.dat, pcsk_demo.dat

skproject/SKWIN/data_dm2_csb/     — CSB cross variant
  GRAPHICS.dat

skproject/SKWIN/data_csb/         — CSB data (different dungeon format)
skproject/SKWIN/data_dm1_dm/      — DM1 data (33 KB dungeon, 363 KB graphics)
skproject/SKWIN/data_dm1_tq/      — DM1 Theron's Quest variant
```

### Firestaff Data Loading

`dm2_v1_game.c` uses hash-based file discovery via `asset_find_by_md5_list()`:

```c
/* Known DM2 DUNGEON.DAT MD5 hashes */
static const char *const dm2_dungeon_hashes[] = {
    "6caccd7875009e82fe2e28e7f6d6adc0",  /* DM2 PC English DUNGEON.DAT */
    NULL
};

/* Known DM2 GRAPHICS.DAT MD5 hashes */
static const char *const dm2_graphics_hashes[] = {
    "25247ede4dabb6a71e5dabdfbcd5907d",  /* PC English */
    "b4d733576ea60c41737f79f212faf528",  /* PC French */
    "e52ab5e01715042b16a4dcff02052e5d",  /* PC German/English JewelCase */
    NULL
};
```

### Dungeon Format (DM2 vs DM1)

DM2 dungeon format (from `dm2_v1_dungeon_loader.c`, SKULL.ASM source):

```
Header:
  uint16_t level_count          — number of levels
Level entry (per level, 8 bytes):
  uint8_t  level_type           — 0=indoor, 1=outdoor (DM2 extension)
  uint8_t  level_width
  uint8_t  level_height
  uint8_t  (padding)
  uint16_t offset_low           — offset (lower 16 bits)
  uint16_t offset_high          — offset (upper 16 bits)
Per-square (2 bytes):
  uint16_t square_type & 0x1F   — 5-bit tile type
```

Key differences from DM1:
- **Level type byte**: DM1 has none; DM2 marks outdoor vs indoor
- **39 KB vs 33 KB**: DM2 has more levels and wider maps
- **Outdoor levels**: DM2 outdoor maps rendered by `dm2_v1_outdoor_renderer.c`
- **Max levels**: DM2_V1_MAX_LEVELS (defined in header, typically > DM1)

### GRAPHICS.DAT Size Comparison

| Game      | GRAPHICS.DAT  |
|-----------|---------------|
| DM1       | ~363 KB       |
| DM2       | ~8.6 MB       |
| DM2 (PC98)| ~12 MB (PC-9821 multilanguage) |

DM2's graphics are ~24× larger than DM1's, reflecting outdoor artwork,
animated weather, building sprites, day/night palettes.

### Save Game Format

SKSAVE*.DAT — binary format with header:
- 4 bytes: save slot magic
- party state (position, level, direction)
- champion states (stats, inventory, skills)
- time_of_day, gold, reputation
- outdoor/dungeon flag
- companion list

Load code in `dm2_v1_save_load.c` (SKULL.ASM sourced).

### Music Data

- HMP format (Herbert's MIDI Packed) — used by both Allegro and MCI
- Music path: `./DATA/%02x.hmp.mid` (compiled into SKULLWIN as `00.hmp.mid`..`1c.hmp.mid`)
- `music.bin` — compiled track list for SKULLWIN

### GDAT System

SKWIN/SKULLWIN use a `c_gdatfile` class for structured data access:
- Categories: spells, creatures, items, AI, text (localized)
- Indexed by `(category, index, field)` triplet
- Supports localized text via `s_textLangSel[category][index][field]`
- `DM2_EXTENDED_MODE=1` adds `s_imageLangSel` for localized UI graphics
