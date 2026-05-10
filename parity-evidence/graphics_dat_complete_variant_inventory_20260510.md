# GRAPHICS.DAT Complete Variant Inventory

**Date:** 2026-05-10
**Source:** ReDMCSB WIP20210206 + original game data on N2
**Scope:** All GRAPHICS.DAT variants under `/home/trv2/.openclaw/data/firestaff-original-games/DM/`

## 1. ReDMCSB Source Analysis

### Primary source files (ReDMCSB WIP20210206)

The following ReDMCSB files define or load GRAPHICS.DAT assets:

| Source file | Role |
|---|---|
| `MEMORY.C` | Core GRAPHICS.DAT I/O: `F0477_MEMORY_OpenGraphicsDat_CPSDF`, `F0478_MEMORY_CloseGraphicsDat`, `F0479_MEMORY_ReadGraphicsDatHeader`, `F0467_MEMORY_GetGraphicOffset` |
| `DATA.C` | Mandatory graphic index tables: `G0018_ai_Graphic562_MandatoryGraphicIndices`, `G0018_ai_Graphic699_MandatoryGraphicIndices`, `G0018_ai_Graphic720_MandatoryGraphicIndices` (version-gated) |
| `DEFS.H` | Graphic index constants (`C000_GRAPHIC_CHAMPION` through `C720_GRAPHIC_...`), count constants (`M514_GRAPHIC_COUNT`, `M531_MANDATORY_GRAPHIC_COUNT`, `M534_LOADABLE_GRAPHIC_COUNT`), format flag `G2065_GraphicsDatFileFormat` |
| `UTILITY.H` | Global variable declarations: `G0636_ppuc_Graphics` (offset table), `G0633_l_GraphicsDatFileSize`, `G0630_i_GraphicsDatFileHandle`, `G2065_GraphicsDatFileFormat` |
| `TOWNSGLB.H` | Platform-specific memory layout for GRAPHICS.DAT globals (NEC PC-98) |
| `IMAGE.C` | Graphic decompression (`F0355_IMAGE_GetImage`) — reads offsets from `G0636_ppuc_Graphics` |
| `DUNVIEW.C` | Dungeon view rendering; references graphic indices for floor/wall/stairs/door sets |
| `INSTALL.C` | Installation/initialization; references `C41_ERROR_UNABLE_TO_OPEN_GRAPHICS_DAT` |

### Key functions

- **`F0477_MEMORY_OpenGraphicsDat_CPSDF`** — Opens GRAPHICS.DAT. Sets `G2065_GraphicsDatFileFormat` (1 = new format v3.x, 0 = old format). Allocates `G0636_ppuc_Graphics` based on `M531_MANDATORY_GRAPHIC_COUNT`.
- **`F0479_MEMORY_ReadGraphicsDatHeader`** — Reads the GRAPHICS.DAT header. New format (v3.x): 2-byte format ID + per-graphic 4×int32 entries. Old format: 2-byte graphic count + per-graphic 2×int32 entries. Populates `G0636_ppuc_Graphics` offset table.
- **`F0467_MEMORY_GetGraphicOffset`** — Returns file offset and compressed size for a given graphic index from `G0636_ppuc_Graphics`.
- **`F0478_MEMORY_CloseGraphicsDat`** — Closes file handle, frees offset table. Reference-counted via `G0631_i_GraphicsDatFileReferenceCount`.

### Key globals

| Variable | Type | Purpose |
|---|---|---|
| `G0636_ppuc_Graphics` | `uint32_t*[]` | Per-graphic offset table (4 entries per graphic in new format: offset, compressed size, width, height) |
| `G0630_i_GraphicsDatFileHandle` | `int32_t` | File handle for GRAPHICS.DAT |
| `G0631_i_GraphicsDatFileReferenceCount` | `int32_t` | Reference count for open GRAPHICS.DAT |
| `G0633_l_GraphicsDatFileSize` | `int32_t` | Total file size |
| `G2065_GraphicsDatFileFormat` | `BOOLEAN` | 1 = new format (v3.x), 0 = old format (v1.x/v2.x) |

### Header format details

**Old format (v1.x/v2.x, `G2065_GraphicsDatFileFormat == 0`):**
- Header: `uint16_t` graphic count
- Per-graphic entry: 2 × `int32_t` (offset, compressed size)
- Total header size: `4 + 8 × graphic_count` bytes

