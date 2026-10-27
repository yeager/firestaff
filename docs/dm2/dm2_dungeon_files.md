# DM2 V1 Dungeon Data Files — Source-Lock Audit

## Sources

- SKULL.ASM (sha256: a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099)
- skproject/SkGlobal.h
- skproject/SkWinCore.cpp
- SKWIN/knowledge/SKWin.GDAT2.InternalCodes.txt
- include/dm2_v1_dungeon_loader.h
- docs/dm2-v1-dungeon-audit/dm2_dungeon.md
- ReDMCSB DEFS.H (for DM1 comparison)

---

## DM2 Data File Format

### DUNGEON.DAT

DM2 DUNGEON.DAT structure follows the same overall layout as DM1:
1. **Header** — dungeon metadata
2. **Map descriptors** — per-level map information
3. **Square-first-thing index** — spatial indexing
4. **Thing data** — creatures, items, triggers
5. **Text data** — dungeon strings

DM1 DUNGEON_HEADER (ReDMCSB DEFS.H:985-998, 44 bytes):
- OrnamentRandomSeed (uint16)
- RawMapDataByteCount (uint16)
- MapCount (uint8)
- TextDataWordCount (uint16)
- InitialPartyLocation (uint16)
- SquareFirstThingCount (uint16)
- ThingCount[16] (uint16[16])

DM2 uses the same FTL compression signature (0x8104 big-endian) as DM1. COMPRESSED_DUNGEON_HEADER structure is identical: Signature(2) + DecompressedByteCount(4, BE) + DungeonID(2).

Source: docs/dm2-v1-dungeon-audit/dm2_dungeon.md, ReDMCSB DEFS.H:975-998

### File Size Comparison

| Variant | DUNGEON.DAT bytes | vs DM1 |
|---------|------------------|--------|
| DM1 PC 3.4 | 33,357 | baseline |
| DM2 DOS EN | 39,437 | +18% |
| DM2 Sega CD | 37,957 | +14% |

Source: docs/dm2-v1-dungeon-audit/dm2_dungeon.md

---

## Key Format Differences from DM1

### Extended Creature AI Table

DM2 uses 64 creature AI slots vs DM1's 42:
-  (SkGlobal.h:1007, DM2 extended)
-  (SkGlobal.h:1009, original/DM1/CSB)

MAXAI increases from 62 to 255 in extended mode.

Source: skproject/SkGlobal.h:1007-1009

### Extended GDAT Category System

DM2 extends GDAT category count from 0x1D (29) to 0xF0 (240):
-  (SkGlobal.h:636, DM2 extended)
-  (SkGlobal.h:638, original)

New GDAT categories in DM2 (defines.h:409-439):
| Category | Value | Purpose |
|----------|-------|---------|
| GDAT_CATEGORY_SPELL_DEF | 0x02 | Spell definitions (custom spells, up to 255) |
| GDAT_CATEGORY_CREATURE_AI | 0x19 | Per-creature AI behaviors |
| GDAT_CATEGORY_CHAMPIONS | 0x16 | Champion NPC data |
| GDAT_CATEGORY_TELEPORTERS | 0x18 | Teleporter square type |
| GDAT_CATEGORY_DOORS | 0x0E | Door properties |
| GDAT_CATEGORY_WEAPONS | 0x10 | Weapon data with projectile flags |
| GDAT_CATEGORY_DIALOG_BOXES | 0x1A | Dialog box graphics |
| GDAT_CATEGORY_ENVIRONMENT | 0x17 | Environment settings |

Source: defines.h:409-439, SkGlobal.h:636-638

### Extended Spell System

MAXSPELL increases from 34 (original) to 255 (custom) via GDAT spell definitions. EXTENDED_LOAD_SPELLS_DEFINITION loads custom spell tables from GDAT when in DM2 extended mode.

Source: skproject/SkWinCore.cpp, dm2-v1-overview/dm2_newfeatures.md

---

## GDAT2 Internal Codes (DM2 Extended Format)

SKWin.GDAT2.InternalCodes.txt documents the extended GDAT2 format with new field codes:

### Animated Ornate Fields
- : Number of cycled animated frames (up to 10)
- Sequence field: animated frame sequence

### Door Fields (New in DM2)
- : Door strength
- : Color key 1 (cyan) — see-through effect
- : Color key 2 (dark green) — secondary transparency
- : Animated mirrored door flag

### Creature Drop Fields
-  through : 11 drop slots (vs DM1's single drop)

### Item Animation
- : Animation field (e.g. 0x0504 = 4-frame animation)

### Spell Missile Fields
- , : Missile strength

### Potion Fields
- : Behavior
- : Water value
- : Spell missile association

### Ambient Light Fields
- : Default ambient light
- : Lowest acceptable light level (0-5)
- : Ambient darkness / sight distance (0 = full light, 8 = dark)

Source: SKWin.GDAT2.InternalCodes.txt

---

## Map Descriptor Structure (16 bytes, same as DM1)

DM2 uses the same 16-byte MAP descriptor as DM1 (DEFS.H:1048-1116), but with extended level field range:

- **Bitfield A** (offset 12-13): Level(6) + Width-1(5) + Height-1(5)
- **Bitfield B** (offset 14-15): WallOrnament(4) + RandomWall(4) + Floor(4) + RandomFloor(4)
- **Bitfield C** (offset 16-17): DoorOrnament(4) + CreatureType(4) + Unref(4) + Difficulty(4)
- **Bitfield D** (offset 18-19): FloorSet(4) + WallSet(4) + DoorSet0(4) + DoorSet1(4)
- **RawMapDataByteOffset** (uint16, offset 0-1)
- **OffsetMapX/OffsetMapY** (uint8 each, offset 4-5)

Source: ReDMCSB DEFS.H:1048-1116, docs/dm2-v1-dungeon-audit/dm2_dungeon.md

---

## DM2 Level Type Data in DUNGEON.DAT

The  struct (include/dm2_v1_dungeon_loader.h:25-29) stores:
- : number of levels (max 30)
- : OUTDOOR/INDOOR/BUILDING per level
- , : per-level dimensions
- : offsets into raw dungeon data
- , : outdoor-specific

---

## Comparison: DM1 vs DM2 DUNGEON.DAT Format

| Feature | DM1 | DM2 |
|---------|-----|-----|
| Header size | 44 bytes | Same |
| Compression | 0x8104 FTL | Same |
| Map descriptor | 16 bytes | Same |
| Map count | Variable (uint8) | 30 levels max |
| Thing types | Standard set | Extended with champions |
| Creature AI slots | 42 | 64 |
| GDAT categories | 29 (0x1D) | 240 (0xF0) |
| Custom spells | None (max 34) | 255 via GDAT |
| Drop slots | 1 per creature | 11 per creature |
| Outdoor levels | No | Yes (type 0) |
| Building levels | No | Yes (type 2) |
| Teleporter GDAT | No | Yes (0x18) |
| Champion GDAT | No | Yes (0x16) |

---

## Firestaff Implementation Status

No DM2-specific dungeon parser exists in Firestaff. The existing  only covers DM1 PC 3.4 format.

Source: docs/dm2-v1-dungeon-audit/dm2_dungeon.md: STATUS: GAP — DM2 dungeon format needs formal specification and parser

Firestaff DM2 V1 header files exist but lack a working parser:
- include/dm2_v1_dungeon_loader.h (stub loader)
- include/dm2_v1_outdoor_renderer.h
- include/dm2_v1_game.h

---

## STATUS: SOURCE-LOCKED — Formal parser still needed
