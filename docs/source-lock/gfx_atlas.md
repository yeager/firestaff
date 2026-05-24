# DM1 V1 Graphic Atlas — Bitmap Index Ranges

File: /tmp/gfx_atlas.md
Audit: DM1 V1 GRAPHICS.DAT bitmap index ranges by type
Sources: ReDMCSB DEFS.H, MEMORY.C, DUNVIEW.C; Firestaff dm1_v1_graphics_loader_pc34_compat.c

---

## 1. GRAPHICS.DAT Layout

GRAPHICS.DAT contains ~600+ compressed 4-bitplane bitmap images indexed 0-N.

Loading: F0474_MEMORY_LoadGraphic_CPSDF reads the GRAPHICS.DAT file, seeks
to the graphic offset, and decompresses via F0497_LZW_Decompress.

Source: MEMORY.C:707 -- F0474_MEMORY_LoadGraphic_CPSDF
Source: MEMORY.C:2583 -- F0490_MEMORY_LoadDecompressAndExpandGraphic

---

## 2. Bitmap Index Ranges (PC 3.4 / I34E)

ReDMCSB source: DEFS.H:2260-2392

| Constant                            | Value       | Description               |
|-------------------------------------|-------------|---------------------------|
| C076_GRAPHIC_FIRST_FIELD            | 76          | Teleporter fields         |
| M652_GRAPHIC_FIRST_FIELD_MASK       | 69 / 70     | Field mask (V1/V2)        |
| M644_GRAPHIC_FIRST_FLOOR_SET        | 75 / 78     | Floor sets (V1/V2)        |
| M646_GRAPHIC_FIRST_WALL_SET         | 77 / 86     | Wall sets (V1/V2)         |
| M645_GRAPHIC_FIRST_STAIRS           | 90 / 108    | Stairs (V1/V2)            |
| M633_GRAPHIC_FIRST_DOOR_SET         | 108 / 246   | Door sets (V1/V2)         |
| M615_GRAPHIC_FIRST_WALL_ORNAMENT    | 121 / 259   | Wall ornaments            |
| M616_GRAPHIC_FIRST_FLOOR_ORNAMENT   | 247 / 385   | Floor ornaments           |
| M617_GRAPHIC_FIRST_DOOR_ORNAMENT    | 303 / 441   | Door ornaments            |
| M634_GRAPHIC_FIRST_DOOR_BUTTON      | 315 / 453   | Door button               |
| M613_GRAPHIC_FIRST_PROJECTILE        | 316 / 454   | Projectile graphics       |
| M614_GRAPHIC_FIRST_EXPLOSION        | 348 / 486   | Explosion graphics        |
| M636_GRAPHIC_FIRST_EXPLOSION_PATTERN | 351 / 489   | Explosion patterns        |
| C360_GRAPHIC_FIRST_OBJECT            | 360         | Object graphics (V1)      |
| M612_GRAPHIC_FIRST_OBJECT           | 360 / 498   | Object graphics (V1/V2)   |
| M618_GRAPHIC_FIRST_CREATURE         | 446 / 584   | First creature bitmap     |
| M719_GRAPHIC_FIRST_SOUND           | 533 / 671   | Sound/attack graphics    |

---

## 3. Derived Bitmap Cache

ReDMCSB source: DEFS.H:2412-2419, 3339-3356

| Constant                                  | Value | Description            |
|-------------------------------------------|-------|------------------------|
| C004_DERIVED_BITMAP_FIRST_WALL_ORNAMENT   | 4     | Wall ornament scaled   |
| C068_DERIVED_BITMAP_FIRST_DOOR_ORNAMENT_D3 | 68   | Door ornament D3       |
| C069_DERIVED_BITMAP_FIRST_DOOR_ORNAMENT_D2 | 69  | Door ornament D2       |
| C102_DERIVED_BITMAP_FIRST_DOOR_BUTTON     | 102   | Door button            |
| C104_DERIVED_BITMAP_FIRST_OBJECT          | 104   | Object scaled (178)    |
| M537_DERIVED_BITMAP_FIRST_PROJECTILE      | 282   | Projectile scaled      |
| M538_DERIVED_BITMAP_FIRST_EXPLOSION       | 438   | Explosion scaled       |
| M539_DERIVED_BITMAP_FIRST_CREATURE        | 495   | Creature scaled (219)  |

Cache lookup: F0491_CACHE_IsDerivedBitmapInCache (MEMORY.C:2641)

---

## 4. Creature Bitmap Details (M618=584 base)

ReDMCSB source: DUNVIEW.C:2864, DEFS.H:2321, 2392

87 creature native bitmap slots in GRAPHICS.DAT (indices 584-670).

CreatureAspect table (G0219): maps creature type to:
  - firstNativeBitmapRelativeIndex: offset from M618
  - firstDerivedBitmapIndex: derived cache start
  - coordinateSet_transparentColor: packed coords + transparent color
  - replacementColorSetIndices: color replacement lookups
  - graphicInfo: flags

Derived cache: M539_DERIVED_BITMAP_FIRST_CREATURE = 495

---

## 5. Compatibility Status

| Component            | ReDMCSB source | Firestaff status |
|----------------------|---------------|------------------|
| GRAPHICS.DAT header  | MEMORY.C:707  | implemented      |
| Index offset lookup  | MEMORY.C:742  | needs verify     |
| LZW decompress       | LZW.C:107     | implemented      |
| IMG3 expand          | IMAGE3.C:1100 | partial          |
| Derived bitmap cache | MEMORY.C:2641 | partial          |
| Creature range 584+  | DEFS.H:2321   | documented       |
