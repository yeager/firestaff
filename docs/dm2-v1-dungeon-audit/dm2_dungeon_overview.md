# DM2 V1 Dungeon Overview — Source-Lock Audit

## Sources

- SKULL.ASM (522,128 lines IDA disassembly, sha256: a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099)
- skproject (github.com/gbsphenx/skproject) HEAD a962896
- include/dm2_v1_dungeon_loader.h (Firestaff header)
- docs/dm2-v1-dungeon-audit/dm2_dungeon.md

---

## How Many Dungeon Levels in DM2?

DM2 supports up to **30 levels** (DM2_V1_MAX_LEVELS = 30) per dungeon.

Source: include/dm2_v1_dungeon_loader.h:15


DM1's DUNGEON_HEADER (ReDMCSB DEFS.H:985) stores MapCount as uint8, so maximum 255 maps in theory, but practical DM1 dungeons had far fewer (DM1 PC 3.4 has 16 maps for 10 levels).

DM2's three distinct level types:
| Type | Value | Description |
|------|-------|-------------|
| DM2_LEVEL_OUTDOOR | 0 | Sky, ground, trees, buildings |
| DM2_LEVEL_INDOOR | 1 | Standard first-person dungeon |
| DM2_LEVEL_BUILDING | 2 | Multi-floor buildings within outdoor areas |

Source: include/dm2_v1_dungeon_loader.h:19-21

---

## Comparison with DM1 Dungeon Size

| Metric | DM1 PC 3.4 | DM2 DOS EN | Ratio |
|--------|-----------|-----------|-------|
| DUNGEON.DAT | 33,357 bytes | 39,437 bytes | +18% |
| Max levels | ~16 maps (MapCount uint8) | 30 levels | +87% |
| Creature AI slots | 42 | 64 | +52% |
| GDAT categories | 29 (0x1D) | 240 (0xF0) | +8x |

DM1 map count (DEFS.H:985):  — variable, typically 16 for 10-level DM1.
DM2 level count (include/dm2_v1_dungeon_loader.h:25): stored in  int, max 30.

DM1 used 16-byte MAP descriptors (DEFS.H:1048-1116) with bitfields for dimensions, level, ornament sets.
DM2 uses the same 16-byte map descriptor structure but extends the level field range via the extended mode.

---

## DM2 Level Type System

DM2 introduces a three-type level system not present in DM1:

- **OUTDOOR (0)**: Full landscape view. Renders sky_texture_index, ground texture, tree density, building count, weather zones. Outdoor combat uses different movement rules (include/dm2_v1_combat.h: companion NPCs, outdoor combat with different movement rules).
- **INDOOR (1)**: Standard first-person dungeon, similar rendering to DM1 (wall/floor/ceiling).
- **BUILDING (2)**: Multi-floor structures embedded in outdoor areas. Transition renderer between outdoor and indoor views.

Source: include/dm2_v1_dungeon_loader.h:19-29

---

## DUNGEON.DAT File Size Evidence

| Variant | DUNGEON.DAT bytes |
|---------|------------------|
| DM1 PC 3.4 | 33,357 |
| DM2 DOS EN (EN/LEGEND) | 39,437 |
| DM2 Sega CD | 37,957 |

The ~18% larger DM2 DUNGEON.DAT is due to:
1. Extended creature AI table (64 vs 42 slots)
2. Additional GDAT category data (240 vs 29 categories)
3. Outdoor level data fields (sky/weather per level)
4. Extended spell definition tables (255 vs 34)

Source: docs/dm2-v1-dungeon-audit/dm2_dungeon.md

---

## FTL Compression

DM2 uses the same FTL decompression signature (0x8104 big-endian) as DM1 for dungeon data compression. COMPRESSED_DUNGEON_HEADER structure is identical.

Source: docs/dm2-v1-dungeon-audit/dm2_dungeon.md (skproject READ_DUNGEON_STRUCTURE)

---

## STATUS: SOURCE-LOCKED