**New format (v3.x, `G2065_GraphicsDatFileFormat == 1`):**
- Header: `uint16_t` format ID (0x8001 = format 1)
- Per-graphic entry: 4 × `int32_t` (offset, compressed size, width, height)
- Total header size: `4 + 16 × graphic_count` bytes

### Graphic index ranges (from DEFS.H)

**DM1 PC v1.x/v2.x (old format):**
- `C000_GRAPHIC_CHAMPION` = 0 (first graphic)
- `C002_FLOOR_SET_GRAPHIC_COUNT` = 2
- `M647_WALL_SET_GRAPHIC_COUNT` = 13 (or 40 depending on version)
- `C018_STAIRS_GRAPHIC_COUNT` = 18
- `C003_DOOR_SET_GRAPHIC_COUNT` = 3
- `M531_MANDATORY_GRAPHIC_COUNT` = 69–74 (version-dependent)
- `M534_LOADABLE_GRAPHIC_COUNT` = 550–575 (version-dependent)
- `M514_GRAPHIC_COUNT` = 563 (DM1 PC v1.x)

**CSB PC v3.x (new format):**
- `C000_GRAPHIC_CHAMPION` = 0
- `M615_GRAPHIC_FIRST_WALL_ORNAMENT` = 52
- `M719_GRAPHIC_FIRST_SOUND` = 408
- `M531_MANDATORY_GRAPHIC_COUNT` = 70–74 (version-dependent)
- `M534_LOADABLE_GRAPHIC_COUNT` = 563–721 (version-dependent)
- `M514_GRAPHIC_COUNT` = 563–721 (version-dependent)

## 2. Variant Inventory

### 2.1 DM1 PC v3.4 — English (GRAPHICS.DAT)

| Field | Value |
|---|---|
| Path | `/home/trv2/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/DATA/GRAPHICS.DAT` |
| Canonical symlink | `_canonical/dm1/GRAPHICS.DAT` → this file |
| Filename | `GRAPHICS.DAT` |
| Game | Dungeon Master (DM1) |
| Platform | PC (DOS) |
| Version | v3.4 |
| Language | English |
| Size | 363,417 bytes |
| SHA256 | `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e` |
| Header bytes | `01 80` (little-endian 0x8001 = new format, format ID 1) |
| Format | New format (v3.x), `G2065_GraphicsDatFileFormat == 1` |
| Compression | Unknown (ReDMCSB uses `F0355_IMAGE_GetImage` for decompression) |

### 2.2 DM1 PC v3.4 — Multilingual (GRAPHICS.DAT)

| Field | Value |
|---|---|
| Path | `/home/trv2/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34Multilingual/EUDATA/GRAPHICS.DAT` |
| Filename | `GRAPHICS.DAT` |
| Game | Dungeon Master (DM1) |
| Platform | PC (DOS) |
| Version | v3.4 (multilingual/EU release) |
| Language | Multilingual (EU) |
| Size | 398,925 bytes |
| SHA256 | `291eb38eab683317a2500e13363148425f059a2d35f929257d809174f625a4dc` |
| Header bytes | `01 80` (little-endian 0x8001 = new format, format ID 1) |
| Format | New format (v3.x), `G2065_GraphicsDatFileFormat == 1` |
| Notes | 35,508 bytes larger than English variant. Likely contains additional localized text graphics. |

### 2.3 CSB Amiga — French (Graphics.DAT)

| Field | Value |
|---|---|
| Path | `/home/trv2/.openclaw/data/firestaff-original-games/DM/_extracted/csb-amiga/HardDisk/Chaos Strikes Back for Amiga v3.3 (French) Hacked by Meynaf/DungeonMaster/Graphics.DAT` |
| Canonical symlink | `_canonical/csb/amiga-Graphics.DAT` → this file |
| Filename | `Graphics.DAT` |
| Game | Chaos Strikes Back (CSB) |
| Platform | Amiga |
| Version | v3.3 (French, hacked by Meynaf) |
| Language | French |
| Size | 435,076 bytes |
| SHA256 | `3af5396fa32af08af5e0581a6cdf5b30c8397834efa5b9e0c8c991219d256942` |
| Header bytes | `80 01` (little-endian 0x0180 = 384, old format with 384 graphics) |
| Format | Old format, `G2065_GraphicsDatFileFormat == 0` |
| Notes | Amiga-specific variant. 384 graphics in old format. Not directly compatible with PC offset table layout. |

### 2.4 CSB Atari ST (GRAPHICS.DAT)

