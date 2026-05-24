# DM2 V1 Dungeon Data — Source-Lock Audit

## Sources

- SKULL.ASM (IDA disassembly, sha256 )
- skproject (https://github.com/gbsphenx/skproject) SKWIN/SkGlobal.h, SkWinCore.cpp
- Greatstone Atlas: 
- Firestaff , 

## DUNGEON.DAT File Sizes

| Variant | DUNGEON.DAT bytes |
|---|---:|
| DM1 PC 3.4 | 33,357 |
| DM2 DOS EN (EN/LEGEND) | 39,437 |
| DM2 Sega CD | 37,957 |

DM2 is ~18% larger than DM1's DUNGEON.DAT, indicating additional dungeon data fields or structures.

## DM2 Dungeon Format — Differences from DM1

### skproject READ_DUNGEON_STRUCTURE

-  —  — core dungeon loading routine
- DM2 uses the same overall DUNGEON.DAT layout as DM1: header + maps + thing data, but extended fields

### Key Differences

1. **Extended creature AI table**: DM2 uses  (DM2 extended mode); DM1/CSB use 42
   - skproject/SkGlobal.h:1007 
   - skproject/SkGlobal.h:1009  (original/DM1/CSB)

2. **GDAT_CATEGORY_LIMIT**: DM2 extended mode allows 0xF0 categories; original allows 0x1D
   - skproject/SkGlobal.h:636  (DM2 extended)
   - skproject/SkGlobal.h:638  (original)

3. **New GDAT categories in DM2**: skproject SkWinCore.cpp uses categories not present in DM1:
   -  (spell definitions)
   -  (door-specific data, GDAT_DOOR_MIRRORED)
   -  (0x18)
   - 
   - 
   - 
   - 

4. **New door type**: DM2 uses a second clan door type (0x0A) for the Skullkeep Dragon door
   - Greatstone/SKWin.GDAT2.InternalCodes.txt: "Note: The second clan door type (0x0A) is actually used for the Skullkeep Dragon door."

5. **Animated mirrored door**: DM2 has 
   - This has no particular use in DM2 standard dungeons

6. **Outdoor areas**: DM2 adds outdoor combat with different movement rules
   - include/dm2_v1_combat.h: "companion NPCs, outdoor combat with different movement rules"

## DM2 Map Descriptor

DM2 map descriptors follow the same 16-byte structure as DM1 (DEFS.H MAP descriptor), but DM2 dungeons have more levels/maps per dungeon. The DM2 dungeon loaded by SKULL.EXE uses the same FTL compression signature (0x8104) as DM1.

## Firestaff Implementation

No DM2-specific dungeon parser exists yet in Firestaff. The existing  only covers DM1 PC 3.4 format.

## STATUS: GAP — DM2 dungeon format needs formal specification and parser
