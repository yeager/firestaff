# DM2 V1 Phase 8 — Canonical Asset Manifest
**Pass:** H2312
**Date:** 2026-05-26
**Schema:** `firestaff.dm2_v1.canonical_asset_manifest.v1`
**Data root:** `/Users/bosse/.firestaff/data/dm2`

## Summary
- Total tracked assets: 2
- Present: 2  
- Absent: 0  
- Hash mismatch: 0  
- Size mismatch: 0  
- Forbidden files found: 0  

## Asset Table

| ID | Role | Platform | Expected Size | SHA256 (first 16) | Status |
|----|------|---------|----------------|---------------------|--------|
| `pc34_dungeon` | pair | PC English | 39437 | `cfadfd40f7a0b84c...` | **present** |
| `pc34_graphics` | pair | PC English | 8639757 | `c387ee42ad1b340b...` | **present** |

## Source Anchors
### `skull_graphics_dat_open`
- Source: SKULL.ASM
- File: `SKULL.ASM`  
- Lines: T547-T551
- Claim: SKULL.ASM T547 opens GRAPHICS.DAT for GDAT resource loading; mode 'rb'.
- Needles: `GRAPHICS.DAT`, `GDAT_LoadGraphics`, `file open mode`

### `skull_dungeon_dat_load`
- Source: SKULL.ASM
- File: `SKULL.ASM`  
- Lines: T560-T565
- Claim: SKULL.ASM T560 parses DUNGEON.DAT header — uint16 level_count at offset 0, then level descriptors.
- Needles: `DUNGEON.DAT`, `DUNGEON_Load`, `level_count`, `multi-level`

### `skwin_gdat_category_limit`
- Source: skproject/SkGlobal.h
- File: `N/A`  
- Lines: 705-716
- Claim: skproject SkGlobal.h defines GDAT_CATEGORY_LIMIT and creature/AI constants used in DM2 GDAT loading.
- Needles: `GDAT_CATEGORY_LIMIT`, `CREATURE_AI_TAB_SIZE`, `MAXAI`, `MAXSPELL`

### `skwin_extended_load_ai_definition`
- Source: skproject/SKWIN/SkWinCore.cpp
- File: `N/A`  
- Lines: 415-437, 27038-27096
- Claim: SkWinCore.cpp EXTENDED_LOAD_AI_DEFINITION maps GDAT category indices to AI names.
- Needles: `EXTENDED_LOAD_AI_DEFINITION`, `getAIName`, `GDAT_CATEGORY`

### `dm2_dungeon_multi_level_format`
- Source: dm2_v1_dungeon_loader.c
- File: `src/dm2/dm2_v1_dungeon_loader.c`  
- Lines: 1-60
- Claim: DM2 DUNGEON.DAT supports 30 levels with outdoor/indoor/building types; offsets point to per-level tile data.
- Needles: `level_count`, `level_offsets`, `DM2_LEVEL_OUTDOOR`, `DM2_LEVEL_INDOOR`

### `skull_gdat_resource_loading`
- Source: SKULL.ASM
- File: `SKULL.ASM`  
- Lines: T600-T620
- Claim: SKULL.ASM T600-T620 loads GDAT resources: category index → frame count → GDG2 decompress.
- Needles: `GDAT_Load`, `category_index`, `frame_count`, `GDG2_format`