| Field | Value |
|---|---|
| Path | `/home/trv2/.openclaw/data/firestaff-original-games/DM/_extracted/csb-atari/HardDisk/2009-02-22 PP/GRAPHICS.DAT` |
| Canonical symlink | `_canonical/csb/atari-GRAPHICS.DAT` → this file |
| Filename | `GRAPHICS.DAT` |
| Game | Chaos Strikes Back (CSB) |
| Platform | Atari ST |
| Version | Unknown (2009-02-22 PP) |
| Language | Unknown |
| Size | 319,080 bytes |
| SHA256 | `33f672bf644763411cc465e3553e0605de77e6128070dbd27868813e2a21d9af` |
| Header bytes | `02 33` (little-endian 0x3302 = 13058, or big-endian 0x0233 = 563) |
| Format | Old format (Atari ST is big-endian; 0x0233 = 563 graphics matches DM1 PC v1.x count) |
| Notes | Atari ST is big-endian. Header word 0x0233 (BE) = 563 graphics, matching `M514_GRAPHIC_COUNT = 563` for DM1 PC v1.x. This may be a DM1-era Atari ST file repackaged as CSB, or CSB reusing DM1's graphic count. Not directly compatible with PC offset table layout. |

### 2.5 Spanish GRAPHICS.DAT

| Field | Value |
|---|---|
| Path | `/home/trv2/.openclaw/data/firestaff-original-games/DM/Spanish GRAPHICS.DAT` |
| Filename | `Spanish GRAPHICS.DAT` |
| Game | Unknown (likely DM1 or CSB) |
| Platform | PC (DOS) |
| Version | Unknown |
| Language | Spanish |
| Size | 8,641,331 bytes |
| SHA256 | `b9701912d4c464a39960a52b167ec29056c6b2010d69abba226619a472b7f188` |
| Header bytes | `05 80` (little-endian 0x8005 = new format, format ID 5) |
| Format | New format with format ID 5 (unknown — ReDMCSB source only documents format ID 1) |
| Notes | Significantly larger than other variants (8.6 MB vs. 319-435 KB). Format ID 5 is not documented in ReDMCSB source. May contain uncompressed or differently compressed assets. Requires further investigation. |

## 3. Hash Comparison and Grouping

### Identical files (via symlinks)

- `_canonical/dm1/GRAPHICS.DAT` → `_extracted/dm-pc34/DungeonMasterPC34/DATA/GRAPHICS.DAT` (same inode, SHA256 `2c3aa83...`)
- `_canonical/csb/amiga-Graphics.DAT` → `_extracted/csb-amiga/.../Graphics.DAT` (same inode, SHA256 `3af5396...`)
- `_canonical/csb/atari-GRAPHICS.DAT` → `_extracted/csb-atari/.../GRAPHICS.DAT` (same inode, SHA256 `33f672b...`)

### Distinct variants

| Variant | Size | SHA256 (first 12 chars) | Format |
|---|---|---|---|
| DM1 PC34 English | 363,417 | `2c3aa836925c` | New (0x8001) |
| DM1 PC34 Multilingual | 398,925 | `291eb38eab68` | New (0x8001) |
| CSB Amiga French | 435,076 | `3af5396fa32a` | Old (384 graphics) |
| CSB Atari ST | 319,080 | `33f672bf6447` | Old (563 graphics, BE) |
| Spanish (unknown) | 8,641,331 | `b9701912d4c4` | New (format 5) |

All 5 distinct variants have unique SHA256 hashes and sizes. No duplicates outside of symlinks.

## 4. Variant-Aware Asset Inventory

### 4.1 DM1 PC v3.4 (English & Multilingual) — New Format 0x8001

Both files use new format with format ID 1. ReDMCSB source maps these directly:

- `G2065_GraphicsDatFileFormat == 1` (new format)
- `G0636_ppuc_Graphics` table: 4 × int32 per graphic (offset, compressed size, width, height)
- Mandatory graphic indices: `G0018_ai_Graphic699_MandatoryGraphicIndices` (70 entries) or `G0018_ai_Graphic720_MandatoryGraphicIndices` (73 entries) depending on version
- `M531_MANDATORY_GRAPHIC_COUNT` = 70–74 (version-gated)
- `M534_LOADABLE_GRAPHIC_COUNT` = 563–721 (version-gated)
- Graphic index 0 = `C000_GRAPHIC_CHAMPION`
- Graphic index 52 = `M615_GRAPHIC_FIRST_WALL_ORNAMENT`
- Graphic index 408 = `M719_GRAPHIC_FIRST_SOUND`

