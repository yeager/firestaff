# FM Towns cross-game recovery coverage

Session 2026-08-07 comprehensive tally of Firestaff's byte-verified
FM Towns real-data recovery across DM1, CSB, and DM2.

## Discs used

| Game | Disc archive | Track 01 sha256 (via header strip) |
|---|---|---|
| DM1 | Dungeon Master (Japan) (En,Ja) (Rev 1).7z | c888470d.. (EDM.EXP) |
| CSB | Dungeon-Master-Chaos-Strikes-Back-Expansion-Set-1_FM-Towns_JA-EN.zip | 08cceb0c.. (CDATA/GRAPHICS.DAT) |
| DM2 | Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip | 634e7004.. (DATA/GRAPHICS.DAT) |

Full per-file hashes in `docs/fmtowns/all_games_real_data_hashes.json`.

## Cross-game byte-identical payloads

Data structures that appear byte-identical across two or three games
at per-game virtual addresses:

| Payload | Size | DM1 vaddr | CSB vaddr | DM2 vaddr | Games |
|---|---:|:---:|:---:|:---:|:---:|
| Menu font raster | 768 | (via asset 557) | 0x50f1a in GRAPHICS.DAT | 0x2f5a3 in GRAPHICS.DAT | 3 |
| CHAR geometry (7 words) | 14 | 0x26c8a | 0x2c94c | 0x1f6 | 3 |
| ICON geometry (4 words) | 8 | 0x26c68 | 0x2c938 | 0x1de | 3 |
| SPELL_COSTS | 32 | 0x24388 | 0x29f64 | 0x3bb0 | 3 |
| SPELL_MULT | 8 | 0x243a0 | 0x29f7c | 0x3bc8 | 3 |
| Phar Lap fs: 4-slot bridge | — | ✓ | ✓ | ✓ | 3 |
| Direct I/O port 0x04E9 | — | ✓ | ✓ | ✓ | 3 |
| OICON descriptor | 1344 | 0x224db | 0x27f77 | — | 2 (DM1+CSB) |
| PLAYER_COLOR | 8 | 0x291b8 | 0x2d164 | — | 2 |
| ICON_PAL | 6 | 0x28f44 | 0x2cd8a | — | 2 |
| DYNA_BUTTONS pool | ≥500 | 0x24194 | 0x29d50 | — | 2 |

## Per-game unique payloads

Data specific to each game that other games do NOT share:

- **DM1 EDM.EXP SYM1 table** (1174 entries) — unique to DM1.
- **CSB TMENU.EXP SYM1 table** (1724 entries) — unique to CSB launcher.
- **DM1 region registry** (23 blocks / 994 records) — unique DM1 menu layout.
- **DM1 LEVEL_SONGS** — DM1-specific CDDA mapping.
- **DM1 DOOR_PAL, DM_MUSIC** — DM1-specific chrome.
- **DM1 tmenu_input schema** (TMENU.EXP event queue) — DM1 launcher.
- **CSB CHTWJ.EXP, SWITCHTW.EXP, ANIMTW.EXP, UTILE.EXP** — CSB-specific binaries with independent code but no SYM1 stripped from release.
- **DM2 SKULL.EXP** — DM2 game code, no SYM1, no shared menu-render
  data (different action-label set).
- **DM2 GRAPHICS.DAT format** (extended v4 with 4-byte records) — DM2's
  own container variant.

## Shipping Firestaff modules

Every module below is source-locked with a real-data round-trip test:

### Byte-verified constants + accessors

- `dm1_v1_fmtowns_text_geometry` (CHAR/ICON macros — same for all games)
- `dm1_v1_fmtowns_icon_geometry` (ICON constants — same for all games)
- `dm1_v1_fmtowns_icon_category` (LOAD_ICON threshold table)
- `dm1_v1_fmtowns_oicon_descriptor` (1344-byte OICON records)
- `dm1_v1_fmtowns_dyna_buttons` (English action label pool)
- `dm1_v1_fmtowns_font_asset` (asset 557 identity)
- `dm1_v1_fmtowns_font_rasteriser` (6x128 raster decoder)
- `dm1_v1_fmtowns_menu_regions` (23-block region registry)
- `dm1_v1_fmtowns_menu_bss` (12 vaddrs + PLAYER schema)
- `dm1_v1_fmtowns_egb_rect` (region-tree walker)
- `dm1_v1_fmtowns_menu_render` (end-to-end composer)
- `dm1_v1_fmtowns_music_tables` (LEVEL_SONGS, SPELL_COSTS, etc.)
- `dm1_v1_fmtowns_edm_sym1` (1174 named symbols)
- `dm1_v1_fmtowns_snd_api` (36 SND_* function vaddrs)
- `dm1_v1_fmtowns_tmenu_input` (TMENU event schema)
- `dm1_v1_fmtowns_tbios_id` (8 TBIOS version fingerprints)
- `dm1_v1_fmtowns_pharlap_bridge` (4-slot layout)
- `dm1_v1_fmtowns_direct_io` (5 direct I/O ports)

### Cross-game aliases

- `csb_v1_fmtowns_oicon_descriptor` (aliases DM1's OICON via vaddr)
- `csb_v1_fmtowns_dyna_buttons` (aliases DM1's label pool via vaddr)
- `csb_v1_fmtowns_tmenu_sym1` (1724 CSB-specific launcher symbols)

### Shared cross-game infrastructure

- `fmtowns_pharlap_all_games` (11 binary profiles, 8-game direct I/O)
- `fmtowns_graphics_dat_format` (legacy/ext_v1/ext_v4 classifier)
- `fmtowns_geometry_all_games` (per-game CHAR/ICON vaddrs)
- `fmtowns_shared_tables_all_games` (SPELL/PLAYER/ICON per-game vaddrs)
- `fmtowns_font_raster_all_games` (per-game font raster locations)

### Documentation + evidence

- `docs/fmtowns/all_games_real_data_hashes.json`
- `docs/fmtowns/pharlap_call_sites.json` (502 call sites, 11 binaries)
- `docs/fmtowns/TOWNSOS_BIOS_INTEGRATION.md` (Tsugaru + FMT_F20.ROM)
- `docs/fmtowns/CROSS_GAME_INVENTORY.md` (earlier inventory)
- `docs/dm1/fmtowns_real_data_hashes.json` (DM1-specific hashes)
- `parity-evidence/dm1_fmtowns_region_table_full.md` (994 records)

## Completeness

- **DM1 FM Towns**: 100% of extractable byte-verified real data
  now shipped. Every runtime table, symbol, and I/O surface a
  hosted-BIOS integration would need is source-locked.
- **CSB FM Towns**: All cross-shared payloads (font, OICON,
  DYNA_BUTTONS, spell tables, ICON_PAL, PLAYER_COLOR) via alias
  modules. Independent CSB-only menu-render pipeline still open
  (CSB has its own region table; no SYM1 in game binary).
- **DM2 FM Towns**: Font raster + spell tables + geometry constants
  shared via cross-game modules. DM2's own menu-render pipeline
  and OICON/DYNA_BUTTONS require independent recovery (DM2 has
  different item/action set from DM1/CSB).

## What is intentionally NOT shipped

- **Extended-format GRAPHICS.DAT decoder** for CSB (0x8001) and DM2
  (0x8004). Format classifier identifies these but per-record
  decoding requires more RE work per game.
- **TownsOS BIOS runtime execution**. Requires Tsugaru integration
  or hosted TBIOS shim (see integration doc).
- **JDM.EXP Shift-JIS glyph table**. Out of scope per user directive.
- **Atari ST and Amiga variants**. Out of scope per user directive.
