# DM2 V1 Dungeon Design — Source-Locked

## Sources

- SKULL.ASM (522,128 lines IDA disassembly, sha256: a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099)
- skproject (github.com/gbsphenx/skproject) HEAD a962896
- docs/dm2-v1-dungeon-audit/dm2_dungeon.md (existing audit)
- SKWIN/SkGlobal.h, SkWinCore.cpp
- SKWIN/knowledge/SKWin.GDAT2.InternalCodes.txt

---

## DM2 Dungeon Format Overview

DM2 uses the same overall DUNGEON.DAT structure as DM1:
- Header with map count and offsets
- Map data (tile grids, 16-byte map descriptors)
- Thing data (creatures, items, triggers)

**File size comparison:**
| Variant | DUNGEON.DAT bytes |
|---------|------------------|
| DM1 PC 3.4 | 33,357 |
| DM2 DOS EN | 39,437 |
| DM2 Sega CD | 37,957 |

DM2 is ~18% larger, indicating extended fields and additional level types.

---

## Key Structural Differences from DM1

### 1. Extended Creature AI Table
DM2 uses 64 creature AI slots vs DM1's 42.

Source: skproject/SkGlobal.h:
- CREATURE_AI_TAB_SIZE 64 (DM2 extended)
- CREATURE_AI_TAB_SIZE 42 (original/DM1/CSB)

MAXAI also increases from 62 to 255 in extended mode.

### 2. Extended GDAT Category System
DM2 extends GDAT categories from 0x1D (29) to 0xF0 (240).

Source: skproject/SkGlobal.h:636 (DM2 extended) vs 638 (original)

New DM2 categories (from SkWinCore.cpp):
- GDAT_CATEGORY_SPELL_DEF - spell definitions loaded from GDAT
- GDAT_CATEGORY_CREATURE_AI - per-creature AI behaviors
- GDAT_CATEGORY_CREATURES - creature stats/drops
- GDAT_CATEGORY_WEAPONS - weapon data with projectile flags
- GDAT_CATEGORY_DOORS (0x0E) - door properties
- GDAT_CATEGORY_TELEPORTERS (0x18) - teleporter squares
- GDAT_CATEGORY_CHAMPIONS (0x16) - champion NPC data

### 3. FTL Compression Signature
DM2 uses the same FTL signature (0x8104) as DM1 for dungeon data.

Source: docs/dm2-v1-dungeon-audit/dm2_dungeon.md

---

## New Level Types (Outdoor System)

DM2 introduces three distinct level types (dm2_v1_dungeon_loader.h):

1. **DM2_LEVEL_OUTDOOR (0)**
   - Sky texture (sky_texture_index field)
   - Ground texture
   - Tree density
   - Building count
   - Weather zones (rain, fog, storm)

2. **DM2_LEVEL_INDOOR (1)**
   - Standard DM1-style first-person dungeon
   - Same wall/floor/orrnate system

3. **DM2_LEVEL_BUILDING (2)**
   - Multi-floor buildings within outdoor areas
   - Transition between outdoor and indoor rendering

Source: include/dm2_v1_dungeon_loader.h, include/dm2_v1_outdoor_renderer.h

---

## Map Descriptor Structure

DM2 map descriptors follow the same 16-byte structure as DM1 (DEFS.H MAP descriptor).
However, DM2 dungeons support more levels/maps per dungeon (up to 30 levels).

Source: docs/dm2-v1-dungeon-audit/dm2_dungeon.md

---

## New Wall/Floor Ornate Features

From SKWin.GDAT2.InternalCodes.txt:

### Animated Ornnates
- 0D 00 00: Number of cycled animated frames
- 0D 00 00 (TXT): Sequence of animated frames (up to 10)
- Activation sound fires at start of each cycle

### Ladder Detection
- 11 00 00: 1 = ladder going up, absent = ladder going down
- Source: SkWinCore.cpp queries GDAT_WALL_ORNATE__IS_LADDER_UP

### Animated Mirrored Door (DM2 new)
- Field 20 00 00: Animated mirrored door (like DM1 force field)
- Note: unused in standard DM2 dungeons

---

## New Door Type: Dragon Door (0x0A)

DM2 adds a second clan door type specifically for the Skullkeep Dragon:
- Type 0x0A: "actually used for the Skullkeep Dragon door"
- Door Strength field (0F 00 00)
- Color keys for transparency effects:
  - 04 00 00: Color key 1 (cyan) - see graphics behind door
  - 0C 00 00: Color key 2 (dark green) - see through door

Source: SKWin.GDAT2.InternalCodes.txt DOOR section

---

## Teleporter Square Type (0x18)

DM2 adds a dedicated Teleporter category in GDAT:
- Teleporter squares are rendered via QUERY_DUNGEON_MAP_CHIP_PICT using GDAT_CATEGORY_TELEPORTERS
- Sound: 89 00 00 (SND): Teleport sound

Source: SkWinCore.cpp, SKWin.GDAT2.InternalCodes.txt

---

## Creature Drop System

DM2 extends creature drop tables (0x0A through 0x14 = 11 drop slots).

Source: SKWin.GDAT2.InternalCodes.txt CREATURE section:
- 0A 00 00: Drop 1 (item ID + counts + random)
- 0B 00 00: Drop 2
- ...
- 14 00 00: Drop 11

DM1 only had a single drop slot per creature type.

---

## Outdoor Rendering System

DM2 outdoor renderer (include/dm2_v1_outdoor_renderer.h):
- Sky texture field
- Ground texture field
- Tree density
- Building count
- Weather system: clear/rain/fog/storm
- Time-of-day cycle (0.0-1.0) affects sky color

Source: include/dm2_v1_outdoor_renderer.h, include/dm2_v1_combat.h:
"companion NPCs, outdoor combat with different movement rules"

---

## Graphics Set Extensions

DM2 Graphics Set GDAT (0x08) adds:
- 67 00 00: Default ambient light
- 68 00 00: Lowest acceptable light level (0-5)
- 6D 00 00: Ambient darkness / sight distance (0 = full light, 8 = dark)

Source: SKWin.GDAT2.InternalCodes.txt GRAPHICS SET section

---

## Firestaff Implementation Status

The existing DM1 dungeon parser (M11/M12) does NOT cover DM2 format.
From docs/dm2-v1-dungeon-audit/dm2_dungeon.md:
"STATUS: GAP - DM2 dungeon format needs formal specification and parser"

## STATUS: SOURCE-LOCKED