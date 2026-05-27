# DM1 V1 Data File Loading — Source Lock

## Asset Status System (M12_AssetStatus)

src/shared/asset_status_m12.c is the primary mechanism for locating and
validating game data files. Called during startup to scan for original files.

### Game Versions and File Sets

#### DM1
| Version ID | Label | Required Files | Known MD5 |
|---|---|---|---|
| pc34-en | PC 3.4 English | GRAPHICS.DAT | fa6b1aa29e191418713bf2cda93d962e |
| pc34-multi | PC 3.4 Multilanguage | GRAPHICS.DAT | f934d97e43e1ba6e5159839acbcd0611 |
| st12-en | Atari ST 1.2 English | GRAPHICS.DAT | 9ce2eaf7a9e78620e3f17594437caffa |

#### CSB
| Version ID | Label | Required Files |
|---|---|---|
| pc34-en | PC 3.4 English | GRAPHICS.DAT, CSBGRAPH.DAT |
| st20-21-en | Atari ST 2.0/2.1 English | GRAPHICS.DAT, CSBGRAPH.DAT |
| amiga35-en | Amiga 3.5 English | GRAPHICS.DAT, CSBGRAPH.DAT |
| amiga35-multi | Amiga 3.5 Multilanguage | GRAPHICS.DAT, CSBGRAPH.DAT |

#### DM2
| Version ID | Label | Required Files |
|---|---|---|
| pc-en | PC English | GRAPHICS.DAT, DM2GRAPHICS.DAT, DM2DUNGEON.DAT |
| pc-fr | PC French | GRAPHICS.DAT, DM2GRAPHICS.DAT, DM2DUNGEON.DAT |
| pc-jewel | PC German/English JewelCase | GRAPHICS.DAT, DM2GRAPHICS.DAT, DM2DUNGEON.DAT |

#### Nexus
| Version ID | Label | Required Files |
|---|---|---|
| nexus-saturn-jp | Saturn JP extracted | DM.BIN, Dungeon-Master-Nexus_SEGA-Saturn_JA.zip |
| nexus | Nexus original Saturn JP | DM.BIN, Dungeon-Master-Nexus_SEGA-Saturn_JA.zip |
| nexus2 | Nexus V2 upscaled | DM.BIN, Dungeon-Master-Nexus_SEGA-Saturn_JA.zip |

### Scan Search Roots

M12_AssetStatus_Scan() uses 3 search roots:
1. FIRESTAFF_DATA environment variable
2. Platform-specific user data dir (%APPDATA%/Firestaff on Win,
   ~/Library/Application Support/Firestaff on macOS, ~/.local/share/firestaff on Linux)
3. Hardcoded fallback directories for each game

### Original Candidate Names

Files considered as original candidates:
GRAPHICS.DAT, DUNGEON.DAT, DUNGEONF.DAT, DUNGEONG.DAT,
CSBGRAPH.DAT, CSB.DAT, DM2GRAPHICS.DAT, DM2DUNGEON.DAT,
SKULLKEEP.GFX, Dungeon-Master-Nexus_SEGA-Saturn_JA.zip

## TITLE File Discovery (main_loop_m11.c m11_find_title_dat_for_intro)

Priority order for locating TITLE intro file:
1. FIRESTAFF_TITLE_DAT environment variable (validated with V1_Title_IsCanonicalPc34Title)
2. Sibling of matched DM1 asset (parent of GRAPHICS.DAT/DUNGEON.DAT, then grandparent
   for DungeonMasterPC34/DATA/ layout where TITLE lives beside game dir)
3. Suffix search in data dir
4. Home-tree OpenClaw original-data anchors:
   ~/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/TITLE etc.

Suffixes checked:
TITLE, TITLE.DAT,
DungeonMasterPC34/TITLE, DungeonMasterPC34/TITLE.DAT,
DungeonMasterPC34Multilingual/TITLE, etc.,
dm-pc34/DungeonMasterPC34/TITLE, etc.

## GRAPHICS.DAT Loading (dm1_v1_graphics_loader_pc34_compat.c)

m11_gfx_open_dat():
1. Open file (fopen path, rb)
2. Read bitmap count (2-byte uint16_t header)
3. Read per-bitmap headers: width (u16), height (u16), compressed_size (u32), offset
4. Compressed data read on demand per-bitmap via m11_gfx_load_bitmap()

LZW decompression: 9-bit initial code width, grows to 12-bit max.
ReDMCSB refills dictionary on code-width change (not just on clear code).
File handle stored in M11_GFX_LoaderState.dat_file. Lazy decompression per asset request.

## DUNGEON.DAT Loading (dm1_v1_dungeon_loader_pc34_compat.c)

m11_dl_load_from_file():
1. Read DungeonHeader (level_count, map_data_offset, per-level metadata)
2. Per-level tile maps: column-major storage (x * height + y indexing)
3. Tile layout follows ReDMCSB DUNGEON.C F0151 column-major format.

Loaded lazily on dungeon entry, not at startup.

## Data Directory Creation (firestaff_startup.c fs_startup_ensure_data_dirs)

Creates subdirectories if missing: data/dm1/, data/csb/, data/dm2/,
data/dm1-multilingual/, data/nexus/
Also writes data/README.txt with placement instructions.

## Path Resolution Priority (FSP_ResolveDataDir in fs_portable_compat.c)

1. Explicit requestedDir argument (if non-empty)
2. FIRESTAFF_DATA environment variable
3. Legacy ~/.firestaff/data on POSIX (only if directory already exists)
4. <user-data-dir>/data
5. Current directory .

## ReDMCSB Reference

Canonical source for data file loading:
- Dungeon.C / Dungeon2.C — DUNGEON.DAT structure and loading
- Graphics.C — GRAPHICS.DAT format, LZW decompression
- Title.C — TITLE.DAT intro sequence
- LoadGame.C / SaveGame.C — save file format