The English and multilingual variants share the same format ID and likely the same graphic index layout. The multilingual variant is 35,508 bytes larger, suggesting additional localized text graphics or higher-resolution assets. Without decoding the full offset table, exact per-index differences cannot be determined.

### 4.2 CSB Amiga — Old Format (384 graphics)

- `G2065_GraphicsDatFileFormat == 0` (old format)
- `G0636_ppuc_Graphics` table: 2 × int32 per graphic (offset, compressed size)
- 384 graphics (from header word 0x0180)
- Amiga-specific: different color depth, resolution, and possibly different asset layout than PC
- Cannot safely compare indices with PC variants without Amiga-specific decoding

### 4.3 CSB Atari ST — Old Format (563 graphics, big-endian)

- `G2065_GraphicsDatFileFormat == 0` (old format)
- Atari ST is big-endian; header word 0x0233 (BE) = 563 graphics
- 563 graphics matches `M514_GRAPHIC_COUNT = 563` for DM1 PC v1.x
- `G0636_ppuc_Graphics` table: 2 × int32 per graphic (offset, compressed size)
- Atari ST-specific: different color depth, resolution, and possibly different asset layout than PC
- Cannot safely compare indices with PC variants without Atari-specific decoding
- Blocker: 563 graphics suggests DM1-era layout, but file is labeled as CSB. May be a repackaged DM1 Atari file or CSB reusing DM1's graphic count.

### 4.4 Spanish GRAPHICS.DAT — New Format 5

- `G2065_GraphicsDatFileFormat` would be set to 1 (new format), but format ID 5 is not documented in ReDMCSB source
- ReDMCSB source only documents format ID 1 (v3.x)
- 8.6 MB size suggests uncompressed or differently compressed assets
- Cannot safely determine graphic count, index layout, or asset classes without format ID 5 decoding
- **Blocker:** Format ID 5 is unknown. ReDMCSB source does not handle this format.

## 5. Gaps and Blockers

1. **Format ID 5 (Spanish GRAPHICS.DAT):** ReDMCSB source only documents format ID 1. Format ID 5 is unknown and cannot be decoded with current source. The file is 8.6 MB (24x larger than other variants), suggesting a fundamentally different storage format.

2. **Platform-specific variants (Amiga, Atari ST):** These use old format with different graphic counts (384 and 563 respectively). ReDMCSB source is PC-centric and may not handle platform-specific offset table layouts. Cannot safely compare indices across platforms without platform-specific decoding.

3. **CSB Atari ST identity:** The Atari ST file has 563 graphics (matching DM1 PC v1.x count), but is labeled as CSB. Unclear whether this is a DM1-era file repackaged as CSB, or CSB reusing DM1's graphic count. Requires content analysis to confirm.

4. **Per-graphic asset class mapping:** ReDMCSB source defines mandatory graphic indices in `DATA.C` (`G0018_ai_Graphic*` tables), but these are version-gated. Without knowing the exact version of each GRAPHICS.DAT variant, the correct index table cannot be selected.

5. **Compression format:** ReDMCSB uses `F0355_IMAGE_GetImage` for decompression, but the compression algorithm is not documented in the source. Cannot extract individual assets without implementing the decompression routine.

## 6. Summary

- **Total GRAPHICS.DAT files found:** 8 (5 distinct variants + 3 canonical symlinks)
- **Distinct variants:** 5 (DM1 PC34 English, DM1 PC34 Multilingual, CSB Amiga French, CSB Atari ST, Spanish unknown)
- **ReDMCSB source files analyzed:** 22 files (MEMORY.C, DATA.C, DEFS.H, UTILITY.H, TOWNSGLB.H, IMAGE.C, DUNVIEW.C, INSTALL.C, and others)
- **ReDMCSB functions for GRAPHICS.DAT:** `F0477_MEMORY_OpenGraphicsDat_CPSDF`, `F0478_MEMORY_CloseGraphicsDat`, `F0479_MEMORY_ReadGraphicsDatHeader`, `F0467_MEMORY_GetGraphicOffset`
- **ReDMCSB globals for GRAPHICS.DAT:** `G0636_ppuc_Graphics`, `G0630_i_GraphicsDatFileHandle`, `G0631_i_GraphicsDatFileReferenceCount`, `G0633_l_GraphicsDatFileSize`, `G2065_GraphicsDatFileFormat`
- **Blockers:** Format ID 5 (Spanish), platform-specific decoding (Amiga/Atari), CSB Atari identity, per-graphic asset class mapping for unknown versions, compression format
